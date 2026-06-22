#ifndef VOTERNN_CALLBACKS_API
#define VOTERNN_CALLBACKS_API
#include "../include.hpp"
#include "../util/socket/listener.hpp"
#include "../simulation/simulation.hpp"

#define API_RETURN webserver_handler_return_t
#define API_PARAMS const std::string& token, const nlohmann::json& json

namespace voternn::api {
	namespace simulation {
		API_RETURN map(API_PARAMS);
		API_RETURN increment(API_PARAMS);
		API_RETURN reset(API_PARAMS);
		API_RETURN snap(API_PARAMS);
		API_RETURN system(API_PARAMS);
		API_RETURN presidential(API_PARAMS);
	}

	namespace polling {
		API_RETURN get(API_PARAMS);
		API_RETURN last(API_PARAMS);
		API_RETURN run(API_PARAMS);
	}

	namespace voter {
		API_RETURN list(API_PARAMS);
		API_RETURN get(API_PARAMS);
		API_RETURN create(API_PARAMS);
		API_RETURN edit(API_PARAMS);
		API_RETURN del(API_PARAMS);
	}

	namespace district {
		API_RETURN get(API_PARAMS);
		API_RETURN create(API_PARAMS);
		API_RETURN edit(API_PARAMS);
		API_RETURN del(API_PARAMS);
		API_RETURN nuke(API_PARAMS);
		API_RETURN merge(API_PARAMS);
		API_RETURN reassign(API_PARAMS);
	}

	namespace party {
		API_RETURN get(API_PARAMS);
		API_RETURN create(API_PARAMS);
		API_RETURN edit(API_PARAMS);
		API_RETURN del(API_PARAMS);
		API_RETURN merge(API_PARAMS);
	}	

	namespace company {
		API_RETURN create(API_PARAMS);
		API_RETURN edit(API_PARAMS);
		API_RETURN del(API_PARAMS);
	}

	namespace product {
		API_RETURN create(API_PARAMS);
		API_RETURN edit(API_PARAMS);
		API_RETURN del(API_PARAMS);
	}

	namespace statistics {
		API_RETURN edit_gdp(API_PARAMS);
	}

	namespace crises {
		webserver_handler_return_t create(API_PARAMS);
		webserver_handler_return_t del(API_PARAMS);
	}

	namespace media {
		webserver_handler_return_t create(API_PARAMS);
		webserver_handler_return_t edit(API_PARAMS);
		webserver_handler_return_t del(API_PARAMS);
	}

	namespace law {
		webserver_handler_return_t add(API_PARAMS);
		webserver_handler_return_t force(API_PARAMS);
		webserver_handler_return_t repeal(API_PARAMS);
		webserver_handler_return_t add_article(API_PARAMS);
		webserver_handler_return_t repeal_article(API_PARAMS);
	}

	namespace constitution {
		webserver_handler_return_t add_article(API_PARAMS);
		webserver_handler_return_t remove_article(API_PARAMS);
		webserver_handler_return_t redraft(API_PARAMS);
	}

	namespace misc {
		API_RETURN version(API_PARAMS);
	}
}

namespace voternn {
	void register_handlers(listener& listener);
}

#endif
