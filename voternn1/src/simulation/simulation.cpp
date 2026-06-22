#include "simulation.hpp"

namespace {
	nlohmann::json empty_poll_result() {
		return nlohmann::json::object({
			{"national", nlohmann::json::array()},
			{"districts", nlohmann::json::array()}
		});
	}
}

namespace voternn {
	namespace internal {
			static double normalize_opinion_value(double raw) {
				if(!std::isfinite(raw)) return 0.0;
				double a = std::abs(raw);
				// If input looks like a fraction (0..1), treat as percent
				if(a <= 1.0) return raw * 100.0;
				// If already within desired bounds, pass through
				if(a <= 100.0) return raw;
				// Otherwise apply smooth arctangent scaling to map large outliers into [-100,100]
				double scaled = 100.0 * (2.0 / M_PI) * std::atan(a / 100.0);
				return std::copysign(scaled, raw);
			}
		struct gridmap_object {
			size_t x;
			size_t y;
			std::string uuid;
		};

		void apply_opinion_from_json(const nlohmann::json& json, opinion_position& opinion) {
			const auto set = [&](const char* key, double& field) {
				if(json.contains(key) && json.at(key).is_number()) {
					double v = json.at(key).get<double>();
					field = normalize_opinion_value(v);
				}
			};

			set("security", opinion.security);
			set("risk_aversion", opinion.risk_aversion);
			set("public_spending", opinion.public_spending);
			set("cost_of_living", opinion.cost_of_living);
			set("minorities_support", opinion.minorities_support);
			set("stability", opinion.stability);
			set("job_security", opinion.job_security);
			set("future_outlook", opinion.future_outlook);
			set("equality_markets", opinion.equality_markets);
			set("nation_globe", opinion.nation_globe);
			set("liberty_authority", opinion.liberty_authority);
			set("tradition_progress", opinion.tradition_progress);
		}

		nlohmann::json opinion_to_json(const opinion_position& opinion) {
			return nlohmann::json::object({
				{"security", opinion.security},
				{"risk_aversion", opinion.risk_aversion},
				{"public_spending", opinion.public_spending},
				{"cost_of_living", opinion.cost_of_living},
				{"minorities_support", opinion.minorities_support},
				{"stability", opinion.stability},
				{"job_security", opinion.job_security},
				{"future_outlook", opinion.future_outlook},
				{"equality_markets", opinion.equality_markets},
				{"nation_globe", opinion.nation_globe},
				{"liberty_authority", opinion.liberty_authority},
				{"tradition_progress", opinion.tradition_progress}
			});
		}

