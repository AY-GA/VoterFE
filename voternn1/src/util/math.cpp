#include "math.hpp"

namespace voternn {
	double distance(
			const double v1[],
			const double v2[],
			const size_t length
		) {	
		double sum = 0;
		for(size_t i = 0; i < length; i++) {
			const double val = std::abs(v1[i]-v2[i]);
			sum += (val*val);
		}

		return std::sqrt(sum);
	}

	std::mt19937_64 random_generator;

	void init_random_generator() {
		std::random_device rd;

		random_generator.seed(rd());
	}

	std::string random_string(const size_t length) {
		std::string result;
		std::uniform_int_distribution<char8_t> distrib(97, 122);

		for(uint64_t i = 0; i < length; i++) {
			result += (char)distrib(random_generator);
		}

		return result;
	}

	double pythagoras(const int64_t dx, const int64_t dy) {
		const uint64_t adx = std::abs(dx);
		const uint64_t ady = std::abs(dy);

		return std::sqrt(adx*adx+ady*ady);
	}
};