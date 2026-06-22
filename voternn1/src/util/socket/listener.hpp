#ifndef VOTERNN_UTIL_HTTP_LISTENER
#define VOTERNN_UTIL_HTTP_LISTENER
#include "listener_types.hpp"
#include "../../util/math.hpp"

//TODO handle returning from handler function
//		either no data to return, or json data, or client's error
//		server error - exception!

namespace voternn {
	/// @brief HTTP Listener
	class listener {
	public:
		listener() = delete;
		listener(const std::string_view token, const uint16_t port);

		/// @brief Registers a new API route.
		/// @param method Method of route (GET, POST, ...)
		/// @param path Path of API route.
		/// @param handler Handler itself. @see webserver_handler_t
		/// @param params Required params that will be checked for.
		listener& register_handler(
			http_method_t method,
			const std::string& path,
			const webserver_handler_t& handler,
			const std::vector<std::string>& params
		);

		/// @brief Polls for new events. Every call checks exactly one event.
		/// @return Either true, if there is an event to process, false, if there isnt, or and http_request_error_t enum struct if the request is malformed.
		bool check();

		std::string get_token() const;

		~listener();
	private:
		webserver_socket_t handle;
		uint16_t port;
		pollfd pfd; //we have single socket

		std::string token;

		std::unordered_map<std::string, webserver_handler_information_t> handlers;

		std::unordered_map<std::string, uint64_t> hits;
		std::unordered_map<std::string, ip_block_data_t> ipblock;

		http_method_t get_method(const std::string_view method) const;
		http_version_t get_version(const std::string_view version) const;

		std::expected<http_request_info_t, http_request_error_t> get_info(const std::string& request) const;

		const char* get_headers() const;
		const char* get_response_name(const uint16_t status_code) const;
		std::string make_empty_response(const uint16_t status_code) const;
		std::string make_response(const uint16_t status_code, const nlohmann::json& data) const;
	};
}

#endif
