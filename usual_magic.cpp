// clang-format off
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <map>
#include <set>
#include <unordered_set>
#include <limits>
#include <string.h>
#include <float.h>
#include <deque>
#include <math.h>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <cassert>
#include <deque>

#include "usual_magic.h"

#include <ctime>

#ifdef _MSC_VER
#include <intrin.h>
#define __builtin_popcount __popcnt
#define __builtin_popcountl _mm_popcnt_u64
#else
#include <x86intrin.h>
#endif

#define FOR(i, b, e)    for(int i = (b); i <= (e); i++)
#define FORL(i, b, e)    for(int i = (b); i < (e); i++)
#define FORD(i, e, b)    for(int i = (e); i >= (b); i--)
#define FOR0(i, e)        FORL(i, 0, (int) (e))

#define min(a, b)        (((a) < (b)) ? (a) : (b))
#define max(a, b)        (((a) > (b)) ? (a) : (b))
#define MINA(a, b)        do { if ((a) > (b)) (a) = (b); } while(0)
#define MAXA(a, b)        do { if ((a) < (b)) (a) = (b); } while(0)
#define MINA2(a, b, i, j)        do { if ((a) > (b)) { (a) = (b); (i) = (j); } } while(0)
#define MAXA2(a, b, i, j)        do { if ((a) < (b)) { (a) = (b); (i) = (j); } } while(0)
#define MAXA3(a, b, i, j, k, l)        do { if ((a) < (b)) { (a) = (b); (i) = (j); (k) = (l);} } while(0)

#define SWAP(a, b)        do { int _t = a; a = b; b = _t; } while(0)
#define SWAPT(a, b, t)    do { t _t = a; a = b; b = _t; } while(0)
#define SQR(a)            ((a) * (a))
#define MSET(a, b)        memset(a, b, sizeof(a))

#define INT                int
#define INT_CAP            0x3F3F3F3F

typedef long long int    LI;
#define SZ(c)            (static_cast<int>(c.size()))
using namespace std;


int g_seed = 777778;

inline int randn0(int mod) { g_seed = (214013 * g_seed + 2531011); return (((g_seed >> 16) & 0x7FFF) * mod) >> 15; }

struct pos_t {
	int y;
	int x;
	pos_t(int y = -1, int x = -1) : y(y), x(x) {}
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
	int GetDist2(const pos_t& o) const { return SQR(x - o.x) + SQR(y - o.y); }
	bool operator==(const pos_t& o) const { return x == o.x && y == o.y; }
	bool operator<(const pos_t& o) const { return x < o.x || x == o.x && y < o.y; }
	bool operator!=(const pos_t& o) const { return x != o.x || y != o.y; }
	bool isvalid() const { return x != -1; }
	friend ostream& operator<< (ostream& stream, const pos_t& p)
	{
		return stream << p.x << ":" << p.y << " ";
	}
};

pos_t dir[4] = { { -1,0 },{ 0,1 },{ 1,0 },{ 0,-1 } };
char dirc[5] = "^>V<";

pos_t pos_t::GetPos(int d) const
{
	return pos_t(y + dir[d].y, x + dir[d].x);
}

void bomb(map_t& m, pos_t p0, int r)
{
	m[p0.y][p0.x] = '.';
	if (!m.bombrange.empty())
		r = m.bombrange[p0.y][p0.x] - '0';
	FOR0(d, 4) {
		auto p = p0;
		FOR(i, 1, r) {
			p = p.GetPos(d);
			char& c = m[p.y][p.x];
			if (c == 'O')
				break;
			else if (c == '*') {
				c = '+';
				break;
			}
			else if (c == '+') {
				c = '-';
				break;
			}
			else if (c == '-') {
				c = '.';
				break;
			}
			else if (c >= '1' && c <= '5')
				bomb(m, p, 1);
			else if (c >= '6' && c <= '9' || c == '0')
				bomb(m, p, 2);
			else
				c = '.';
		}
	}
}

map_t sim(map_t& m)
{
	auto res = m;
	FOR0(y, SZ(m)) {
		FOR0(x, SZ(m)) {
			char c = m[y][x];
			char c0 = c;
			pos_t p(y, x);
			if (c == '0')
				c = '9';
			else if (c == '1')
				bomb(res, p, 1);
			else if (c == '6')
				bomb(res, p, 2);
			else if (c >= '2' && c <= '9')
				--c;
			else if (c == '.')
				c = ' ';
			if (res[y][x] == c0)
				res[y][x] = c;
		}
	}
	return res;
}

struct reach_t {
	int turn = 1000;
	int step = 0;
	int dirfrom = -1;
	int mode = 0;
};

struct que_item_t {
	pos_t p;
	int turn = 0;
	int step = -1;
	int delay = 0;
};

