#ifndef VOTERNN_UTIL_LLM_API_CALLER
#define VOTERNN_UTIL_LLM_API_CALLER
#include "../../include.hpp"

namespace voternn {
	const char* LLM_ENDPOINT = "";

	extern std::string BASE_PROMPT;

	std::string build_prompt();
}

#endif