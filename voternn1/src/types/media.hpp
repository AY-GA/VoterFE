#ifndef VOTERNN_TYPE_MEDIA
#define VOTERNN_TYPE_MEDIA

#include <string>

namespace voternn {
	class media {
		public:
			media() = default;
			explicit media(const std::string& name_) : name(name_) {}

			std::string name;
		private:
	};
};

#endif