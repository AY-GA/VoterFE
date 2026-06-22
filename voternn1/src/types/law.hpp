#ifndef VOTERNN_TYPE_LAW
#define VOTERNN_TYPE_LAW

#include <string>

namespace voternn {
	class law {
		public:
			law() = default;
			explicit law(const std::string& text_) : text(text_) {}

			std::string text;
		private:
	};
};

#endif