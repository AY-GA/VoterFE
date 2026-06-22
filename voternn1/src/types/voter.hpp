#ifndef VOTERNN_TYPE_VOTER
#define VOTERNN_TYPE_VOTER
#include "../include.hpp"
#include "opinion.hpp"

namespace voternn {
	/// @brief Character of voter.
	struct voter_character {
		double& openness            = this->values[0];
		double& neuroticism         = this->values[1];
		double& consciencitiousness = this->values[2];
		double& agreeableness       = this->values[3];
		double& extraversion        = this->values[4];
		double& intelligence        = this->values[5];
		double& racism              = this->values[6];
		double& skepticism          = this->values[7];
		double& ambition            = this->values[8];
		double& loyalty             = this->values[9];
		double& taxProgressiveness  = this->values[10];

		double values[11] {};
	};

	/// @brief Trust of voter towards govt.
	struct voter_trust {
		double& local    = this->values[0];
		double& national = this->values[1];

		double values[2] {};
	};

	/// @brief voter's personal situation.
	struct voter_situation {
		double& income            = this->values[0];
		double& job_satisfaction   = this->values[1];
		double& age               = this->values[2];
		double& education         = this->values[3];
		double& crime_themselves   = this->values[4];
		double& crime_on_him        = this->values[5];
		double& tolerance         = this->values[6];
		double& community         = this->values[7];
		double& unemployment_time  = this->values[8];
		double& personal_taxes_level = this->values[9];

		double values[10] {};
	};
	
	/// @brief voter instance.
	class voter {
		friend voter make_random_voter();
	public:
		voter();
		// Ensure proper value-copying so reference-backed sub-objects
		// in each voter bind to that voter's internal storage.
		voter(const voter& other);
		voter(voter&& other) noexcept;
		voter& operator=(const voter& other);
		voter& operator=(voter&& other) noexcept;

		voter_character& get_character() noexcept;
		voter_trust& get_trust() noexcept;
		voter_situation& get_situation() noexcept;
		opinion_position& get_position() noexcept;

		const voter_character& get_character() const noexcept;
		const voter_trust& get_trust() const noexcept;
		const voter_situation& get_situation() const noexcept;
		const opinion_position& get_position() const noexcept;

		~voter();
	private:
		voter_character character;
		voter_trust trust;
		voter_situation situation;
		opinion_position opinion;
	};

	voter make_random_voter();
}

#endif
