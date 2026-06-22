#ifndef VOTERNN_SYSTEM_ELECTORAL
#define VOTERNN_SYSTEM_ELECTORAL
#include "../include.hpp"
#include "../types/party.hpp"

namespace voternn {
	/// @brief Results of a single district. 
	struct district_electoral_result {
		/// @brief Map from UUID to amount of votes per round.
		std::unordered_map<std::string, std::vector<uint64_t>> results;
		bool multi_round_system_used;
	};

	/// @brief Final electoral results for parliaments.
	struct electoral_result {
		std::unordered_map<std::string, uint64_t> mps;
		std::string winner;
	};

	/// @brief Final electoral results for presidents, where there is one winner.
	struct presidential_electoral_result {
		std::string winner;
		double winner_percentage;
	};

	/// @brief Structure representing the electoral system.
	/// D'Hondt, the HungryOrbman (Hungarian under Fidesz and Orban Viktor), perfect proportional representation, STV and British FPTP are implemented.
	/// Others must be implemented by the user themselves.
	struct electoral_system {
		virtual electoral_result vote(const std::vector<district_electoral_result>& districts) {
			throw std::runtime_error("Don't use this basic type - used a specific parliamentary electoral system.");
		}
	};

	/// @brief Structure representing the presidential electoral system.
	/// Two-round popular and the Electoral College are implemented.
	struct presidential_electoral_system {
		virtual presidential_electoral_result vote(const std::vector<district_electoral_result>& districts) {
			throw std::runtime_error("Don't use this basic type - used a specific presidential electoral system.");
		}
	};

	namespace electoral_systems {
		/// @brief D'Hondt electoral system.
		/// Larger parties have a boost.
		/// Used in Poland.
		/// 
		struct d_hondt final : public electoral_system {
			electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};

		/// @brief First-past-the-post electoral system.
		/// Every district gets 1 MP.The amount of members of parliament is not variable with this system.
		/// Used in the UK.
		struct fptp final : public electoral_system {
			electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};

		/// @brief Hungarian electoral system under Viktor Orban.
		/// Used in Hungary under Viktor Orban.
		struct hungry_orbman final : public electoral_system {
			electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};

		/// @brief Perfect proportional representation.
		/// Used... nowhere :/
		struct perfect final : public electoral_system {
			electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};

		/// @brief Single trasferable vote.
		/// Used in Australia.
		struct stv final : public electoral_system {
			electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};
	}

	namespace presidential_electoral_systems {
		struct two_round final : public presidential_electoral_system {
			presidential_electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};

		/// @brief Electoral college electoral system.
		/// Every district gets 
		/// Used in the US.
		struct electoral_college final : public presidential_electoral_system {
			presidential_electoral_result vote(const std::vector<district_electoral_result>& districts) override;
		};
	};

	/// @brief Type definition of unordered map from party UUID to total popular vote across districts.
	typedef std::unordered_map<std::string, uint64_t> popular_election_results;

	popular_election_results sum_votes(const std::vector<district_electoral_result>& districts);
}

#endif