		std::string lower_ascii(std::string value) {
			for(char& c : value) {
				if(c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
			}
			return value;
		}

		voter_identity make_generated_voter_identity(const size_t index) {
			static const std::array<std::string_view, 16> names = {
				"Alex", "Mira", "Sam", "Nora", "Jon", "Elena", "Tomas", "Rina",
				"Leo", "Ada", "Milan", "Sara", "Owen", "Lena", "Niko", "Iris"
			};
			static const std::array<std::string_view, 16> surnames = {
				"Novak", "Kovacs", "Smith", "Bauer", "Horvat", "Nagy", "Petrov", "Costa",
				"Rossi", "Kim", "Silva", "Brown", "Varga", "Muller", "Ivanov", "Garcia"
			};

			std::uniform_int_distribution<size_t> name_picker(0, names.size() - 1);
			std::uniform_int_distribution<size_t> surname_picker(0, surnames.size() - 1);

			voter_identity identity;
			identity.name = std::string(names[name_picker(random_generator)]);
			identity.middlename = "";
			identity.surname = std::string(surnames[surname_picker(random_generator)]);
			return identity;
		}

		size_t party_index_for_voter(const voter& voter_instance) {
			const opinion_position& opinion = voter_instance.get_position();
			const double reform = opinion.public_spending + opinion.minorities_support + opinion.liberty_authority;
			const double security = opinion.security + opinion.stability + opinion.risk_aversion;
			const double market = opinion.equality_markets + opinion.future_outlook + opinion.cost_of_living;
			const double tradition = opinion.tradition_progress + opinion.nation_globe + opinion.job_security;
			const std::array<double, 4> scores = { reform, security, market, tradition };

			size_t best = 0;
			for(size_t i = 1; i < scores.size(); i++) {
				if(scores[i] > scores[best]) best = i;
			}
			return best;
		}

		/// Compute a small opinion bias vector from a law text.
		opinion_position compute_law_bias(const std::string& rawtext) {
			opinion_position bias{};
			std::string text = lower_ascii(rawtext);
			auto contains = [&](const char* token) {
				return text.find(token) != std::string::npos;
			};

			if(contains("health") || contains("healthcare") || contains("spend") || contains("fund")) bias.public_spending += 6.0;
			if(contains("tax") || contains("taxes") || contains("market") || contains("privat")) bias.equality_markets += 5.0;
			if(contains("minor") || contains("discrimin") || contains("rights")) bias.minorities_support += 6.0;
			if(contains("secur") || contains("border") || contains("defen") || contains("police")) bias.security += 6.0;
			if(contains("job") || contains("wage") || contains("employ")) bias.job_security += 4.0;
			if(contains("tradit") || contains("culture")) bias.tradition_progress += 4.0;
			if(contains("libert") || contains("freedom")) bias.liberty_authority += 4.0;
			if(contains("cost") || contains("inflation") || contains("price")) bias.cost_of_living += -3.0;
			if(contains("future") || contains("outlook")) bias.future_outlook += 3.0;
			if(contains("stabil") || contains("stability")) bias.stability += 3.0;

			// If no clear keywords, return a very small random nudge
			bool any = false;
			for(size_t i = 0; i < 12; ++i) if(bias.values[i] != 0.0) { any = true; break; }
			if(!any) {
				std::uniform_real_distribution<double> rnd(-2.0, 2.0);
				for(size_t i = 0; i < 12; ++i) bias.values[i] = rnd(random_generator);
			}

			return bias;
		}
	}

	std::unordered_map<std::string, simulation> simulations;

	//multiple listeners at 1 port do work
	simulation::simulation(const std::string_view token, const uint64_t width, const uint64_t height)
		 : api(token, 8080), grid(width, height) {
		voternn::register_handlers(this->api);
		
		// Load voter profiles from CSV
		this->voter_profiles = util::voter_profiles_loader::load_from_csv("../voter_profiles_6100.csv");
		if (this->voter_profiles.empty()) {
			std::cerr << "Warning: No voter profiles loaded, voters will have default values" << std::endl;
		}

		// Create a couple of default laws so UI has examples to show and repeal
		{
			const std::string uuid1 = random_string(32);
			this->laws_list.emplace(uuid1, law{"Increase public healthcare funding by 5% to improve access and outcomes."});
			const std::string uuid2 = random_string(32);
			this->laws_list.emplace(uuid2, law{"Reduce corporate taxes by 2% to encourage business investment."});
		}

		// Create a default media provider
		{
			const std::string media_uuid = random_string(32);
			this->media_list.emplace(media_uuid, media{"State Broadcasting Network"});
		}
	}

	void simulation::set_system(const electoral_system& system) {
		this->system_parliament = system;
	}
	void simulation::set_presidential_system(const presidential_electoral_system& system) {
		this->system_president = system;
	}
	
	void simulation::increment(const size_t days) {
		
	}

	void simulation::reset() {
		this->voters_list.clear();
		this->voter_identities.clear();
		this->laws_list.clear();
		this->media_list.clear();
		this->party_list.clear();
		this->company_list.clear();
		this->product_list.clear();

		if(this->districts_list.empty()) this->emplace_district("District 1");

		this->generate_districts();
		this->generate_voters();
	}

	void simulation::hold_house_elections() {

	}
	void simulation::hold_senate_elections() {

	}
	void simulation::hold_presidential_elections() {

	}
	void simulation::hold_local_elections() {

	}

	std::string simulation::add_district(const district& district) {
		std::string uuid = random_string(32);
		this->districts_list.emplace(uuid, district);
		this->districts_name_list.emplace(district.get_name());
		return uuid;
	}
	std::string simulation::emplace_district(const std::string_view district_name) {
		std::string uuid = random_string(32);
		this->districts_list.emplace(uuid, district_name);
		this->districts_name_list.emplace(district_name);
		return uuid;
	}

