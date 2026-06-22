#include "init.hpp"

namespace voternn {
	void init() {
		std::cout.sync_with_stdio(false);

		init_random_generator();

		setlocale(LC_ALL, "");		
	}
}