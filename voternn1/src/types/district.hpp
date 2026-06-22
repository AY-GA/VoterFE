#ifndef VOTERNN_TYPE_DISTRICT
#define VOTERNN_TYPE_DISTRICT
#include "parliament.hpp"
#include "voter.hpp"
#include "../system/electoral.hpp"

namespace voternn {
	struct district_statistics {
		double& crime_rate          = this->values[0];
		double& gdp_capita          = this->values[1];
		double& minority_percentage = this->values[2];
		double& freedom             = this->values[3];
		double& infrastructure      = this->values[4];
		double& life_expectancy     = this->values[5];
		double& price_food          = this->values[6];
		double& price_water         = this->values[7];
		double& price_energy        = this->values[8];
		double& price_housing       = this->values[9];
		double& tax_progressiveness = this->values[10];

		double values[11] {};
	};

	/// @brief Single electoral district.
	class district {
	public:
		district();
		district(std::string_view name);

		nlohmann::json get_data_as_json(const std::string_view uuid) const noexcept;

		std::string get_name() const noexcept;

		district_electoral_result run_elections_local();
		district_electoral_result run_elections_parliamentary(const electoral_system& system);
		district_electoral_result run_elections_presidential();

		~district();
	private:
		std::string name;
		uint8_t r, g, b;
		std::unordered_map<size_t, voter> voters;
		district_statistics statistics;
	};
}

#endif
