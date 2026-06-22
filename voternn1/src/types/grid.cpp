#include "grid.hpp"
#include "../util/math.hpp"

namespace voternn {
	simulation_grid::simulation_grid(const uint64_t width, const uint64_t height)
		: width(width), height(height) {
			this->allegiance.resize(this->width);
			for(uint64_t i = 0; i < this->width; i++)
				this->allegiance[i].resize(this->height);
		}

	void simulation_grid::change_allegiance(const size_t x, const size_t y, std::string_view newdistrict) {
		if(x >= this->width || y >= this->height) throw std::out_of_range("X, Y out of grid.");
		this->allegiance[x][y] = newdistrict;
	}

	std::optional<std::string> simulation_grid::get_allegiance(const size_t x, const size_t y) const noexcept {
		if(x >= this->width || y >= this->height) return std::nullopt;
		return this->allegiance[x][y];
	}

	bool simulation_grid::validate_grid() const noexcept {
		if(this->width == 0 || this->height == 0) return false;
		
		for(uint64_t x = 0; x < this->width; x++)
			for(uint64_t y = 0; y < this->height; y++)
				if(!this->get_allegiance(x, y).has_value()) return false;
		return true;
	}

	size_t simulation_grid::get_width() const noexcept {
		return this->width;
	}
	size_t simulation_grid::get_height() const noexcept {
		return this->height;
	}
	size_t simulation_grid::get_total() const noexcept {
		return this->width*this->height;
	}

	simulation_grid::~simulation_grid() {}

	namespace internal {
		struct coordhash {
			std::size_t operator()(const std::pair<uint32_t, uint32_t>& x) const {
				return std::hash<uint64_t>{}((uint64_t)x.first << 32 | (uint64_t)x.second);
			}
		};
	}

	std::ostream& operator<<(std::ostream& s, const simulation_grid& sg) {
		std::unordered_map<std::string, uint64_t> map;

		for(uint64_t y = 0; y < sg.get_height(); y++) {
			for(uint64_t x = 0; x < sg.get_width(); x++) {
				const auto v = sg.get_allegiance(x, y);
				if(v.has_value()) {
					std::string uuid = v.value();
					if(!map.contains(uuid)) map[uuid] = map.size();
					s << (char)(map[uuid]+65);
				}
				else s << '/';
			}
			s << '\n';
		}

		return s;
	}

	namespace internal {
		//generate_district_neighbor_skip_condition was too long :)
		constexpr bool gdnsc(
			const uint64_t x, const uint64_t y,
			const int8_t lx, const int8_t ly,
			const size_t width, const size_t height,
			const bool diag_banned = false) {
			return (lx < 0 && x == 0) ||
					(lx > 0 && x == width-1) ||
					(lx == 0 && ly == 0) ||
					(lx+x) < 0 || (ly+y) < 0 ||
					(lx+x) >= width || (ly+y) >= height ||
					(diag_banned && (lx == ly || lx == -ly));
		}
	}

	void generate_districts(simulation_grid& grid, const std::unordered_map<std::string, district>& districts, const bool enable_smoothing) {
		if(districts.size() == 0) return;
		
		std::uniform_int_distribution<> xuid(0, grid.get_width()-1);
		std::uniform_int_distribution<> yuid(0, grid.get_height()-1);

		std::vector<std::vector<bool>> occupied_state;
		std::unordered_map<std::string, std::unordered_set<std::pair<uint32_t, uint32_t>, internal::coordhash>> neighbouring_state;
		occupied_state.resize(grid.get_width());
		for(auto& a : occupied_state) a.resize(grid.get_height());

		//seed
		for(const auto& p : districts) {
			size_t x, y;
			do {
				x = xuid(random_generator);
				y = yuid(random_generator);
			}
			while(occupied_state[x][y]);

			grid.change_allegiance(x, y, p.first);
			occupied_state[x][y] = true;
			for(auto& d : districts) neighbouring_state[d.first].erase(std::make_pair(x, y));

			//get original neighbours
			for(int8_t lx = -1; lx < 2; lx++)
				for(int8_t ly = -1; ly < 2; ly++) {
					const int64_t rx = x+lx;
					const int64_t ry = y+ly;
					if(internal::gdnsc(x, y, lx, ly, grid.get_width(), grid.get_height()))
						continue;
					if(!occupied_state[rx][ry]) {
						neighbouring_state[p.first].insert(std::make_pair(rx, ry));
					}
				}
		}

		//expand while space remaining
		uint64_t total = grid.get_total();
		uint64_t occupied = districts.size();
		std::uniform_real_distribution ten(0.0, 1.0);
		
		//get random neighbor
		while(true) {
			for(const auto& p : districts) {
				if(neighbouring_state[p.first].size() == 0) continue;

				const uint64_t id = ten(random_generator)*neighbouring_state[p.first].size();
				uint64_t x = INT64_MAX, y = INT64_MAX;

				uint64_t nsflpid = 0;
				for(std::pair<uint32_t, uint32_t> ns : neighbouring_state[p.first]) {
					if(nsflpid == id) {
						x = ns.first;
						y = ns.second;
						break;
					}
					nsflpid++;
				}
				
				grid.change_allegiance(x, y, p.first);
				occupied_state[x][y] = true;
				occupied++;
				for(auto& d : districts) neighbouring_state[d.first].erase(std::make_pair(x, y));

				if(occupied >= total) goto end;

				for(int8_t lx = -1; lx < 2; lx++) for(int8_t ly = -1; ly < 2; ly++) {
					const int64_t rx = x+lx;
					const int64_t ry = y+ly;
					
					if(internal::gdnsc(x, y, lx, ly, grid.get_width(), grid.get_height(), true))
						continue;
					if(!occupied_state[rx][ry]) neighbouring_state[p.first].insert(std::make_pair(rx, ry));
				}
			}
		}
		end:

		if(enable_smoothing) {
			//smoothing pass
			//every tile is set to one of which neighbors most it has

			for(uint64_t x = 1; x < grid.get_width()-1; x++) {
				for(uint64_t y = 1; y < grid.get_height()-1; y++) {
					//UUID to frequency
					std::unordered_map<std::string, uint64_t> freq;

					for(int8_t lx = -1; lx < 2; lx++) for(int8_t ly = -1; ly < 2; ly++) {
						const int64_t rx = x+lx;
						const int64_t ry = y+ly;
						
						if(internal::gdnsc(x, y, lx, ly, grid.get_width(), grid.get_height(), false)) 
							continue;

						const std::string duuid = grid.get_allegiance(rx, ry).value();
						freq[duuid]++;
					}

					std::string max_freq_district_uuid;
					uint64_t max_freq = 0;
					for(const auto& pair : freq) {
						if(pair.second > max_freq) {
							max_freq = pair.second;
							max_freq_district_uuid = pair.first;
						}
					}

					grid.change_allegiance(x, y, max_freq_district_uuid);
				}
			}
		}

		if(!grid.validate_grid())
			throw std::runtime_error("Grid invalid!");
	}
}