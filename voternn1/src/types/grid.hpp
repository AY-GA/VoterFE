#ifndef VOTERNN_TYPE_GRID
#define VOTERNN_TYPE_GRID
#include "district.hpp"

namespace voternn {
	/// @brief Represenation of simulation map.
	class simulation_grid {
	public:
		simulation_grid(const uint64_t width, const uint64_t height);

		void change_allegiance(const size_t x, const size_t y, std::string_view newdistrict);

		std::optional<std::string> get_allegiance(const size_t x, const size_t y) const noexcept;

		bool validate_grid() const noexcept;

		size_t get_width() const noexcept;
		size_t get_height() const noexcept;
		size_t get_total() const noexcept;

		~simulation_grid();
	private:
		std::vector<std::vector<std::optional<std::string>>> allegiance;
		size_t width, height;
	};

	std::ostream& operator<<(std::ostream& s, const simulation_grid& sg);

	/// @brief District generator function.
	/// @param grid Simulation grid, map.
	/// @param districts Array of districts' UUIDs for map to be generated from. 
	/// @param enable_smoothing Whether or not to enable smoothing pass.
	/// @returns Nothing, all data written to grid reference.
	void generate_districts(simulation_grid& grid, const std::unordered_map<std::string, district>& districts, const bool enable_smoothing = true);
};

#endif