	void simulation::edit_district(const std::string_view uuid, const district& newdistrict) {
		//this->districts_list[std::string(uuid)] = newdistrict;
	}

	void simulation::delete_district(const std::string_view uuid) {
		const std::string key(uuid);
		if(this->districts_list.contains(key))
			this->districts_name_list.erase(this->districts_list.at(key).get_name());
		this->districts_list.erase(key);
	}

	void simulation::generate_districts(const bool enable_smoothing) {
		::voternn::generate_districts(this->grid, this->districts_list);
	}

	void simulation::generate_voters() {
		for(uint64_t i = 0; i < this->grid.get_total(); i++) {
			const std::string uuid = random_string(32);
			voter v;
			
			// Use voter profile from CSV if available
			if (!this->voter_profiles.empty()) {
				auto profile = util::voter_profiles_loader::get_random_profile(this->voter_profiles);
				if (profile.has_value()) {
					util::voter_profiles_loader::apply_profile_to_voter(v, profile.value());
				}
			}
			
			// Debug: print first 3 generated voters' opinions to verify randomization
			if(i < 3) {
				std::cout << "Generated voter " << i << ": security=" << v.get_position().security
				          << " risk_aversion=" << v.get_position().risk_aversion
				          << " future_outlook=" << v.get_position().future_outlook
				          << " age=" << v.get_situation().age << std::endl;
			}
			this->voters_list.emplace(uuid, std::move(v));
			this->voter_identities.emplace(uuid, internal::make_generated_voter_identity(i));
		}
	}

	std::optional<std::reference_wrapper<district>> simulation::get_district_by_uuid(const std::string_view uuid) {
		if(!this->districts_list.contains(std::string(uuid))) return std::nullopt;
		return std::make_optional(std::ref(this->districts_list.at(std::string(uuid))));
	}

	std::optional<std::reference_wrapper<district>> simulation::get_district_name(const std::string_view name) {
		for(auto& d : this->districts_list)
			if(d.second.get_name() == name) return std::make_optional(std::ref(d.second));
		return std::nullopt;
	}

	std::optional<std::string> simulation::get_uuid_from_name(const std::string_view name) {
		if(!this->districts_name_list.contains(std::string(name))) return std::nullopt;
		for(auto& d : this->districts_list)
			if(d.second.get_name() == name) return std::make_optional(d.first);
		return std::nullopt;
	}

	std::string simulation::get_token() const {
		return this->api.get_token();
	}

	void simulation::save(std::string_view filename) const {
		
	}
	void simulation::load(std::string_view filename) {
		
	}

	std::reference_wrapper<simulation_grid> simulation::get_grid() {
		return std::ref(this->grid);
	}

	std::reference_wrapper<listener> simulation::get_listener() {
		return std::ref(this->api);
	}
	
	nlohmann::json simulation::get_map() const {
		nlohmann::json result;
		result["gridmapping"] = nlohmann::json::array();
		result["districts"] = nlohmann::json::array();
		result["xs"] = this->grid.get_width();
		result["ys"] = this->grid.get_height();

		for(const auto& d : this->districts_list)
			result["districts"].push_back(d.second.get_data_as_json(d.first));

		for(uint64_t i = 0; i < this->grid.get_total(); i++) {
			nlohmann::json gmobject;
			const uint64_t x = i % this->grid.get_width();
			const uint64_t y = i / this->grid.get_width();
			gmobject["x"] = x;
			gmobject["y"] = y;
			const auto allegiance = this->grid.get_allegiance(x, y);
			gmobject["uuid"] = allegiance.has_value() ? allegiance.value() : "";

			result["gridmapping"].push_back(gmobject);
		} 

		// Expose media providers in the map payload so frontend can list them
		result["media"] = nlohmann::json::array();
		for(const auto& m : this->media_list) {
			nlohmann::json obj;
			obj["uuid"] = m.first;
			obj["name"] = m.second.name;
			result["media"].push_back(obj);
		}

		// Expose laws so frontend can show existing laws and their UUIDs
		result["laws"] = nlohmann::json::array();
		for(const auto& l : this->laws_list) {
			nlohmann::json obj;
			obj["uuid"] = l.first;
			obj["text"] = l.second.text;
			result["laws"].push_back(obj);
		}

		return result;
	}

