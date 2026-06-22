#ifndef VOTERNN_SYSTEM_PARLIAMENT_CONFIG
#define VOTERNN_SYSTEM_PARLIAMENT_CONFIG
#include "../include.hpp"

namespace voternn {
	/// @brief Configuration structure of parliament.
	struct parliament_config {
		/// @brief Percentage (0.5-1) of MPs needed to overrule veto. Min. value 0.5.
		double veto_percentnage;
		/// @brief Amount of MPs is parliament.
		/// @warning When FPTP electoral system is used, this value is ignored.
		/// @note Electoral systems may add additional ones (e.g. German MMR).
		uint64_t mps;
	};
}

#endif