bool checkcornerwithcycle(map_t& m, pos_t p, int dirfrom)
{
	FOR(rel, -1, 1) {
		if (rel == 0)
			continue;
		pos_t p1 = p.GetPos((dirfrom + rel + 4) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		p1 = p1.GetPos((dirfrom + rel + 4) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		p1 = p1.GetPos((dirfrom + 2) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		p1 = p1.GetPos((dirfrom + 2) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		p1 = p1.GetPos((dirfrom + 4 - rel) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		p1 = p1.GetPos((dirfrom + 4 - rel) % 4);
		if (m[p1.y][p1.x] != ' ')
			continue;
		return true;
	}
	return false;
}

void show(map_t& m)
{
	FOR0(i, SZ(m))
		cerr << m[i] << endl;
	cerr << endl;
}

reach_t reaches[23][23];

void show(map_t& m, pos_t start, pos_t target)
{
	map_t res = m;
	int cnt = 0;
	while(target != start) {
		auto r = reaches[target.y][target.x];
		if (r.dirfrom == -1)
			break;
		target = target.GetPos((r.dirfrom + 2) % 4);
		res[target.y][target.x] = dirc[r.dirfrom];
		if (++cnt > 100) {
			cerr << "maxstep error (loop?)" << endl;
			break;
		}
	}
	show(res);
}

// note getdist to reach there (pick up will be next turn)
int getdist(map_t& m, pos_t start, vector<pos_t> targets, int r, int stepcnt, int bombcnt, vector<event_t>& events)
{
	vector<map_t> maps;
	maps.push_back(m);
	int MAXTURN = 50;
	FOR0(i, MAXTURN) // turn ahead
		maps.push_back(sim(maps.back()));
	deque<que_item_t> q;
	que_item_t qi;
	qi.p = start;
	q.push_back(qi);
	reach_t unreachable;
	FOR0(y, SZ(m))
		FOR0(x, SZ(m))
			reaches[y][x] = unreachable;
	reaches[start.y][start.x].turn = 0;
	reaches[start.y][start.x].step = 0;
	int lastturn = 0;
	pos_t firsttargetreached;
	while (!q.empty()) {
		qi = q.front();
		q.pop_front();
		if (!targets.empty()) {
			FOR0(i, SZ(targets)) {
				if (reaches[targets[i].y][targets[i].x].turn < 100) {
					if (!firsttargetreached.isvalid())
						firsttargetreached = targets[i];
					swap(targets[i], targets.back());
					targets.pop_back();
				}
			}
			if (targets.empty()) {
				show(m, start, firsttargetreached);
				return reaches[firsttargetreached.y][firsttargetreached.x].turn;
			}
		}
		if (qi.step == stepcnt - 1) {
			++qi.turn;
			if (qi.turn >= MAXTURN) {
				cerr << "max turn reached" << endl;
				return -1;
			}
			if (qi.turn > lastturn) {
				lastturn = qi.turn;
				FOR0(i, SZ(events)) {
					if (events[i].turn == lastturn) {
						int type = events[i].variable;
						int* val = type == 1 ? &r : type == 2 ? &stepcnt : &bombcnt;
						*val += events[i].change;
						swap(events[i], events.back());
						events.pop_back();
						--i;
					}
				}
			}
			if (qi.delay)
				--qi.delay;
			qi.step = 0;
		}
		else
			++qi.step;
		if (qi.delay) {
			q.push_back(qi);
			continue;
		}
		bool spec = false;
		FOR0(d, 4) {
			pos_t p = qi.p.GetPos(d);
			char c = maps[qi.turn][p.y][p.x];
			bool bombnextturn = maps[qi.turn + 1][p.y][p.x] == '.';
			if (bombnextturn)
				spec = true;
			if (!qi.delay && (c == ' ' || c == '.') && (qi.step < stepcnt - 1 || !bombnextturn)) {
				if (c != '.' && (reaches[p.y][p.x].turn < qi.turn || reaches[p.y][p.x].turn == qi.turn && reaches[p.y][p.x].step <= qi.step))
					continue;
				reaches[p.y][p.x].turn = qi.turn;
				reaches[p.y][p.x].dirfrom = d;
				reaches[p.y][p.x].step = qi.step;
				auto qi2 = qi;
				qi2.p = p;
				q.push_back(qi2);
			}
			else if (c == '-' || c == '+' || c == '*') {
				auto qi2 = qi;
				qi2.p = p;
				int mul = c == '-' ? 1 : c == '+' ? 2 : 3;
				float div = 0;
				int cap = min(mul, min(r, bombcnt));
				auto p1 = p;
				int mode = 0;
				if (cap == 1)
					div = 1;
				else {
					FOR0(i, cap) {
						p1 = p1.GetPos((d + 2) % 4);
						char c1 = maps[qi.turn][p1.y][p1.x];
						if (c1 != ' ' && c1 != '.')
							break;
						++div;
					}
				}
				if (div > 1) {
					mode = 1;
				}
				if (div == 1 && mul > 1 && bombcnt > 1 && checkcornerwithcycle(maps[qi.turn], p, d)) {
					div = 1.5;
					mode = 2;
				}
				else if (qi.turn >= 1 && maps[qi.turn - 1][p.y][p.x] != c && bombcnt > 1 && r > 1) {
					div = 1.5;
					mode = 3;
				}
				qi2.delay = (int) (mul / div * 5 + div);
				if (qi.step > 0)
					++qi2.delay; // we can only lay next turn
				if (qi.turn + qi2.delay > reaches[p.y][p.x].turn)
					continue;
				reaches[p.y][p.x].turn = qi.turn + qi2.delay;
				reaches[p.y][p.x].step = 0;
				reaches[p.y][p.x].dirfrom = d;
				reaches[p.y][p.x].mode = mode;
				q.push_back(qi2);
			}
		}
		if (spec && maps[qi.turn + 1][qi.p.y][qi.p.x] != '.')
			q.push_back(qi);
	}
	return -1;
}

int getdist(map_t m, int r, int stepcnt, int bombcnt, vector<event_t> events)
{
	pos_t start, target;
	bool ok = false;
	vector<pos_t> targets;
	FOR0(y, SZ(m)) {
		FOR0(x, SZ(m)) {
			if (m[y][x] == 'P') {
				m[y][x] = ' ';
				start = pos_t(y, x);
			}
			if (m[y][x] == 'T') {
				m[y][x] = ' ';
				targets.push_back(pos_t(y, x));
			}
		}
	}
	return getdist(m, start, targets, r, stepcnt, bombcnt, events);
}

map_t genempty(int n)
{
	map_t m;
	m.resize(n);
	FOR0(i, n)
		FOR0(j, n)
			m[i].push_back(' ');
	FOR0(i, n) {
		m[0][i] = 'O';
		m[n - 1][i] = 'O';
		m[i][0] = 'O';
		m[i][n - 1] = 'O';
		FOR0(j, n) {
			if (i % 2 == 0 && j % 2 == 0)
				m[i][j] = 'O';
		}
	}
	return m;
}

map_t gen(int n)
{
	auto m = genempty(n);
	FOR0(t, 300) {
		int i = 1 + randn0(n - 2);
		int j = 1 + randn0(n - 2);
		if (i % 2 == 0 && j % 2 == 0)
			continue;
		int v = randn0(3);
		m[i][j] = v == 0 ? '-' : v == 1 ? '+' : '*';
	}
	m[1][1] = 'P';
	m[n - 2][n - 2] = 'T';
	return m;
}

// todo: 
// multiple bomb in the same place?

// strategically
// stay center
// item: relative gain (better have 2<-1 range, than 3<-2 bomb)
// item: range first, than number, than speed

UsualMagic::UsualMagic(const GameDescription& gameDescription)
    : mGameDescription(gameDescription)
{
    // Maybe some constructor magic? :)
    std::srand(static_cast<unsigned int>(time(nullptr)));
}

Answer UsualMagic::Tick(const TickDescription& tickDescription)
{
    Answer answer;

	auto& me = tickDescription.mMe;
	map_t m = genempty(mGameDescription.mMapSize);
	m.bombrange = m;
	for (const auto& bat : tickDescription.mAllBats)
		m[bat.mY][bat.mX] = bat.mDensity == 1 ? '-' : bat.mDensity == 2 ? '+' : '*';
	vector<pos_t> targets;
	for (const auto& powerup : tickDescription.mPowerUps)
		targets.push_back(pos_t(powerup.mY, powerup.mX));
	vector<event_t> events;
	if (me.mRunningShoesTick > 0)
		events.push_back(event_t(me.mRunningShoesTick, 2, -1));
	for (const auto& grenade : tickDescription.mGrenades) {
		m[grenade.mY][grenade.mX] = grenade.mTick + '0';
		m.bombrange[grenade.mY][grenade.mX] = grenade.mRange + '0';
		if (grenade.mId == me.mId)
			events.push_back(event_t(grenade.mTick, 3, 1));
	}
	getdist(m, pos_t(me.mY, me.mX), targets, me.mGrenadeRange, me.mRunningShoesTick > 0 ? 3 : 2, me.mPlacableGrenades, events);

// todo: handle multi bomb on same position?
// todo: handle ghost mode
// todo: assess enemy vampires as well

    std::vector<char> dirs = { 'U', 'R', 'D', 'L' };
    // answer.mSteps.push_back(dirs[static_cast<size_t>(std::rand() % 4)]);
    // answer.mSteps.push_back(dirs[static_cast<size_t>(std::rand() % 4)]);
    answer.mSteps = {'R', 'L'};

    return answer;
}

// clang-format on