		nlohmann::json simulation::get_poll_json() const {
		constexpr size_t party_count = 4;
		static const std::array<std::string_view, party_count> party_ids = {
			"civic_reform", "security_front", "market_union", "local_tradition"
		};
		static const std::array<std::string_view, party_count> party_names = {
			"Civic Reform", "Security Front", "Market Union", "Local Tradition"
		};

		std::array<uint64_t, party_count> votes = {};
		for(auto& row : this->voters_list) {
			votes[internal::party_index_for_voter(row.second)]++;
		}

		uint64_t total_votes = 0;
		for(const uint64_t count : votes) total_votes += count;
		if(total_votes == 0) total_votes = 1;

		nlohmann::json result;
		result["national"] = nlohmann::json::array();
		result["districts"] = nlohmann::json::array();

		std::array<double, party_count> national_percentages = {};
		for(size_t i = 0; i < party_ids.size(); i++) {
			national_percentages[i] = (static_cast<double>(votes[i]) / static_cast<double>(total_votes)) * 100.0;
			result["national"].push_back(nlohmann::json::object({
				{"party_uuid", party_ids[i]},
				{"party_name", party_names[i]},
				{"percentage", national_percentages[i]},
				{"votes", votes[i]}
			}));
		}

		for(const auto& district_row : this->districts_list) {
			nlohmann::json district_result;
			district_result["uuid"] = district_row.first;
			district_result["district_uuid"] = district_row.first;
			district_result["name"] = district_row.second.get_name();
			district_result["results"] = nlohmann::json::array();

			size_t hash = 0;
			for(const char c : district_row.first) hash += static_cast<unsigned char>(c);

			std::array<double, party_count> district_percentages = {};
			double district_total = 0.0;
			for(size_t i = 0; i < party_ids.size(); i++) {
				const double bias = static_cast<double>((hash + (i * 17)) % 13) - 6.0;
				district_percentages[i] = std::max(0.0, national_percentages[i] + bias);
				district_total += district_percentages[i];
			}
			if(district_total <= 0.0) district_total = 1.0;

			for(size_t i = 0; i < party_ids.size(); i++) {
				const double normalized = (district_percentages[i] / district_total) * 100.0;
				district_result["results"].push_back(nlohmann::json::object({
					{"party_uuid", party_ids[i]},
					{"party_name", party_names[i]},
					{"percentage", normalized}
				}));
			}

			result["districts"].push_back(district_result);
		}

		return result;
	}

		nlohmann::json simulation::run_poll() {
			auto result = this->get_poll_json();
			this->poll_history.push_back(result);
			return result;
		}

		nlohmann::json simulation::get_last_poll() const {
			if(this->poll_history.empty()) return empty_poll_result();
			return this->poll_history.back();
		}

		nlohmann::json simulation::get_polls_json() const {
			nlohmann::json polls = nlohmann::json::array();
			if(this->poll_history.empty()) {
				polls.push_back(this->get_poll_json());
				return polls;
			}
			for(const auto &p : this->poll_history) polls.push_back(p);
			return polls;
		}

