#include "voter.hpp"

namespace voternn {
	voter::voter() {

	}

	// Copy constructor: copy the internal arrays so that sub-objects' references
	// are bound to this object's own storage (default copy would alias source storage).
	voter::voter(const voter& other) {
		for(size_t i = 0; i < 11; ++i) this->character.values[i] = other.character.values[i];
		for(size_t i = 0; i < 2; ++i) this->trust.values[i] = other.trust.values[i];
		for(size_t i = 0; i < 10; ++i) this->situation.values[i] = other.situation.values[i];
		for(size_t i = 0; i < 12; ++i) this->opinion.values[i] = other.opinion.values[i];
	}

	// Move constructor: copy internal arrays (references must bind to this object's storage).
	voter::voter(voter&& other) noexcept {
		for(size_t i = 0; i < 11; ++i) this->character.values[i] = other.character.values[i];
		for(size_t i = 0; i < 2; ++i) this->trust.values[i] = other.trust.values[i];
		for(size_t i = 0; i < 10; ++i) this->situation.values[i] = other.situation.values[i];
		for(size_t i = 0; i < 12; ++i) this->opinion.values[i] = other.opinion.values[i];
	}

	voter_character& voter::get_character() noexcept {
		return this->character;
	}
	voter_trust& voter::get_trust() noexcept {
		return this->trust;
	}
	voter_situation& voter::get_situation() noexcept {
		return this->situation;
	}
	opinion_position& voter::get_position() noexcept {
		return this->opinion;
	}

	voter::~voter() {

	}

	voter& voter::operator=(const voter& other) {
		if(this == &other) return *this;
		for(size_t i = 0; i < 11; ++i) this->character.values[i] = other.character.values[i];
		for(size_t i = 0; i < 2; ++i) this->trust.values[i] = other.trust.values[i];
		for(size_t i = 0; i < 10; ++i) this->situation.values[i] = other.situation.values[i];
		for(size_t i = 0; i < 12; ++i) this->opinion.values[i] = other.opinion.values[i];
		return *this;
	}

	voter& voter::operator=(voter&& other) noexcept {
		if(this == &other) return *this;
		for(size_t i = 0; i < 11; ++i) this->character.values[i] = other.character.values[i];
		for(size_t i = 0; i < 2; ++i) this->trust.values[i] = other.trust.values[i];
		for(size_t i = 0; i < 10; ++i) this->situation.values[i] = other.situation.values[i];
		for(size_t i = 0; i < 12; ++i) this->opinion.values[i] = other.opinion.values[i];
		return *this;
	}

	const voter_character& voter::get_character() const noexcept { return this->character; }
	const voter_trust& voter::get_trust() const noexcept { return this->trust; }
	const voter_situation& voter::get_situation() const noexcept { return this->situation; }
	const opinion_position& voter::get_position() const noexcept { return this->opinion; }

	voter make_random_voter() {
		std::uniform_real_distribution<double> one(0.0, 1.0);
		std::uniform_real_distribution<double> age(18.0, 90.0);
		// opinions: sample directly in -100..100 to avoid extra scaling steps
		std::uniform_real_distribution<double> opin(-100.0, 100.0);

		voter v;

		v.character.openness = one(random_generator);
		v.character.neuroticism = one(random_generator);
		v.character.consciencitiousness = one(random_generator);
		v.character.agreeableness = one(random_generator);
		v.character.extraversion = one(random_generator);
		v.character.intelligence = one(random_generator);
		v.character.racism = one(random_generator);
		v.character.skepticism = one(random_generator);
		v.character.ambition = one(random_generator);
		v.character.loyalty = one(random_generator);
		v.character.taxProgressiveness = one(random_generator);

		v.trust.local = one(random_generator);
		v.trust.national = one(random_generator);

		v.situation.income = one(random_generator);
		v.situation.job_satisfaction = one(random_generator);
		v.situation.age = age(random_generator);
		v.situation.education = one(random_generator);
		v.situation.crime_themselves = one(random_generator);
		v.situation.crime_on_him = one(random_generator);
		v.situation.tolerance = one(random_generator);
		v.situation.community = one(random_generator);
		v.situation.unemployment_time = one(random_generator);
		v.situation.personal_taxes_level = one(random_generator);

		v.opinion.security = opin(random_generator);
		v.opinion.risk_aversion = opin(random_generator);
		v.opinion.public_spending = opin(random_generator);
		v.opinion.cost_of_living = opin(random_generator);
		v.opinion.minorities_support = opin(random_generator);
		v.opinion.stability = opin(random_generator);
		v.opinion.job_security = opin(random_generator);
		v.opinion.future_outlook = opin(random_generator);
		v.opinion.equality_markets = opin(random_generator);
		v.opinion.nation_globe = opin(random_generator);
		v.opinion.liberty_authority = opin(random_generator);
		v.opinion.tradition_progress = opin(random_generator);

		return v;
	}
}
