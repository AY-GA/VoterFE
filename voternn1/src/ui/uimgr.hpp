#ifndef VOTERNN_UI
#define VOTERNN_UI
#include "../include.hpp"

namespace voternn {
	namespace ui {
		/// @brief NCurses color pair.
		struct color_pair {
			uint8_t fg;
			uint8_t bg;
		};

		/// @brief NCurses form abstraction.
		class form {
		public:
		private:
			FORM handle;
		};

		class screen;


		/// @brief Main NCurses screen manager.
		class screen {
		public:
			screen();

			screen& update();

			screen& register_colors(const uint8_t fg, const uint8_t bg);
			screen& set_colors(const uint8_t fg, const uint8_t bg);

			size_t getCols() const;
			size_t getRows() const;

			~screen();
		private:
			std::vector<color_pair> pairs;
			
		};

		namespace graphics {
			extern const std::string logo;
		}

		/// @brief NCurses window abstraction.
		class window {
		public:
			window(screen& scr, window* parent = nullptr);

			window& set_colors(const uint8_t fg, const uint8_t bg);

			window& input();

			window& print(const char* text);

			window& add_child(window& window);

			void apply();

			~window();
		private:
			std::reference_wrapper<screen> screen_reference;
			WINDOW* handle;
			std::unordered_map<size_t, uint16_t> pairs;
			window* parent;
			std::vector<std::reference_wrapper<window>> children;
		};
	};
};

#endif