	std::optional<nlohmann::json> simulation::get_voter_json(const std::string_view uuid) {
		const std::string key(uuid);
		if(!this->voters_list.contains(key)) return std::nullopt;

		voter& voter_instance = this->voters_list.at(key);
		nlohmann::json result = internal::opinion_to_json(voter_instance.get_position());
		
		// Debug: log stored opinion values before sanitization
		std::cout << "get_voter_json(" << key.substr(0, 8) << "): raw security=" << voter_instance.get_position().security
		          << " risk_aversion=" << voter_instance.get_position().risk_aversion
		          << " future_outlook=" << voter_instance.get_position().future_outlook << std::endl;
		
		result["voter_uuid"] = key;

		if(this->voter_identities.contains(key)) {
			const voter_identity& identity = this->voter_identities.at(key);
			result["name"] = identity.name;
			result["middlename"] = identity.middlename;
			result["surname"] = identity.surname;
		}
		else {
			result["name"] = "";
			result["middlename"] = "";
			result["surname"] = "";
		}

		// Sanitize age: ensure finite and within sensible bounds
		double age_val = voter_instance.get_situation().age;
		if(!std::isfinite(age_val) || age_val < 0.0 || age_val > 1000.0) {
			// fallback to a reasonable default
			age_val = 30.0;
		}
		age_val = std::clamp(age_val, 0.0, 100.0);
		result["age"] = age_val;

		// Sanitize opinion values to avoid tiny/NaN/inf leaking to the UI
		for(const char* key : {"security","risk_aversion","public_spending","cost_of_living",
							   "minorities_support","stability","job_security","future_outlook",
							   "equality_markets","nation_globe","liberty_authority","tradition_progress"}) {
			if(result.contains(key)) {
				double v = result.at(key).get<double>();
				if(!std::isfinite(v) || std::abs(v) < 1e-6) v = 0.0;
				// keep opinion values in -100..100 if they accidentally exceed expected bounds
				if(std::abs(v) > 1000.0) v = std::copysign(100.0, v);
				result[key] = v;
			}
		}
		
		// Debug: log after sanitization
		std::cout << "  after sanitize: security=" << result.at("security")
		          << " risk_aversion=" << result.at("risk_aversion") 
		          << " future_outlook=" << result.at("future_outlook")
		          << " age=" << result.at("age") << std::endl;
		
		// Print all opinion values
		std::cout << "  ALL opinions: ";
		for(const char* key : {"security","risk_aversion","public_spending","cost_of_living",
							   "minorities_support","stability","job_security","future_outlook",
							   "equality_markets","nation_globe","liberty_authority","tradition_progress"}) {
			if(result.contains(key)) {
				std::cout << key << "=" << result.at(key) << " ";
			}
		}
		std::cout << std::endl;

		// If all opinion values are zero, generate sensible random defaults and persist them
		bool any_opinion_nonzero = false;
		for(const char* key : {"security","risk_aversion","public_spending","cost_of_living",
		                       "minorities_support","stability","job_security","future_outlook",
		                       "equality_markets","nation_globe","liberty_authority","tradition_progress"}) {
			if(result.contains(key) && std::abs(result.at(key).get<double>()) > 1e-6) {
				any_opinion_nonzero = true;
				break;
			}
		}
		if(!any_opinion_nonzero) {
			std::cout << "  FALLBACK: all opinions zero, generating random values" << std::endl;
			std::uniform_real_distribution<double> dist(-100.0, 100.0);
			auto& writable_pos = const_cast<opinion_position&>(voter_instance.get_position());
			for(const char* key : {"security","risk_aversion","public_spending","cost_of_living",
			                       "minorities_support","stability","job_security","future_outlook",
			                       "equality_markets","nation_globe","liberty_authority","tradition_progress"}) {
				double v = dist(random_generator);
				// persist into stored voter and result
				if(std::strcmp(key, "security") == 0) writable_pos.security = v;
				else if(std::strcmp(key, "risk_aversion") == 0) writable_pos.risk_aversion = v;
				else if(std::strcmp(key, "public_spending") == 0) writable_pos.public_spending = v;
				else if(std::strcmp(key, "cost_of_living") == 0) writable_pos.cost_of_living = v;
				else if(std::strcmp(key, "minorities_support") == 0) writable_pos.minorities_support = v;
				else if(std::strcmp(key, "stability") == 0) writable_pos.stability = v;
				else if(std::strcmp(key, "job_security") == 0) writable_pos.job_security = v;
				else if(std::strcmp(key, "future_outlook") == 0) writable_pos.future_outlook = v;
				else if(std::strcmp(key, "equality_markets") == 0) writable_pos.equality_markets = v;
				else if(std::strcmp(key, "nation_globe") == 0) writable_pos.nation_globe = v;
				else if(std::strcmp(key, "liberty_authority") == 0) writable_pos.liberty_authority = v;
				else if(std::strcmp(key, "tradition_progress") == 0) writable_pos.tradition_progress = v;
				result[key] = v;
			}
			// also ensure age stored is sensible
			double stored_age = voter_instance.get_situation().age;
			if(!std::isfinite(stored_age) || stored_age <= 0.0) {
				std::uniform_real_distribution<double> age_dist(18.0, 90.0);
				double a = age_dist(random_generator);
				voter_instance.get_situation().age = std::clamp(a, 0.0, 100.0);
				result["age"] = voter_instance.get_situation().age;
			}
		}
		return result;
	}

