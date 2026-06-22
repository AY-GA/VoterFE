#include "listener_types.hpp"

namespace voternn {
	std::string to_string(const http_method_t e) noexcept {
		switch(e) {
			case(http_method_t::NONE): return "UNKNOWN method.";
			case(http_method_t::GET): return "GET";
			case(http_method_t::POST): return "POST";
			case(http_method_t::HEAD): return "HEAD";
			case(http_method_t::OPTIONS): return "OPTIONS";
			case(http_method_t::PATCH): return "PATCH";
			case(http_method_t::PUT): return "PUT";
			case(http_method_t::TRACE): return "TRACE";
			case(http_method_t::CONNECT): return "CONNECT";
			case(http_method_t::DELETE): return "DELETE";
		}
		return "voternn::to_string for http_method_t failed!";
	}
}