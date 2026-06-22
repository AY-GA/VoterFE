#include "listener.hpp"

namespace voternn {
	namespace internal {
		class socket_handler {
		public:
			socket_handler(webserver_socket_t& socket) : socket(socket) {}
			
			~socket_handler() {
				shutdown(this->socket, SHUT_RDWR);
				close(this->socket);
			}
		private:
			webserver_socket_t& socket;
		};

		std::string trim_path(std::string path) {
			if(path.size() > 1 && path.back() == '/') path.pop_back();
			return path;
		}
	}

	listener::listener(const std::string_view token, const uint16_t port)
		: port(port), token(token) {
		addrinfo hint;
		memset(&hint, 0, sizeof(addrinfo));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = IPPROTO_TCP;
		hint.ai_flags = AI_PASSIVE;

		addrinfo* result = NULL;
		std::string port_as_string = std::to_string(this->port);
		webserver_socket_t error = getaddrinfo(NULL, port_as_string.c_str(), &hint, &result);

		this->handle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if(this->handle == -1) throw std::runtime_error("Invalid socket!");

		int reuse = 1;
		if(
			setsockopt(this->handle, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ||
			setsockopt(this->handle, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse))
		) throw std::runtime_error("Error while setting up socket!");

		if(bind(this->handle, result->ai_addr, result->ai_addrlen))
			throw std::runtime_error("Error while binding to socket!");

		//handle async IO with existing flags untouched
		//fcntl(this->handle, F_SETFL, fcntl(this->handle, F_GETFL, 0));

		this->pfd.fd = this->handle;
		this->pfd.events = POLLERR | POLLIN;

		listen(this->handle, 10);

		freeaddrinfo(result);		
	}

	listener& listener::register_handler(
			http_method_t method,
			const std::string& path,
			const webserver_handler_t& handler,
			const std::vector<std::string>& params
		) {
		const std::string normalized_path = internal::trim_path(path);
		if(this->handlers.contains(normalized_path)) {
			throw std::runtime_error("Handler for this path exists!");
		}
		this->handlers.emplace(normalized_path, std::move(webserver_handler_information_t(method, handler, params)));

		return *this;
	}

