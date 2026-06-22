#include "opinion.hpp"

namespace voternn {
	double calculate_opinion_distance(
		const opinion_position& pos1,
		const opinion_position& pos2
	) {
		return distance(
			pos1.values,
			pos2.values,
			12
		);
	}
} //namespace voternn
