// clang-format off
#pragma once

#include <iostream>
#include <chrono>
#include "i_magic.h"

struct map_t : std::vector<std::string> 
{
	std::vector<std::string> bombrange = {};
};

struct event_t {
	int turn;
	int variable; // 1 r, 2 stepcnt, 3 bombcnt, 4 ghost
	int change = 1;
	event_t(int _turn, int _variable, int _change): turn(_turn), variable(_variable), change(_change) {}
};

map_t sim(map_t& m, bool dot = true);
int getdist(map_t m, int r, int stepcnt, int bombcnt, std::vector<event_t> events);
std::pair<int, std::vector<pos_t>> collectgoodbombpos(map_t& m, pos_t start, int r);
std::vector<pos_t> bombsequence(map_t& m, pos_t start, int r, int maxstep);
int collectavoids(map_t& m, map_t& nextmap, pos_t player, std::vector<pos_t> enemieswithbomb);

class UsualMagic : public IMagic {
public:
	bool mInPhase1 = true;
    explicit UsualMagic(const GameDescription& gameDescription);
    Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points);
	phase_t mPhase; 
	std::vector<pos_t> mPath;
	int mAvoids;
};

// clang-format on
