#include "voter_profiles_loader.hpp"
#include "../include.hpp"
#include "math.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <iostream>
#include <unordered_map>

namespace voternn::util {
	namespace {
		// Parse CSV line into fields
		std::vector<std::string> parse_csv_line(const std::string& line) {
			std::vector<std::string> fields;
			std::stringstream ss(line);
			std::string field;
			
			while (std::getline(ss, field, ',')) {
				fields.push_back(field);
			}
			return fields;
		}
		
		// Safely convert string to double
		double safe_stod(const std::string& str, double default_val = 0.0) {
			try {
				return std::stod(str);
			} catch (...) {
				return default_val;
			}
		}
	}

	std::vector<voter_profile_data> voter_profiles_loader::load_from_csv(const std::string& filepath) {
		std::vector<voter_profile_data> profiles;
		std::ifstream file(filepath);
		
		if (!file.is_open()) {
			std::cerr << "Failed to open voter profiles CSV: " << filepath << std::endl;
			return profiles;
		}

		std::string line;
		
		// Skip header row
		if (!std::getline(file, line)) {
			std::cerr << "CSV file is empty" << std::endl;
			return profiles;
		}
		
		// Parse header to get column indices
		auto header_fields = parse_csv_line(line);
		
		// Create a map of column name to index
		std::unordered_map<std::string, size_t> column_index;
		for (size_t i = 0; i < header_fields.size(); ++i) {
			column_index[header_fields[i]] = i;
		}

		// Read data rows
		while (std::getline(file, line)) {
			if (line.empty()) continue;
			
			auto fields = parse_csv_line(line);
			if (fields.size() < 44) continue;  // Ensure we have all columns
			
			voter_profile_data profile;
			
			// Extract fields by column name
			auto get_field = [&](const std::string& name) -> double {
				auto it = column_index.find(name);
				if (it != column_index.end() && it->second < fields.size()) {
					return safe_stod(fields[it->second]);
				}
				return 0.0;
			};
			
			profile.openness = get_field("openness");
			profile.neuroticism = get_field("neuroticism");
			profile.consciencitiousness = get_field("conscientiousness");
			profile.agreeableness = get_field("agreeableness");
			profile.extraversion = get_field("extraversion");
			profile.intelligence = get_field("intelligence");
			profile.racism = get_field("racism");
			profile.skepticism = get_field("skepticism");
			profile.ambition = get_field("ambition");
			profile.loyalty = get_field("loyalty");
			
			profile.local = get_field("local");
			profile.national = get_field("national");
			
			profile.income = get_field("income");
			profile.job_satisfaction = get_field("job_satisfaction");
			profile.age = get_field("age");
			profile.education = get_field("education");
			profile.crime_themselves = get_field("crime_themselves");
			profile.crime_on_him = get_field("crime_on_him");
			profile.tolerance = get_field("tolerance");
			profile.community = get_field("community");
			profile.unemployment_time = get_field("unemployment_time");
			profile.personal_taxes_level = get_field("personal_taxes_level");
			
			profile.security = get_field("security") * 100.0;
			profile.risk_aversion = get_field("risk_aversion") * 100.0;
			profile.public_spending = get_field("public_spending") * 100.0;
			profile.cost_of_living = get_field("cost_of_living") * 100.0;
			profile.minorities_support = get_field("minorities_support") * 100.0;
			profile.stability = get_field("stability") * 100.0;
			profile.job_security = get_field("job_security") * 100.0;
			profile.future_outlook = get_field("future_outlook") * 100.0;
			profile.equality_markets = get_field("equality_markets") * 100.0;
			profile.nation_globe = get_field("nation_globe") * 100.0;
			profile.liberty_authority = get_field("liberty_authority") * 100.0;
			profile.tradition_progress = get_field("tradition_progress") * 100.0;
			
			profiles.push_back(profile);
		}
		
		file.close();
		std::cout << "Loaded " << profiles.size() << " voter profiles from CSV" << std::endl;
		if (!profiles.empty()) {
			std::cout << "  First profile: age=" << profiles[0].age 
			          << " security=" << profiles[0].security 
			          << " future_outlook=" << profiles[0].future_outlook << std::endl;
		}
		return profiles;
	}

	std::optional<voter_profile_data> voter_profiles_loader::get_random_profile(
		const std::vector<voter_profile_data>& profiles
	) {
		if (profiles.empty()) return std::nullopt;
		
		std::uniform_int_distribution<size_t> dist(0, profiles.size() - 1);
		return profiles[dist(::voternn::random_generator)];
	}

	void voter_profiles_loader::apply_profile_to_voter(voter& v, const voter_profile_data& profile) {
		// Apply character traits
		v.get_character().openness = profile.openness;
		v.get_character().neuroticism = profile.neuroticism;
		v.get_character().consciencitiousness = profile.consciencitiousness;
		v.get_character().agreeableness = profile.agreeableness;
		v.get_character().extraversion = profile.extraversion;
		v.get_character().intelligence = profile.intelligence;
		v.get_character().racism = profile.racism;
		v.get_character().skepticism = profile.skepticism;
		v.get_character().ambition = profile.ambition;
		v.get_character().loyalty = profile.loyalty;
		
		// Apply trust
		v.get_trust().local = profile.local;
		v.get_trust().national = profile.national;
		
		// Apply situation
		v.get_situation().income = profile.income;
		v.get_situation().job_satisfaction = profile.job_satisfaction;
		v.get_situation().age = profile.age;
		v.get_situation().education = profile.education;
		v.get_situation().crime_themselves = profile.crime_themselves;
		v.get_situation().crime_on_him = profile.crime_on_him;
		v.get_situation().tolerance = profile.tolerance;
		v.get_situation().community = profile.community;
		v.get_situation().unemployment_time = profile.unemployment_time;
		v.get_situation().personal_taxes_level = profile.personal_taxes_level;
		
		// Apply opinions - these are already in -100..100 range in the CSV
		auto& pos = v.get_position();
		pos.security = profile.security;
		pos.risk_aversion = profile.risk_aversion;
		pos.public_spending = profile.public_spending;
		pos.cost_of_living = profile.cost_of_living;
		pos.minorities_support = profile.minorities_support;
		pos.stability = profile.stability;
		pos.job_security = profile.job_security;
		pos.future_outlook = profile.future_outlook;
		pos.equality_markets = profile.equality_markets;
		pos.nation_globe = profile.nation_globe;
		pos.liberty_authority = profile.liberty_authority;
		pos.tradition_progress = profile.tradition_progress;
	}
}
