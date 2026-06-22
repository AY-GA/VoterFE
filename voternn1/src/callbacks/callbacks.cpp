#include "callbacks.hpp"

namespace voternn::api {
	namespace {
		std::optional<std::string> get_string(const nlohmann::json& json, const std::string& key) {
			if(!json.contains(key)) return std::nullopt;
			if(!json.at(key).is_string()) return std::nullopt;
			return json.at(key).get<std::string>();
		}

		std::optional<uint64_t> get_uint(const nlohmann::json& json, const std::string& key) {
			if(!json.contains(key)) return std::nullopt;
			if(!json.at(key).is_number_unsigned() && !json.at(key).is_number_integer()) return std::nullopt;
			const int64_t value = json.at(key).get<int64_t>();
			if(value < 0) return std::nullopt;
			return static_cast<uint64_t>(value);
		}

		nlohmann::json empty_poll_result() {
			return nlohmann::json::object({
				{"national", nlohmann::json::array()},
				{"districts", nlohmann::json::array()}
			});
		}
	}

	namespace simulation {
		webserver_handler_return_t map(API_PARAMS) {
			return simulations.at(token).get_map();
		}
		webserver_handler_return_t increment(API_PARAMS) {
			const auto days = get_uint(json, "days");
			if(!days.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).increment(days.value());
			return std::monostate{};
		}
		webserver_handler_return_t reset(API_PARAMS) {
			simulations.at(token).reset();
			return std::monostate{};
		}
		webserver_handler_return_t snap(API_PARAMS) {
			const auto param = get_string(json, "type");
			if(!param.has_value()) return handler_client_error_t::NO_PARAM;

			if(param.value() == "general") {
				simulations.at(token).hold_house_elections();
				simulations.at(token).hold_senate_elections();
			}
			else if(param.value() == "president") simulations.at(token).hold_presidential_elections();
			else if(param.value() == "parliament") simulations.at(token).hold_house_elections();
			else if(param.value() == "senate") simulations.at(token).hold_senate_elections();
			else if(param.value() == "local") simulations.at(token).hold_local_elections();
			else if(param.value() == "all") {
				simulations.at(token).hold_local_elections();
				simulations.at(token).hold_house_elections();
				simulations.at(token).hold_senate_elections();
				simulations.at(token).hold_presidential_elections();
			}
			else {
				return handler_client_error_t::INVALID_PARAM;
			}
			// After calling the appropriate election routines, run a poll and store it
			try {
				auto poll = simulations.at(token).run_poll();
				std::cout << "Snap election produced poll: " << poll << std::endl;
			} catch(...) {
				// ignore poll storage failures
			}
			return std::monostate{};
		}
		webserver_handler_return_t system(API_PARAMS) {
			const auto param = get_string(json, "system");
			if(!param.has_value()) return handler_client_error_t::NO_PARAM;

			if(param.value() == "d_hondt") 
				simulations.at(token).set_system(electoral_systems::d_hondt());
			else if(param.value() == "fptp")
				simulations.at(token).set_system(electoral_systems::fptp());
			else if(param.value() == "hungry_orbman")
				simulations.at(token).set_system(electoral_systems::hungry_orbman());
			else if(param.value() == "perfect")
				simulations.at(token).set_system(electoral_systems::perfect());
			else if(param.value() == "stv")
				simulations.at(token).set_system(electoral_systems::stv());
			else {
				return handler_client_error_t::INVALID_PARAM;
			}

			return std::monostate{};
		}
		webserver_handler_return_t presidential(API_PARAMS) {
			const auto param = get_string(json, "system");
			if(!param.has_value()) return handler_client_error_t::NO_PARAM;

			if(param.value() == "electoral_college") 
				simulations.at(token).set_presidential_system(presidential_electoral_systems::electoral_college());
			else if(param.value() == "two_round")
				simulations.at(token).set_presidential_system(presidential_electoral_systems::two_round());
			else {
				return handler_client_error_t::INVALID_PARAM;
			}

			return std::monostate{};
		}
	}

	namespace polling {
				webserver_handler_return_t get(API_PARAMS) {
					// Return stored polls history (or a single computed poll if none stored)
					auto polls = simulations.at(token).get_polls_json();
					return nlohmann::json::object({{"polls", polls}});
				}
				webserver_handler_return_t last(API_PARAMS) {
					auto res = simulations.at(token).get_last_poll();
					std::cout << "Polling last: " << res << std::endl;
					return res;
				}
				webserver_handler_return_t run(API_PARAMS) {
					auto res = simulations.at(token).run_poll();
					std::cout << "Polling run: " << res << std::endl;
					return res;
				}
	}

