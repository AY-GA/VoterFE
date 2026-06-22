#ifndef VOTERNN_TYPE_COMPANY
#define VOTERNN_TYPE_COMPANY
#include "voter.hpp"

namespace voternn {
	enum class product_category {

	};

	class product {
	public:
		product();

		~product();
	private:
		std::string name;
		uint64_t price;
		uint64_t production_price;
		uint64_t reliablity;
	};

	class employee : public voter {
	public:
		employee();

		~employee();
	private:
	};

	class company {
	public:
		company();

		~company();
	private:
		std::vector<product> products;
	};
}

#endif
