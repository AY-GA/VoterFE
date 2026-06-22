#ifndef VOTERNN_UTIL_VOTER_PROFILES_LOADER
#define VOTERNN_UTIL_VOTER_PROFILES_LOADER

#include "../types/voter.hpp"
#include "../types/opinion.hpp"
#include <vector>
#include <string>
#include <optional>

namespace voternn::util {
	struct voter_profile_data {
		// character traits
		double openness, neuroticism, consciencitiousness, agreeableness, extraversion;
		double intelligence, racism, skepticism, ambition, loyalty;
		// trust
		double local, national;
		// situation
		double income, job_satisfaction, age, education;
		double crime_themselves, crime_on_him, tolerance, community;
		double unemployment_time, personal_taxes_level;
		// opinions
		double security, risk_aversion, public_spending, cost_of_living;
		double minorities_support, stability, job_security, future_outlook;
		double equality_markets, nation_globe, liberty_authority, tradition_progress;
	};

	class voter_profiles_loader {
	public:
		/// Load voter profiles from CSV file
		static std::vector<voter_profile_data> load_from_csv(const std::string& filepath);
		
		/// Get a random profile (uses random_generator)
		static std::optional<voter_profile_data> get_random_profile(
			const std::vector<voter_profile_data>& profiles
		);
		
		/// Apply profile data to a voter
		static void apply_profile_to_voter(voter& v, const voter_profile_data& profile);
	};
}

#endif
