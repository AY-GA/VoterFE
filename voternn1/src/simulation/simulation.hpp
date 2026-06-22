#ifndef VOTERNN_SIMULATION
#define VOTERNN_SIMULATION
#include "../ui/uimgr.hpp"
#include "../callbacks/callbacks.hpp"
#include "../util/socket/listener.hpp"
#include "../util/file.hpp"
#include "../util/voter_profiles_loader.hpp"
#include "../types/voter.hpp"
#include "../types/district.hpp"
#include "../types/law.hpp"
#include "../types/media.hpp"
#include "../types/parliament.hpp"
#include "../types/grid.hpp"
#include "../types/company.hpp"
#include "../types/product.hpp"

namespace voternn {
	struct voter_identity {
		std::string name;
		std::string middlename;
		std::string surname;
	};

	/// @brief Main simulation instance.
	class simulation {
	public:
		simulation() = delete;
		simulation(const std::string_view token, const uint64_t width, const uint64_t height);

		void set_system(const electoral_system& system);
		void set_presidential_system(const presidential_electoral_system& system);

		/// @brief Increment the simulation.
		/// @param days How many days to increment it.
		void increment(const size_t days);

		/// @brief Resets the simulation.
		void reset();

		/// @brief Hold elections for the lower chamber of parliament, usually the house of reps.
		void hold_house_elections();
		/// @brief Hold elections for upper chamber of parliament, usually the senate.
		void hold_senate_elections();
		/// @brief Hold elections for president.
		void hold_presidential_elections();
		/// @brief Hold elections for local parliaments.
		void hold_local_elections();

		/// @brief Add new district.
		/// @param district New district.
		/// @returns District UUID
		std::string add_district(const district& district);

		/// @brief Emplace new district.
		/// @param district New district's name. Must be unique.
		/// @returns District UUID
		std::string emplace_district(const std::string_view district_name);

		/// @brief Replaces district with a new one.
		/// @param uuid UUID of district.
		/// @param newdistrict New data.
		void edit_district(const std::string_view uuid, const district& newdistrict);

		/// @brief Deletes a district.
		void delete_district(const std::string_view uuid);

		/// @brief Generates districts.
		/// @param enable_smoothing [optional] Enables smoothing pass.
		void generate_districts(const bool enable_smoothing = true);

		/// @brief Generates voters. One tile is one voter.
		void generate_voters();

		/// @brief Get new district by UUID. O(1) complexity.
		std::optional<std::reference_wrapper<district>> get_district_by_uuid(const std::string_view uuid);
		/// @brief Get new district by name. O(n) complexity. Use get_district_by_uuid when possible.
		std::optional<std::reference_wrapper<district>> get_district_name(const std::string_view name);

		/// @brief Get district uuid from district name. O(n) complexity.
		/// @param name District name.
		/// @return If exists, UUID of district.
		std::optional<std::string> get_uuid_from_name(const std::string_view name);

		/// @brief Gets simulation token.
		/// @return Simulation token.
		std::string get_token() const;

		/// @brief Writes simulation to file.
		/// @param filename Name of file.
		void save(std::string_view filename) const;
		/// @brief Loads simulation from file and overwrites current one.
		/// @param filename Name of file.
		void load(std::string_view filename);

		/// @brief Gets simulation grid.
		/// @return Reference to simulation grid.
		std::reference_wrapper<simulation_grid> get_grid();

		/// @brief Gets API listener.
		/// @return Reference to listener.
		std::reference_wrapper<listener> get_listener();

		/// @brief Gets map of simulated state as JSON.
		/// @return JSON representation.
		/// @see API Documentation.
		nlohmann::json get_map() const;

		nlohmann::json get_poll_json() const;

		/// @brief Run a poll and store it in history.
		nlohmann::json run_poll();

		/// @brief Get the last stored poll result, or an empty poll result if none.
		nlohmann::json get_last_poll() const;

		/// @brief Get all stored polls as a JSON array.
		nlohmann::json get_polls_json() const;
		std::optional<nlohmann::json> get_voter_json(const std::string_view uuid);
		nlohmann::json get_voters_json(const std::string_view search = "", const size_t limit = 50);
		std::string create_voter(const nlohmann::json& json);
		void edit_voter(const std::string_view uuid, const nlohmann::json& json);
		void delete_voter(const std::string_view uuid);

		std::string create_party(const nlohmann::json& json);
		void delete_party(const std::string_view uuid);

		void add_media(const std::string_view name);
		void add_law(const std::string_view text);
		void repeal_law(const std::string_view uuid);

		~simulation();
	private:
		listener api;
		simulation_grid grid;

		electoral_system system_parliament;
		presidential_electoral_system system_president;

		// Voter profiles from CSV
		std::vector<util::voter_profile_data> voter_profiles;

		//pair with UUID
		std::unordered_map<std::string, voter> voters_list;
		std::unordered_map<std::string, voter_identity> voter_identities;
		std::unordered_set<std::string> districts_name_list;
		std::unordered_map<std::string, district> districts_list;
		std::unordered_map<std::string, law> laws_list;
		std::unordered_map<std::string, media> media_list;
		std::unordered_map<std::string, party> party_list;
		std::unordered_map<std::string, company> company_list;
		std::unordered_map<std::string, product> product_list;
		std::vector<nlohmann::json> poll_history;
	};

	extern std::unordered_map<std::string, simulation> simulations;

	std::reference_wrapper<simulation> make_simulation(const uint64_t width, const uint64_t height);
}

#endif
