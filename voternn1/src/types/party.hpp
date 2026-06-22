#ifndef VOTERNN_TYPE_PARTY
#define VOTERNN_TYPE_PARTY
#include "voter.hpp"
#include "opinion.hpp"

namespace voternn {
	/// @brief Member level of a party member.
	class party_member : public voter {
		uint64_t points;
	};

	/// @brief Representation of a political party.
	class party {
	public:
	private:
		std::string name;
		std::string abbreviation;
		opinion_position position;
		std::vector<party_member> members;
	};
} // namespace voternn

#endif