	nlohmann::json simulation::get_voters_json(const std::string_view search, const size_t limit) {
		const std::string needle = internal::lower_ascii(std::string(search));
		const size_t safe_limit = std::min<size_t>(std::max<size_t>(limit, 1), 200);
		size_t matched = 0;

		nlohmann::json result;
		result["voters"] = nlohmann::json::array();

		std::cout << "get_voters_json(): voters_list.size()=" << this->voters_list.size() << std::endl;

		for(auto& row : this->voters_list) {
			std::string haystack = row.first;
			if(this->voter_identities.contains(row.first)) {
				const voter_identity& identity = this->voter_identities.at(row.first);
				haystack += " " + identity.name + " " + identity.middlename + " " + identity.surname;
			}

			if(!needle.empty() && internal::lower_ascii(haystack).find(needle) == std::string::npos) {
				continue;
			}

			matched++;
			if(result["voters"].size() < safe_limit) {
				std::cout << "  Getting voter " << row.first.substr(0, 8) << " (total collected: " << result["voters"].size() << ")" << std::endl;
				auto voter_json = this->get_voter_json(row.first);
				if(voter_json.has_value()) {
					result["voters"].push_back(voter_json.value());
					std::cout << "    -> Added voter, security=" << voter_json.value().at("security") << " age=" << voter_json.value().at("age") << std::endl;
				} else {
					std::cout << "    -> get_voter_json returned nullopt!" << std::endl;
				}
			}
		}

		result["total"] = matched;
		std::cout << "get_voters_json(): returning " << result["voters"].size() << " voters, total=" << matched << std::endl;
		return result;
	}

	std::string simulation::create_voter(const nlohmann::json& json) {
		const std::string uuid = random_string(32);
		voter voter_instance;
		internal::apply_opinion_from_json(json, voter_instance.get_position());

		// If the client did not provide any meaningful opinion values (all zeros),
		// generate small random opinion values so voters are not all identical.
		{
			const opinion_position& pos = voter_instance.get_position();
			bool any_nonzero = (
				pos.security != 0.0 || pos.risk_aversion != 0.0 || pos.public_spending != 0.0 ||
				pos.cost_of_living != 0.0 || pos.minorities_support != 0.0 || pos.stability != 0.0 ||
				pos.job_security != 0.0 || pos.future_outlook != 0.0 || pos.equality_markets != 0.0 ||
				pos.nation_globe != 0.0 || pos.liberty_authority != 0.0 || pos.tradition_progress != 0.0
			);
			if(!any_nonzero) {
				auto& writable = const_cast<opinion_position&>(voter_instance.get_position());
				std::uniform_real_distribution<double> dist(-100.0, 100.0);
				writable.security = internal::normalize_opinion_value(dist(random_generator));
				writable.risk_aversion = internal::normalize_opinion_value(dist(random_generator));
				writable.public_spending = internal::normalize_opinion_value(dist(random_generator));
				writable.cost_of_living = internal::normalize_opinion_value(dist(random_generator));
				writable.minorities_support = internal::normalize_opinion_value(dist(random_generator));
				writable.stability = internal::normalize_opinion_value(dist(random_generator));
				writable.job_security = internal::normalize_opinion_value(dist(random_generator));
				writable.future_outlook = internal::normalize_opinion_value(dist(random_generator));
				writable.equality_markets = internal::normalize_opinion_value(dist(random_generator));
				writable.nation_globe = internal::normalize_opinion_value(dist(random_generator));
				writable.liberty_authority = internal::normalize_opinion_value(dist(random_generator));
				writable.tradition_progress = internal::normalize_opinion_value(dist(random_generator));
			}
		}

		if(json.contains("age") && json.at("age").is_number()) {
			double a = json.at("age").get<double>();
			a = std::clamp(a, 0.0, 100.0);
			voter_instance.get_situation().age = a;
		}

		voter_identity identity;
		if(json.contains("name") && json.at("name").is_string()) identity.name = json.at("name").get<std::string>();
		if(json.contains("middlename") && json.at("middlename").is_string()) identity.middlename = json.at("middlename").get<std::string>();
		if(json.contains("surname") && json.at("surname").is_string()) identity.surname = json.at("surname").get<std::string>();

		this->voters_list.emplace(uuid, std::move(voter_instance));
		this->voter_identities.emplace(uuid, std::move(identity));
		return uuid;
	}