	namespace voter {
		webserver_handler_return_t list(API_PARAMS) {
			const auto search = get_string(json, "search").value_or("");
			const auto limit = get_uint(json, "limit").value_or(50);
			return simulations.at(token).get_voters_json(search, limit);
		}
		webserver_handler_return_t get(API_PARAMS) {
			const auto uuid = get_string(json, "voter_uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			auto voter = simulations.at(token).get_voter_json(uuid.value());
			if(!voter.has_value()) return handler_client_error_t::INVALID_PARAM;
			return voter.value();
		}
		webserver_handler_return_t create(API_PARAMS) {
				{
					const std::string uuid = simulations.at(token).create_voter(json);
					std::cout << "Created voter: " << uuid << std::endl;
					return nlohmann::json::object({{"voter_uuid", uuid}});
				}
		}
		webserver_handler_return_t edit(API_PARAMS) {
			const auto uuid = get_string(json, "voter_uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).edit_voter(uuid.value(), json);
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			const auto uuid = get_string(json, "voter_uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).delete_voter(uuid.value());
			return std::monostate{};
		}
	}

	namespace district {
		webserver_handler_return_t get(API_PARAMS) {
			auto uuid = get_string(json, "district_uuid");
			if(!uuid.has_value()) uuid = get_string(json, "uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			auto district = simulations.at(token).get_district_by_uuid(uuid.value());
			if(!district.has_value()) return handler_client_error_t::INVALID_PARAM;

			return district.value().get().get_data_as_json(uuid.value());
		}
		webserver_handler_return_t create(API_PARAMS) {
			const auto name = get_string(json, "name");
			if(!name.has_value()) return handler_client_error_t::NO_PARAM;

			const std::string uuid = simulations.at(token).emplace_district(name.value());
			return nlohmann::json::object({{"district_uuid", uuid}});
		}
		webserver_handler_return_t edit(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			auto uuid = get_string(json, "district_uuid");
			if(!uuid.has_value()) uuid = get_string(json, "uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).delete_district(uuid.value());
			return std::monostate{};
		}
		webserver_handler_return_t nuke(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t merge(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t reassign(API_PARAMS) {
			auto uuid = get_string(json, "district_uuid");
			if(!uuid.has_value()) uuid = get_string(json, "uuid");
			const auto x = get_uint(json, "x");
			const auto y = get_uint(json, "y");
			if(!uuid.has_value() || !x.has_value() || !y.has_value()) return handler_client_error_t::NO_PARAM;

			if(!simulations.at(token).get_district_by_uuid(uuid.value()).has_value())
				return handler_client_error_t::INVALID_PARAM;

			simulations.at(token).get_grid().get().change_allegiance(x.value(), y.value(), uuid.value());
			return std::monostate{};
		}
	}

	namespace party {
		webserver_handler_return_t get(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t create(API_PARAMS) {
			const std::string party_uuid = simulations.at(token).create_party(json);
			return nlohmann::json::object({{"party_uuid", party_uuid}});
		}
		webserver_handler_return_t edit(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			const auto uuid = get_string(json, "party_uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).delete_party(uuid.value());
			return std::monostate{};
		}
		webserver_handler_return_t merge(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace company {
		webserver_handler_return_t create(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t edit(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace product {
		webserver_handler_return_t create(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t edit(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace statistics {
		webserver_handler_return_t edit_gdp(API_PARAMS) {
			return std::monostate{};
		}

	}

	namespace crises {
		webserver_handler_return_t create(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t del(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace media {
		webserver_handler_return_t create(API_PARAMS) {
			const auto name = get_string(json, "name");
			if(!name.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).add_media(name.value());
			return std::monostate{};
		}
			webserver_handler_return_t edit(API_PARAMS) {
				const auto uuid = get_string(json, "uuid");
				const auto name = get_string(json, "name");
				if(!uuid.has_value() || !name.has_value()) return handler_client_error_t::NO_PARAM;

				simulations.at(token).edit_media(uuid.value(), name.value());
				return std::monostate{};
			}
			webserver_handler_return_t del(API_PARAMS) {
				const auto uuid = get_string(json, "uuid");
				if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

				simulations.at(token).delete_media(uuid.value());
				return std::monostate{};
			}
	}

	namespace law {
		webserver_handler_return_t add(API_PARAMS) {
			const auto text = get_string(json, "text");
			if(!text.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).add_law(text.value());
			return std::monostate{};
		}
		webserver_handler_return_t force(API_PARAMS) {
			const auto text = get_string(json, "text");
			if(!text.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).add_law(text.value());
			return std::monostate{};
		}
		webserver_handler_return_t repeal(API_PARAMS) {
			const auto uuid = get_string(json, "law_uuid");
			if(!uuid.has_value()) return handler_client_error_t::NO_PARAM;

			simulations.at(token).repeal_law(uuid.value());
			return std::monostate{};
		}
		webserver_handler_return_t add_article(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t repeal_article(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace constitution {
		webserver_handler_return_t add_article(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t remove_article(API_PARAMS) {
			return std::monostate{};
		}
		webserver_handler_return_t redraft(API_PARAMS) {
			return std::monostate{};
		}
	}

	namespace misc {
		webserver_handler_return_t version(API_PARAMS) {
			return nlohmann::json::object({ 
				{"version_string", "VoterNN Reference Backend " VNN_STRINGIFY(VNN_MAJOR) "." VNN_STRINGIFY(VNN_MINOR) "." VNN_STRINGIFY(VNN_BUILD)},
				{ "major", VNN_MAJOR },
				{ "minor", VNN_MINOR },
				{ "build", VNN_BUILD }
			});
		}
	}
}

namespace voternn {
	void register_handlers(listener& listener) {
		listener
			.register_handler(http_method_t::GET, "/version/", api::misc::version, {})
			.register_handler(http_method_t::POST, "/map/", api::simulation::map, {})
			.register_handler(http_method_t::POST, "/increment/", api::simulation::increment, {})
			.register_handler(http_method_t::POST, "/reset/", api::simulation::reset, {})
			.register_handler(http_method_t::POST, "/snap/", api::simulation::snap, {})
			.register_handler(http_method_t::POST, "/system/", api::simulation::system, {})
			.register_handler(http_method_t::POST, "/presidential/", api::simulation::presidential, {});

		listener
			.register_handler(http_method_t::POST, "/polling/get/", api::polling::get, {})
			.register_handler(http_method_t::POST, "/polling/last/", api::polling::last, {})
			.register_handler(http_method_t::POST, "/polling/run/", api::polling::run, {});

		listener
			.register_handler(http_method_t::POST, "/voter/list/", api::voter::list, {})
			.register_handler(http_method_t::POST, "/voter/get/", api::voter::get, {})
			.register_handler(http_method_t::POST, "/voter/create/", api::voter::create, {})
			.register_handler(http_method_t::POST, "/voter/edit/", api::voter::edit, {})
			.register_handler(http_method_t::POST, "/voter/delete/", api::voter::del, {});
			
		listener
			.register_handler(http_method_t::POST, "/district/get/", api::district::get, {})
			.register_handler(http_method_t::POST, "/district/create/", api::district::create, {})
			.register_handler(http_method_t::POST, "/district/edit/", api::district::edit, {})
			.register_handler(http_method_t::POST, "/district/delete/", api::district::del, {})
			.register_handler(http_method_t::POST, "/district/nuke/", api::district::nuke, {})
			.register_handler(http_method_t::POST, "/district/merge/", api::district::merge, {})
			.register_handler(http_method_t::POST, "/district/reassign/", api::district::reassign, {});

		listener
			.register_handler(http_method_t::POST, "/party/get/", api::party::get, {})
			.register_handler(http_method_t::POST, "/party/create/", api::party::create, {})
			.register_handler(http_method_t::POST, "/party/edit/", api::party::edit, {})
			.register_handler(http_method_t::POST, "/party/delete/", api::party::del, {})
			.register_handler(http_method_t::POST, "/party/merge/", api::party::merge, {});

		listener
			.register_handler(http_method_t::POST, "/company/add/", api::company::create, {})
			.register_handler(http_method_t::POST, "/company/create/", api::company::create, {})
			.register_handler(http_method_t::POST, "/company/edit/", api::company::edit, {})
			.register_handler(http_method_t::POST, "/company/delete/", api::company::del, {});

		listener
			.register_handler(http_method_t::POST, "/product/add/", api::product::create, {})
			.register_handler(http_method_t::POST, "/product/create/", api::product::create, {})
			.register_handler(http_method_t::POST, "/product/edit/", api::product::edit, {})
			.register_handler(http_method_t::POST, "/product/delete/", api::product::del, {});

		listener
			.register_handler(http_method_t::POST, "/statistics/edit/gdp/", api::statistics::edit_gdp, {});

		listener
			.register_handler(http_method_t::POST, "/crisis/create/", api::crises::create, {})
			.register_handler(http_method_t::POST, "/crisis/delete/", api::crises::del, {});

		listener
			.register_handler(http_method_t::POST, "/media/create/", api::media::create, {})
			.register_handler(http_method_t::POST, "/media/edit/", api::media::edit, {})
			.register_handler(http_method_t::POST, "/media/delete/", api::media::del, {});

		listener
			.register_handler(http_method_t::POST, "/law/add/", api::law::add, {})
			.register_handler(http_method_t::POST, "/law/force/", api::law::force, {})
			.register_handler(http_method_t::POST, "/law/repeal/", api::law::repeal, {})
			.register_handler(http_method_t::POST, "/law/article/add/", api::law::add_article, {})
			.register_handler(http_method_t::POST, "/law/article/repeal/", api::law::repeal_article, {})
			.register_handler(http_method_t::POST, "/law/add_article/", api::law::add_article, {})
			.register_handler(http_method_t::POST, "/law/repeal_article/", api::law::repeal_article, {});
		
		listener
			.register_handler(http_method_t::POST, "/constitution/article/add/", api::constitution::add_article, {})
			.register_handler(http_method_t::POST, "/constitution/article/remove/", api::constitution::remove_article, {})
			.register_handler(http_method_t::POST, "/constitution/redraft/", api::constitution::redraft, {})
			.register_handler(http_method_t::POST, "/constitution/article/redraft/", api::constitution::redraft, {});
	}
}
