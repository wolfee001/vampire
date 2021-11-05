// clang-format off
#pragma once

#include <iostream>
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

struct pos_t {
	int y;
	int x;
	pos_t(int _y = -1, int _x = -1) : y(_y), x(_x) {}
	pos_t GetPos(int d) const;
	int GetDir(pos_t p2) const
	{
		if (p2.y < y)
			return 0;
		if (p2.x > x)
			return 1;
		if (p2.y > y)
			return 2;
		if (p2.x < x)
			return 3;
		return -1;
	}
	int GetDist(const pos_t& o) const { return abs(x - o.x) + abs(y - o.y); }
	bool operator==(const pos_t& o) const { return x == o.x && y == o.y; }
	bool operator<(const pos_t& o) const { return x < o.x || (x == o.x && y < o.y); }
	bool operator!=(const pos_t& o) const { return x != o.x || y != o.y; }
	bool isvalid() const { return x != -1; }
	friend std::ostream& operator<< (std::ostream& stream, const pos_t& p)
	{
		return stream << p.x << ":" << p.y << " ";
	}
};

map_t sim(map_t& m, bool dot = true);
int getdist(map_t m, int r, int stepcnt, int bombcnt, std::vector<event_t> events);
std::pair<int, std::vector<pos_t>> collectgoodbombpos(map_t& m, pos_t start, int r);
std::vector<pos_t> bombsequence(map_t& m, pos_t start, int r, int maxstep);

enum phase_t {
	NONE,
	PHASE1,
	ITEM,
	BETWEEN_ITEMS
};

class UsualMagic : public IMagic {
public:
	bool mInPhase1 = true;
    explicit UsualMagic(const GameDescription& gameDescription);
    Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points);
	phase_t mPhase; 
	std::vector<pos_t> mPath;
};

// clang-format on
