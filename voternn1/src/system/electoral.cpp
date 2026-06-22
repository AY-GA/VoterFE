#include "electoral.hpp"

namespace voternn {
	namespace electoral_systems {
		electoral_result d_hondt::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}

		electoral_result fptp::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}

		electoral_result hungry_orbman::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}

		electoral_result perfect::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}

		electoral_result stv::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}
	}

	namespace presidential_electoral_systems {
		presidential_electoral_result two_round::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}
		presidential_electoral_result electoral_college::vote(const std::vector<district_electoral_result>& districts) {
			return {};
		}
	}

	popular_election_results sum_votes(const std::vector<district_electoral_result>& districts) {
		popular_election_results per;
		for(const auto& der : districts) {
			for(const auto& pe : der.results) {
				for(const uint64_t votes : pe.second)
					per[pe.first] += votes;
			}
		}  

		return per;
	}
}