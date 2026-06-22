#ifndef VOTERNN_UTIL_MATH
#define VOTERNN_UTIL_MATH
#include "../include.hpp"

namespace voternn {
	double distance(
		const double v1[],
		const double v2[],
		const size_t length
	);

	extern std::mt19937_64 random_generator; 

	void init_random_generator();

	std::string random_string(const size_t length);

	double pythagoras(const int64_t dx, const int64_t dy);
};

#endif