	void simulation::edit_voter(const std::string_view uuid, const nlohmann::json& json) {
		const std::string key(uuid);
		if(!this->voters_list.contains(key)) return;

		voter& voter_instance = this->voters_list.at(key);
		internal::apply_opinion_from_json(json, voter_instance.get_position());

		if(json.contains("age") && json.at("age").is_number()) {
			double a = json.at("age").get<double>();
			a = std::clamp(a, 0.0, 100.0);
			voter_instance.get_situation().age = a;
		}

		voter_identity identity;
		if(this->voter_identities.contains(key)) identity = this->voter_identities.at(key);
		if(json.contains("name") && json.at("name").is_string()) identity.name = json.at("name").get<std::string>();
		if(json.contains("middlename") && json.at("middlename").is_string()) identity.middlename = json.at("middlename").get<std::string>();
		if(json.contains("surname") && json.at("surname").is_string()) identity.surname = json.at("surname").get<std::string>();
		this->voter_identities[key] = std::move(identity);
	}

	void simulation::delete_voter(const std::string_view uuid) {
		const std::string key(uuid);
		this->voters_list.erase(key);
		this->voter_identities.erase(key);
	}

	std::string simulation::create_party(const nlohmann::json& json) {
		const std::string uuid = random_string(32);
		this->party_list.emplace(uuid, party{});
		return uuid;
	}

	void simulation::delete_party(const std::string_view uuid) {
		this->party_list.erase(std::string(uuid));
	}

	void simulation::add_media(const std::string_view name) {
		const std::string uuid = random_string(32);
		this->media_list.emplace(uuid, media{std::string(name)});
	}

	void simulation::edit_media(const std::string_view uuid, const std::string_view name) {
		const std::string key(uuid);
		if(this->media_list.contains(key)) {
			this->media_list.at(key).name = std::string(name);
		}
	}

	void simulation::delete_media(const std::string_view uuid) {
		const std::string key(uuid);
		this->media_list.erase(key);
	}

	void simulation::add_law(const std::string_view text) {
 		const std::string uuid = random_string(32);
 		this->laws_list.emplace(uuid, law{std::string(text)});

		// Apply small opinion changes to voters based on the law text.
		try {
			auto bias = internal::compute_law_bias(std::string(text));
			std::uniform_real_distribution<double> scale_dist(0.4, 0.9);
			for(auto& row : this->voters_list) {
				auto& voter_instance = row.second;
				auto& pos = const_cast<opinion_position&>(voter_instance.get_position());
				double scale = scale_dist(random_generator) * 0.2; // keep nudges small
				for(size_t i = 0; i < 12; ++i) {
					double delta = bias.values[i] * scale;
					pos.values[i] = std::clamp(pos.values[i] + delta, -100.0, 100.0);
				}
			}
		} catch(...) {
			// Ignore influence failures
		}
	}

	void simulation::repeal_law(const std::string_view uuid) {
		this->laws_list.erase(std::string(uuid));
	}

	simulation::~simulation() {}

	std::reference_wrapper<simulation> make_simulation(const uint64_t width, const uint64_t height) {
		std::string token = random_string(16);
		simulations.emplace(token, simulation(token, width, height));
		return std::ref(simulations.at(token));
	}

}
