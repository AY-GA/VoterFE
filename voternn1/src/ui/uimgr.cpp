#include "uimgr.hpp"

//black
#define B (uint8_t)1,
//half
#define H (uint8_t)2,
//quarter
#define Q (uint8_t)3,
//nothing
#define E (uint8_t)0,
//newline
#define N (uint8_t)9,

namespace voternn {
	namespace ui {
		static const uint8_t LOGO_RAW[] = {
			B Q E E B Q E E E E E B Q E E E E E E E E E E E E E B Q E E B Q E B Q E E B Q N
			B Q E E B Q E E E E B B B Q E E E E E E E E E E E E B B Q E B Q E B B Q E B Q N
			E B Q B Q E E B Q E E B Q E E E E B B Q E B E B Q E B Q B Q B Q E B Q B Q B Q N
			E B Q B Q E B Q B Q E B Q B Q E B B Q E E B B Q E E B Q E B B Q E B Q E B B Q N
			E E B Q E E E B Q E E E B Q E E E B B Q E B Q E E E B Q E E B Q E B Q E E B Q N
		};
		static const size_t LOGO_WIDTH = 40;
		static const size_t LOGO_HEIGHT = 5;

		//https://en.wikipedia.org/wiki/Block_Elements
		static constexpr std::string make_escaped_string(const uint8_t logo[], const size_t length) {
			std::string result;
			result.reserve(length);

			for(size_t i = 0; i < length; i++) {
				switch(logo[i]) {
					case 1: result.append("\u2588"); break; //full block
					case 2: result.append("\u2592"); break; //half transparent block
					case 3: result.append("\u2591"); break; //almost fully transparent block
					case 0: result.push_back(' '); break; //empty
					case 9: result.push_back('\n'); break; //newline
				}
			}

			return result;
		}

		namespace graphics {
			const std::string logo = make_escaped_string(LOGO_RAW, sizeof(LOGO_RAW));
		}

		screen::screen() {
			initscr();
			start_color();
		}

		screen& screen::update() {
			refresh();
			return *this;
		}

		screen& screen::register_colors(const uint8_t fg, const uint8_t bg) {
			this->pairs.emplace_back(fg, bg);
			init_pair(this->pairs.size(), fg, bg);
			return *this;
		}

		screen& screen::set_colors(const uint8_t fg, const uint8_t bg) {
			for(size_t i = 0; i < pairs.size(); i++) {
				if(pairs[i].fg == fg && pairs[i].bg == bg) {
					wbkgd(stdscr, COLOR_PAIR(i+1));
					return *this;
				}
			}
			
			this->pairs.emplace_back(fg, bg);
			init_pair(this->pairs.size(), fg, bg);
			wbkgd(stdscr, COLOR_PAIR(this->pairs.size()));

			return *this;
		}

		size_t screen::getCols() const {
			return COLS;
		}
		size_t screen::getRows() const {
			return LINES;
		}

		screen::~screen() {

		}

		window::window(screen& scr, window* parent) : parent(parent), screen_reference(scr) {	
			this->handle = newwin(5, 5, 5, 5);
		}
		window& window::set_colors(const uint8_t fg, const uint8_t bg) {
			return *this;
		}
		window& window::input() {
			return *this;
		}
		window& window::print(const char* text) {
			return *this;
		}
		void window::apply() {

		}
		window::~window() {
			delwin(this->handle);
		}
	}
}