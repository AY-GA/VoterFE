#ifndef VOTERNN_UTIL_HTTP_LISTENER_TYPES
#define VOTERNN_UTIL_HTTP_LISTENER_TYPES
#include "../../include.hpp"

namespace voternn {
		enum class handler_client_error_t {
		NONE, WRONG_PARAM_TYPE, PARAM_OUT_OF_RANGE, INVALID_PARAM,
		NO_PARAM, INVALID_OBJECT
	};

	typedef std::variant<std::monostate, nlohmann::json, handler_client_error_t> webserver_handler_return_t;

	typedef int webserver_socket_t;
	/// @brief Handler type. May throw exceptions on failure.
	/// @name webserver_handler_t
	typedef std::function<webserver_handler_return_t(const std::string& token, const nlohmann::json& json)> webserver_handler_t;

	constexpr const uint64_t BUFFER_SIZE = 16384;

	/// @brief IP block data.
	struct ip_block_data_t {
		bool will_be_unblocked = true;
		std::chrono::system_clock::time_point unblock;
	};

	/// @brief HTTP version.
	enum class http_version_t {
		V09, V10, V11, V2, V3
	};

	/// @brief HTTP Method enumerator.
	enum class http_method_t {
		NONE,
		GET, POST,
		HEAD, OPTIONS, PATCH, PUT, TRACE, CONNECT, DELETE
	};	

	std::string to_string(const http_method_t e) noexcept;

	/// @brief HTTP Request information (i.e. first line information)
	struct http_request_info_t {
		http_method_t method;
		std::string path;
		http_version_t version;
		std::vector<std::pair<std::string, std::string>> headers;
		std::string body;

		http_request_info_t() : method(http_method_t::NONE), path(""), version(http_version_t::V09) {}
	};

	/// @brief HTTP request errors
	enum class http_request_error_t {
		NONE, 
		WRONG_METHOD, WRONG_PATH, WRONG_PARAMS,
		WRONG_HTTP_VERSION, 
		NO_BODY,
		NO_ORIGIN_HEADER,
		UNAUTHORIZED,
	};

	/// @brief Handler route information
	struct webserver_handler_information_t {
		http_method_t method;
		webserver_handler_t handler;
		std::vector<std::string> params;

		webserver_handler_information_t() 
			: method(http_method_t::NONE) {}

		webserver_handler_information_t(
			http_method_t method,
			const webserver_handler_t& handler,
			const std::vector<std::string>& params
		) : method(method), handler(std::move(handler)), params(std::move(params)) {}
	};
}

#endif
