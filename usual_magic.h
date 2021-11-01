// clang-format off
#pragma once

#include "models.h"

struct map_t : std::vector<std::string> 
{
	std::vector<std::string> bombrange;
};

struct event_t {
	int turn;
	int variable; // 1 r, 2 stepcnt, 3 bombcnt
	int change = 1;
	event_t(int _turn, int _variable, int _change): turn(_turn), variable(_variable), change(_change) {}
};

map_t sim(map_t& m);
int getdist(map_t m, int r, int stepcnt, int bombcnt, std::vector<event_t> events);

class UsualMagic {
public:
    explicit UsualMagic(const GameDescription& gameDescription);

    Answer Tick(const TickDescription& tickDescription);

private:
    GameDescription mGameDescription;
};
// clang-format on