	bool listener::check() {
		if(poll(&this->pfd, 1, 0) == -1)
			throw std::runtime_error("Polling error!");

		if(this->pfd.revents == 0) return false;
		if(!(this->pfd.revents & POLLIN)) return false;

		std::string request;
		char buffer[BUFFER_SIZE];
		http_request_error_t error = http_request_error_t::NONE;

		webserver_socket_t rs = accept(this->handle, NULL, NULL);
		internal::socket_handler rsh(rs);
		
		while(true) {
			int err = recv(rs, buffer, BUFFER_SIZE, 0);
			if(err == -1) throw std::runtime_error("Read error!");
			if(err == 0) break;
			
			request.append(buffer, err);
			if(err < BUFFER_SIZE) break; //all read
		}

		//std::cout << request << '\n';

		auto hri_expected = this->get_info(request);
		http_request_info_t hri;
		webserver_handler_information_t handler;
		std::string response;
		bool response_ready = false;

		if(!hri_expected.has_value()) error = hri_expected.error();
		else hri = hri_expected.value();

		if(error == http_request_error_t::NONE && hri.version != http_version_t::V11) error = http_request_error_t::WRONG_HTTP_VERSION;
		else {
			if(error == http_request_error_t::NONE && hri.method == http_method_t::OPTIONS) {
				response = this->make_empty_response(204);
				response_ready = true;
			}
			else if(error == http_request_error_t::NONE) {
				try {
					handler = handlers.at(internal::trim_path(hri.path));
				}
				catch(std::out_of_range& e) {
					error = http_request_error_t::WRONG_PATH;
				}

				if(handler.method != hri.method && error == http_request_error_t::NONE) 
					error = http_request_error_t::WRONG_METHOD;
			}
		}		

		if(error == http_request_error_t::NONE && !response_ready) {
			try {
				nlohmann::json body = nlohmann::json::object();

				if(hri.method == http_method_t::POST) {
					if(hri.body.empty()) {
						error = http_request_error_t::NO_BODY;
						throw std::invalid_argument("Missing request body");
					}

					body = nlohmann::json::parse(hri.body);
					if(!body.contains("token") || !body.at("token").is_string() || body.at("token").get<std::string>() != this->token) {
						error = http_request_error_t::UNAUTHORIZED;
						throw std::invalid_argument("Unauthorized token");
					}
				}

				webserver_handler_return_t hrv = handler.handler(this->token, body);
				std::cout << to_string(hri.method) << ' ' << hri.path << " API route reached!\n";

				if(std::holds_alternative<std::monostate>(hrv)) {
					//success, no data

					std::cout << "HRV 204 OK";

					response = this->make_empty_response(204);
				}
				else if(std::holds_alternative<nlohmann::json>(hrv)) {
					//success, data

					std::cout << "HRV 200 OK";

					response = this->make_response(200, std::get<nlohmann::json>(hrv));
				}
				else if(std::holds_alternative<handler_client_error_t>(hrv)) {
					//failure, client's error

					std::cout << "HRV 400 NOT OK";

					response = this->make_empty_response(400);
				}

				std::cout << std::endl;
			}
			catch(nlohmann::json::exception& e) {
				response = this->make_empty_response(400);
				std::cerr << "Bad JSON request for " << hri.path << ": " << e.what() << '\n';
			}
			catch(std::invalid_argument& e) {
				if(error == http_request_error_t::UNAUTHORIZED)
					response = this->make_empty_response(401);
				else
					response = this->make_empty_response(400);
				std::cerr << "Bad request for " << hri.path << ": " << e.what() << '\n';
			}
			catch(std::exception& e) {
				response = this->make_empty_response(500);
				std::cerr << "Handler failure for " << hri.path << ": " << e.what() << '\n';
			}
		}
		else if(!response_ready) {
			switch(error) {
				case(http_request_error_t::WRONG_METHOD):
					response = this->make_empty_response(405);
					break;
				case(http_request_error_t::WRONG_HTTP_VERSION):
					response = this->make_empty_response(505);
					break;
				case(http_request_error_t::WRONG_PATH):
					response = this->make_empty_response(404);
					break;
				case(http_request_error_t::UNAUTHORIZED):
					response = this->make_empty_response(401);
					break;
				case(http_request_error_t::WRONG_PARAMS):
				case(http_request_error_t::NO_BODY):
				case(http_request_error_t::NO_ORIGIN_HEADER):
					response = this->make_empty_response(400);
					break;
				default:
					response = this->make_empty_response(500);
					break;
			}
		}

		int err = send(rs, response.data(), response.size(), 0);
		if(err == -1)
			throw std::runtime_error("Response send error");

		return true;
	}

	std::string listener::get_token() const {
		return this->token;
	}

	listener::~listener() {}

	http_method_t listener::get_method(const std::string_view method) const {
		if(method.empty()) return http_method_t::NONE;
		if(method[0] == 'P') {
			if(method == "POST") return http_method_t::POST;
			else if(method == "PUT") return http_method_t::PUT;
			else if(method == "PATCH") return http_method_t::PATCH;
			else return http_method_t::NONE;
		}
		else {
			if(method == "GET") return http_method_t::GET;
			else if(method == "OPTIONS") return http_method_t::OPTIONS;
			else if(method == "CONNECT") return http_method_t::CONNECT;
			else if(method == "DELETE") return http_method_t::DELETE;

			else return http_method_t::NONE;
		}
	}

	http_version_t listener::get_version(const std::string_view version) const {
		if(version == "HTTP/1.1") return http_version_t::V11;
		else if(version == "HTTP/1.0") return http_version_t::V10;
		else return http_version_t::V09;
	}

