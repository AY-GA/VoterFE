#include "util/init.hpp"
#include "ui/uimgr.hpp"
#include "callbacks/callbacks.hpp"
#include "simulation/simulation.hpp"

int main() {
	voternn::init();

	voternn::simulation& s = voternn::make_simulation(30, 30);

	s.emplace_district("Mazowieckie");
	s.emplace_district("Malopolskie");
	s.emplace_district("Wielkopolskie");
	s.emplace_district("Kujawskie");
	s.emplace_district("Lubuskie");
	s.emplace_district("Pomorskie");
	s.emplace_district("Warminskie");
	s.emplace_district("Podkarpackie");
	s.emplace_district("Zachodnie");
	s.emplace_district("Dolnoslaskie");
	s.emplace_district("Slaskie");
	s.emplace_district("Opolskie");
	s.emplace_district("Lubelskie");
	s.emplace_district("Podlaskie");
	s.emplace_district("Lodzskie");
	s.emplace_district("Swietokrzyskie");

	s.generate_districts();
	s.generate_voters();

	//std::cout << "final:\n" << s.get_grid() << '\n';

	//std::cout << voternn::simulations.at(s.get_token()).get_map() << '\n';
	std::cout << s.get_token() << '\n';

	//voternn::ui::screen scr;

	/*
	scr.register_colors(COLOR_BLACK, COLOR_CYAN);
	scr.register_colors(COLOR_BLACK, COLOR_WHITE);
	scr.set_colors(COLOR_BLACK, COLOR_CYAN);
	scr.update();

	const int y = scr.getCols();
	const int x = scr.getRows();

	WINDOW* win =  newwin(y/4, x/4, y/4, x/4);
	wbkgd(win, COLOR_PAIR(2));
	box(win, 0, 0);
	wrefresh(win);

	WINDOW* iwin = derwin(win, y/4-2, x/4-2, 1, 1);
	wbkgd(iwin, COLOR_PAIR(2));

	wprintw(iwin, voternn::ui::graphics::logo.c_str());

	wprintw(iwin, "Hello! ;%i;%i", x, y);
	wrefresh(iwin);

  	wgetch(iwin);

  	endwin();
	*/

	std::cout << "Listening at 8080\n" << std::endl;
	while(true) while(!s.get_listener().get().check()) NOOP;

	return 0;
}