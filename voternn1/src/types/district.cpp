#include "district.hpp"

namespace voternn {
	district::district() : name(""), r(0), g(0), b(0) {

	}
	district::district(std::string_view name) : name(name) {
		std::uniform_int_distribution c(0,255);
		this->r = c(random_generator);
		this->g = c(random_generator);
		this->b = c(random_generator);
	}

	nlohmann::json district::get_data_as_json(const std::string_view uuid) const noexcept {
		nlohmann::json nj;

		nj["uuid"] = uuid;
		nj["name"] = this->name;
		nj["r"] = this->r;
		nj["g"] = this->g;
		nj["b"] = this->b;
		nj["crime_rate"] = this->statistics.crime_rate;
		nj["gdp_capita"] = this->statistics.gdp_capita;
		nj["minority_percentage"] = this->statistics.minority_percentage;
		nj["freedom"] = this->statistics.freedom;
		nj["infrastructure"] = this->statistics.infrastructure;
		nj["life_expectancy"] = this->statistics.life_expectancy;
		nj["price_food"] = this->statistics.price_food;
		nj["price_water"] = this->statistics.price_water;
		nj["price_energy"] = this->statistics.price_energy;
		nj["price_housing"] = this->statistics.price_housing;
		nj["tax_progressiveness"] = this->statistics.tax_progressiveness;

		return nj;
	}

	std::string district::get_name() const noexcept {
		return this->name;
	}

	district_electoral_result district::run_elections_local() {
		return {};
	}
	district_electoral_result district::run_elections_parliamentary(const electoral_system& system) {
		return {};
	}
	district_electoral_result district::run_elections_presidential() {
		return {};
	}

	district::~district() {

	}
}