	std::expected<http_request_info_t, http_request_error_t> listener::get_info(const std::string& request) const {
		uint64_t header_offset = request.find("\r\n");

		if(header_offset == request.npos)
			return std::unexpected(http_request_error_t::NO_ORIGIN_HEADER);

		std::string_view firstline = std::string_view(request).substr(0, header_offset); 
		http_request_info_t result;

		{
			uint64_t offset = firstline.find(' ');
			if(offset == firstline.npos) return std::unexpected(http_request_error_t::NO_ORIGIN_HEADER);
			result.method = this->get_method(firstline.substr(0, offset));
			uint64_t offset2 = firstline.find(' ', offset+1);
			if(offset2 == firstline.npos) return std::unexpected(http_request_error_t::NO_ORIGIN_HEADER);
			result.path = std::string(firstline.substr(offset+1, offset2-offset-1));
			result.version = this->get_version(firstline.substr(offset2+1));

			// Some proxies send absolute-form request lines (e.g. "POST http://host/path HTTP/1.1").
			// Normalize to just the path component so handler lookup matches registered routes.
			if(result.path.rfind("http://", 0) == 0 || result.path.rfind("https://", 0) == 0) {
				auto scheme_pos = result.path.find("://");
				size_t host_start = (scheme_pos == std::string::npos) ? 0 : scheme_pos + 3;
				size_t path_pos = result.path.find('/', host_start);
				if(path_pos != std::string::npos) result.path = result.path.substr(path_pos);
				else result.path = "/";
			}

			// Strip query string if present
			size_t qpos = result.path.find('?');
			if(qpos != std::string::npos) result.path = result.path.substr(0, qpos);
		}

		const uint64_t body_offset = request.find("\r\n\r\n");
		if(body_offset != request.npos)
			result.body = request.substr(body_offset+4);

		return result;
	}

	//headers in each request
	const char* listener::get_headers() const {
		return
			"Access-Control-Allow-Origin: *\r\n"
			"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
			"Access-Control-Allow-Headers: Content-Type\r\n"
			"Connection: close"
		;
	}

	const char* listener::get_response_name(const uint16_t status_code) const {
		switch(status_code) {
			case(200): return "HTTP/1.1 200 OK\r\n";
			case(201): return "HTTP/1.1 201 Created\r\n";
			case(204): return "HTTP/1.1 204 No Content\r\n";
			case(400): return "HTTP/1.1 400 Bad Request\r\n";
			case(401): return "HTTP/1.1 401 Unauthorized\r\n";
			case(403): return "HTTP/1.1 403 Forbidden\r\n";
			case(404): return "HTTP/1.1 404 Not Found\r\n";
			case(405): return "HTTP/1.1 405 Method Not Allowed\r\n";
			case(429): return "HTTP/1.1 429 Too Many Requests\r\n";
			case(500): default: return "HTTP/1.1 500 Internal Server Error\r\n";
			case(503): return "HTTP/1.1 503 Service Unavailable\r\n";
			case(505): return "HTTP/1.1 505 Version Not Supported\r\n";
		}
	}

	std::string listener::make_response(const uint16_t status_code, const nlohmann::json& data) const {
		std::string serialized_json = nlohmann::to_string(data);
		std::string result;

		result.append(this->get_response_name(status_code));
		result.append("Content-Length: ");
		result.append(std::to_string(serialized_json.size()));
		result.append("\r\nContent-Type: application/json\r\n");
		result.append(this->get_headers());
		result.append("\r\n\r\n");
		result.append(serialized_json);

		return result;
	}

	std::string listener::make_empty_response(const uint16_t status_code) const {
		std::string result;

		result.append(this->get_response_name(status_code));
		result.append(this->get_headers());
		result.append("\r\nContent-Length: 0\r\n\r\n");

		return result;
	}
};
