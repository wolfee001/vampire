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

// #ifdef _MSC_VER
// #include <intrin.h>
// #define __builtin_popcount __popcnt
// #define __builtin_popcountl _mm_popcnt_u64
// #else
// #include <x86intrin.h>
// #endif

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

char dirc[5] = "^>V<";
char dirc2[5] = "URDL";

void bomb(map_t& m, map_t& orim, pos_t p0, int r, bool dot = true)
{
	m[p0.y][p0.x] = dot ? '.' : ' ';
	if (dot && !m.bombrange.empty())
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
				if (orim[p.y][p.x] != c)
					continue;
				c = '-';
				break;
			}
			else if (c == '-') {
				if (orim[p.y][p.x] != c)
					continue;
				c = dot ? '.' : ' ';
				break;
			}
			else if (c >= '1' && c <= '5')
				bomb(m, orim, p, 1);
			else if (c >= '6' && c <= '9' || c == '0')
				bomb(m, orim, p, 2);
			else if (dot)
				c = '.';
		}
	}
}

map_t sim(map_t& m, bool dot)
{
	auto res = m;
	FOR0(y, SZ(m)) {
		FOR0(x, SZ(m)) {
			char c = res[y][x];
			char c0 = c;
			pos_t p(y, x);
			if (c == '0')
				c = '9';
			else if (c == '1') {
				map_t orim = res;
				bomb(res, orim, p, 1, dot);
			}
			else if (c == '6') {
				map_t orim = res;
				bomb(res, orim, p, 2, dot);
			}
			else if (c >= '2' && c <= '9')
				--c;
			else if (m[y][x] == '.')
				c = ' ';
			if (res[y][x] == c0)
				res[y][x] = c;
		}
	}
	return res;
}

static const int MAXTURN = 20; // no sense in more as powerups will disappear

struct reach_t {
	int turn = MAXTURN + 1;
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
vector<int> steps;

bool show(map_t& m, pos_t start, pos_t target)
{
	map_t res = m;
	int cnt = 0;
	steps.clear();
	while(target != start) {
		auto r = reaches[target.y][target.x];
		if (r.dirfrom == -1)
			break;
		target = target.GetPos((r.dirfrom + 2) % 4);
		res[target.y][target.x] = dirc[r.dirfrom];
		steps.push_back(r.dirfrom);
		if (++cnt > 100) {
			cerr << "maxstep error (loop?)" << endl;
			return false;
		}
	}
	show(res);
	return true;
}

// note getdist to reach there (pick up will be next turn)
int getdist(map_t& m, pos_t start, vector<pos_t> targets, int r, int stepcnt, int bombcnt, vector<event_t>& events)
{
	int ghost = 0;
	vector<map_t> maps;
	maps.push_back(m);
	FOR0(i, MAXTURN + 1) // one turn ahead
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
				if (reaches[targets[i].y][targets[i].x].turn < MAXTURN) {
					if (!firsttargetreached.isvalid())
						firsttargetreached = targets[i];
					swap(targets[i], targets.back());
					targets.pop_back();
				}
			}
			if (targets.empty()) {
//				show(m, start, firsttargetreached);
				return reaches[firsttargetreached.y][firsttargetreached.x].turn;
			}
		}
		if (qi.step >= stepcnt - 1) {
			++qi.turn;
			if (qi.turn >= MAXTURN) {
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
			bool bombnextturn = maps[qi.turn + 1][p.y][p.x] == '.' && !ghost;
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
				float div = 1;
				int cap = min(mul, min(r, bombcnt));
				auto p1 = p;
				int mode = 0;
				if (cap == 1)
					div = 1;
				if (div == 1 && mul > 1 && bombcnt > 1 && checkcornerwithcycle(maps[qi.turn], p, d)) {
					div = 1.5;
					mode = 2;
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

UsualMagic::UsualMagic(const GameDescription& gameDescription)
    : IMagic(gameDescription)
{
    // Maybe some constructor magic? :)
    std::srand(static_cast<unsigned int>(time(nullptr)));
	mInPhase1 = true;
}

pair<int, std::vector<pos_t>> collectgoodbombpos(map_t& m, pos_t start, int r, int avoids)
{
	vector<pos_t> res;
	vector<pos_t> other;
	deque<pos_t> q;
	q.push_back(start);
	reach_t unreachable;
	FOR0(y, SZ(m))
		FOR0(x, SZ(m))
			reaches[y][x].step = 1000;
	reaches[start.y][start.x].step = 0;
	int lastturn = 0;	
	pos_t firsttargetreached;
	int cnt = 0;
	while (!q.empty()) {
		pos_t p1 = q.front();
		q.pop_front();
		bool spec = false;
		FOR0(d, 4) {
			if (p1 == start && (avoids & (1 << d)))
				continue;
			pos_t p0 = p1.GetPos(d);
			char c = m[p0.y][p0.x];
			if (c == ' ' && (reaches[p0.y][p0.x].step > reaches[p1.y][p1.x].step + 1 || 
				reaches[p0.y][p0.x].step == 0)) {
				bool simple = false;
				int more = 0;
				FOR0(d2, 4) {
					auto p = p0;
					FOR(i, 1, r) {
						p = p.GetPos(d2);
						char& c = m[p.y][p.x];
						if (c == 'O')
							break;
						else if (c == '*' || c == '+') {
							++more;
							break;
						}
						else if (c == '-') {
							simple = true;
							break;
						}
					}
					if (simple)
						break;
				}
				if (simple || more > 1)
					res.push_back(p0);
				else if (more)
					other.push_back(p0);
				q.push_back(p0);
				reaches[p0.y][p0.x].step = reaches[p1.y][p1.x].step + 1;
				++cnt;
			}
		}
	}
	reaches[start.y][start.x].step = 0;
	if (res.empty())
		return make_pair(cnt, other);
	return make_pair(cnt, res);
}

int bestbombseqval;
int bombseqmaxstep;
int bombtimeout = 1000;
std::chrono::time_point<std::chrono::steady_clock>  bombtimestart;
vector<pos_t> bombseq;
vector<pos_t> bestbombseq;
vector<pos_t> allbombs;
bool bombseqbatcnt = true;

int batcnt(map_t& m)
{
	int cnt = 0;
	FOR0(y, SZ(m)) {
		FOR0(x, SZ(m)) {
			char c = m[y][x];
			if (c == '-')
				++cnt;
			else if (c == '+')
				cnt += 2;
			else if (c == '*')
				cnt += 3;
		}
	}
	return cnt;
}

void bombdfs(map_t& m, pos_t start, int r, int step, int lastbombidx, int depth, int avoids = 0)
{
	reach_t currreaches[23][23];
	auto bombs = collectgoodbombpos(m, start, r, avoids);
	sort(bombs.second.begin(), bombs.second.end());
	if (step > bombseqmaxstep) {
//		for(auto b : bombseq)
//			cerr << b << ' ';
		int v = (bombseqbatcnt ? (1000 - batcnt(m)) : (bombs.first + 1)) * 100 - step;
//		cerr << v << endl;
		MAXA2(bestbombseqval, v, bestbombseq, bombseq);
		return;
	}

	memcpy(currreaches, reaches, sizeof(reaches));

	// keep bombs ordered so that we dont waste on shuffles
	vector<int> indices;
	int oldcnt = SZ(allbombs);
	for (auto b : bombs.second) {
		bool found = false;
		FOR0(i, oldcnt) {
			if (allbombs[i] == b) {
				if (i >= lastbombidx)
					indices.push_back(i);
				found = true;
				break;
			}
		}
		if (!found) {
			indices.push_back(SZ(allbombs));
			allbombs.push_back(b);
		}
	}

	int oldseqsize = SZ(bombseq);

	for (auto i : indices) {
		auto b = allbombs[i];
		bombseq.push_back(b);
		map_t m2 = m;
		bomb(m2, m2, b, r, false);

		int dst = currreaches[b.y][b.x].step;
		if (dst)
			dst = (dst + 1) / 2;
		dst += 6;

		bombdfs(m2, b, r, step + dst, i, depth + 1);
		bombseq.pop_back();
		if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - bombtimestart).count() > bombtimeout)
			break;
	}
	allbombs.resize(oldcnt);
}

vector<pos_t> bombsequence(map_t& m, pos_t start, int r, int maxstep, bool batcount, int avoids)
{
	bombseqbatcnt = batcount;
	bestbombseqval = 0;
	bombseqmaxstep = maxstep;
	bombtimestart = chrono::steady_clock::now();
	bombdfs(m, start, r, 0, 0, 0, avoids);
	return bestbombseq;
}

int getdist(map_t m, vector<pos_t> targets, const TickDescription& tickDescription, const Vampire& vampire, int avoids = 0)
{
	vector<event_t> events;
	if (vampire.mRunningShoesTick > 0)
		events.push_back(event_t(vampire.mRunningShoesTick, 2, -1));
	for (const auto& grenade : tickDescription.mGrenades)
		if (grenade.mId == vampire.mId)
			events.push_back(event_t(grenade.mTick, 3, 1));
	if (vampire.mGhostModeTick > 0) {
		events.push_back(event_t(vampire.mGhostModeTick, 3, vampire.mPlacableGrenades));
		events.push_back(event_t(0, 4, 1));
		events.push_back(event_t(vampire.mGhostModeTick, 4, -1));
	}
	pos_t p(vampire.mY, vampire.mX);
	FOR0(d, 4) {
		if ((avoids & (1 << d)) == 0)
			continue;
		pos_t p2 = p.GetPos(d);
		m[p2.y][p2.x] = 'O';
	}
    return getdist(m, p, targets, vampire.mGrenadeRange, vampire.mRunningShoesTick > 0 ? 3 : 2, vampire.mGhostModeTick ? 0 : vampire.mPlacableGrenades, events);
}

int blockcnt(map_t& m, pos_t p)
{
	int cnt = 0;
	FOR0(d, 4) {
		pos_t p2 = p.GetPos(d);
		if (m[p2.y][p2.x] != ' ')
			++cnt;
	}
	return cnt;
}

// if entrance is accessible by enemy with bomb
int avoidsack(map_t& m, pos_t entrance, pos_t p)
{
	int avoids = 0;
	FOR0(d, 4) {
		pos_t neck = entrance.GetPos(d);
		if (m[neck.y][neck.x] != ' ')
			continue;
		int block = blockcnt(m, neck);
		if (block < 2)
			continue;
		if (block == 2) {
			pos_t sack = neck.GetPos(d);
			if (m[sack.y][sack.x] != ' ' || blockcnt(m, sack) < 3) 
				continue;
			if (p == neck)
				avoids |= (1 << d);
			avoids |= 16;
		}
		if (p == entrance)
			avoids |= (1 << d);
		else if (p == neck)
			avoids |= 16;
	}
	return avoids;
}

int avoidsack(map_t& m, pos_t player, vector<pos_t> enemieswithbomb)
{
	int avoids = 0;
	FOR0(d, 4) {
		pos_t entrance = player;
		FOR0(i, 2) {
			if (i > 0 || d > 0)
				entrance = entrance.GetPos(d);
			if (m[entrance.y][entrance.x] != ' ' || blockcnt(m, entrance) > 2)
				break;
			for (auto e : enemieswithbomb) {
				if (e.GetDist(entrance) <= 2) {
					avoids |= avoidsack(m, entrance, player);
					break;
				}
			}
		}
	}
	return avoids;
}

int avoidtoward(map_t& m, map_t& nextmap, pos_t player, vector<pos_t> enemieswithbomb)
{
	int avoids = 0;
	FOR0(dd, 4) {
		pos_t p2 = player.GetPos(dd);
		int block = 0;
		int bomb = 0;
		int existing = 0;
		if (m[p2.y][p2.x] != ' ' || nextmap[p2.y][p2.x] == '.') {
			avoids |= (1 << dd);
			continue;
		}
		FOR0(d3, 4) {
			pos_t p3 = p2.GetPos(d3);
			if (m[p3.y][p3.x] >= '1' && m[p3.y][p3.x] <= '9') {
				++bomb;
				++existing;
			} else if (m[p3.y][p3.x] != ' ') {
				++block;
				++existing;
			}
			for (auto enemy : enemieswithbomb)
				if (enemy == p3)
					++bomb;					
		}
		if (block + bomb >= 4 && bomb >= 1 && existing >= 2)
			avoids |= (1 << dd);
	}
	return avoids;
}

int avoidstay(map_t& m, pos_t player, vector<pos_t> enemieswithbomb)
{
	int bomb = 0;
	int block = 0;
	int avoiddirs = 0;
	int existing = 0;
	FOR0(dd, 4) {
		pos_t p2 = player.GetPos(dd);
		if (m[p2.y][p2.x] >= '1' && m[p2.y][p2.x] <= '9') {
			++bomb;
			++existing;
		} else if (m[p2.y][p2.x] != ' ') {
			++block;
			++existing;
		}
		for (auto enemy : enemieswithbomb)
			if (enemy == p2) {
				++bomb;					
				avoiddirs |= (1 << dd);
			}
	}
	if (block + bomb >= 4 && bomb >= 1 && existing >= 2)
		return 16 | 32 | avoiddirs; 
	return 0;
}

int collectavoids(map_t& m, map_t& nextmap, pos_t player, vector<pos_t>& enemieswithbomb)
{
	int avoids = avoidtoward(m, nextmap, player, enemieswithbomb);
	avoids |= avoidstay(m, player, enemieswithbomb);
	int sackavoids = avoidsack(m, player, enemieswithbomb);
	if ((avoids & 32) && (sackavoids & 16))
		avoids &= ~32;
	avoids |= sackavoids;

	if ((avoids & 15) == 15) // danger everywhere = no danger at all (we still avoid stay)
		avoids = 0;

	return avoids;
}

bool checkkickthreat(map_t m, pos_t oldbomb, pos_t newbomb, pos_t player)
{
	if (m[oldbomb.y][oldbomb.x] == '2' && newbomb == player)
		return true;
	m[newbomb.y][newbomb.x] = m[oldbomb.y][oldbomb.x];
	m.bombrange[newbomb.y][newbomb.x] = m.bombrange[oldbomb.y][oldbomb.x];
	m[oldbomb.y][oldbomb.x] = ' ';
	auto nextmap = sim(m);
	return nextmap[player.y][player.x] == '.';
}

int avoidkicks(map_t& m, pos_t bomb, int d, pos_t player, int oriradius)
{
	int avoids = 0;
	pos_t p2 = bomb;
	FOR0(st, oriradius) {
		p2 = p2.GetPos(d);
		if (p2.x == 0 || p2.y == 0 || p2.x >= SZ(m) - 1 || p2.y >= SZ(m))
			break;
		if (m[p2.y][p2.x] != ' ' && (m[p2.y][p2.x] < '1' || m[p2.y][p2.x] > '9'))
			continue;
		if (checkkickthreat(m, bomb, p2, player)) {
			avoids |= 16;
			avoids |= player.y == p2.y ? (player.x < p2.x ? 2 : 8) : (player.y > p2.y ? 1 : 4);
		}
	}
	return avoids;
}

int collectavoids(map_t& m, map_t& nextmap, pos_t player, vector<Vampire> enemies, map<int, enemypredict_t>& enemypredict, vector<Grenade> grenades, int oriradius)
{	
	int avoids;
	vector<pos_t> enemieswithbomb;
	for (const auto& enemy : enemies) {
		if (enemy.mPlacableGrenades >= 1) {
			enemieswithbomb.push_back(pos_t(enemy.mY, enemy.mX));
			if (enemy.mPlacableGrenades >= 2 && (enemypredict[enemy.mId].bombnexttoitem || enemypredict[enemy.mId].doublebomber))
				enemieswithbomb.push_back(pos_t(enemy.mY, enemy.mX));
		}
	}

	avoids = collectavoids(m, nextmap, player, enemieswithbomb);

	for (const auto& enemy : enemies) {
		pos_t ep(enemy.mY, enemy.mX);
		if (enemy.mPlacableGrenades >= 1 && nextmap[enemy.mY][enemy.mX] == '.') {
			map_t test = m;
			test.bombrange.clear();
			bomb(test, test, ep, enemy.mGrenadeRange);
			if (test[player.y][player.x] == '.') {
				avoids |= 16;
				avoids |= player.y == enemy.mY ? (player.x < enemy.mX ? 2 : 8) : (player.y > enemy.mY ? 1 : 4);
			}
		}
		for(const auto& b : grenades) {
			if (b.mId == enemy.mId) {
				pos_t bp(b.mY, b.mX);
				if (ep == bp) {
					FOR0(d, 4)
						avoids |= avoidkicks(m, bp, d, player, oriradius);
				}
				FOR0(d, 4) {
					pos_t p2 = ep.GetPos(d);
					if (p2 == bp)
						avoids |= avoidkicks(m, bp, d, player, oriradius);
				}
			}
		}
	}
	return avoids;
}

bool dobomb(const TickDescription& tickDescription)
{
	auto& me = tickDescription.mMe;
	pos_t mypos = pos_t(me.mY, me.mX);
	for (const auto& enemy : tickDescription.mEnemyVampires) {
		pos_t p(enemy.mY, enemy.mX);
		int dy = abs(p.y - mypos.y);
		int dx = abs(p.x - mypos.x);
		if ((dx == 0 && dy <= me.mGrenadeRange - 2) || (dy == 0 && dx <= me.mGrenadeRange - 2))
			return true;
	}
	return false;
}

int stuckdir = -1;

Answer UsualMagic::Tick(const TickDescription& tickDescription, const Simulator::NewPoints& points)
{
	int turn = tickDescription.mRequest.mTick;

	Answer answer;
	mPhase = NONE;
	mAvoids = 0;
	mPreferGrenade = false;
	mPath.clear();

	auto& me = tickDescription.mMe;
	pos_t mypos = pos_t(me.mY, me.mX);
	cerr << "turn " << tickDescription.mRequest.mTick << ' ' << mypos << endl;
	map_t m = genempty(mGameDescription.mMapSize);
	m.bombrange = m;
	for (const auto& bat : tickDescription.mAllBats)
		m[bat.mY][bat.mX] = bat.mDensity == 1 ? '-' : bat.mDensity == 2 ? '+' : '*';
	for (const auto& grenade : tickDescription.mGrenades) {
		m[grenade.mY][grenade.mX] = grenade.mTick + '0';
		m.bombrange[grenade.mY][grenade.mX] = grenade.mRange + '0';
	}
	m = sim(m, false); // just go on after bombs

	auto nextmap = sim(m);

	if (turn == 0) {
		mEnemyPredict.clear();
		for(const auto& enemy: tickDescription.mEnemyVampires) {
			enemypredict_t et;
			mEnemyPredict.insert(make_pair(enemy.mId, et));
		}
	} else {
		for(const auto& enemy: tickDescription.mEnemyVampires) {
			enemypredict_t& et = mEnemyPredict[enemy.mId];
			pos_t ep(enemy.mY, enemy.mX);
			// note: we can't really differentiate between a failed attempt with stay
			if (et.prevpos == ep || enemy.mGrenadeRange == 0) // still has one more bomb (to finish attempt) - also assures that we are "after" phase1
				continue;
			for(const auto& bomb: tickDescription.mGrenades) {
				pos_t bp(bomb.mY, bomb.mX);
				if (bomb.mId != enemy.mId)
					continue;
				if (et.prevpos == bp) { // just put a bomb there
					for(const auto& pred : mEnemyPredict) {
						pos_t otherprev = pred.second.prevpos;
						if (otherprev == ep || (otherprev.GetDist(ep) == 1 && otherprev.GetDist(et.prevpos) == 2)) {
							et.doublebomber = true;
							break;
						}
					}
				}
			}
		}
		for(const auto& enemy: tickDescription.mEnemyVampires) {
			enemypredict_t& et = mEnemyPredict[enemy.mId];
			pos_t ep(enemy.mY, enemy.mX);
			for(const auto& powerup: tickDescription.mPowerUps) {
				pos_t pp(powerup.mY, powerup.mX);
				char pc = m[pp.y][pp.x];
				for(const auto& bomb: tickDescription.mGrenades) {
					pos_t bp(bomb.mY, bomb.mX);
					if (bomb.mId != enemy.mId)
						continue;
					if (pp == ep && bp == pp)
						et.bombonitem = true;
					else if (et.prevpos.GetDist(pp) == 1 && bp == et.prevpos && pc == ' ')
						et.bombnexttoitem = true;
				}
			}
			et.prevpos = ep;
		}
	}
	mEnemyPredict[me.mId].prevpos = mypos;
/*
	if (tickDescription.mRequest.mTick == 0) {
		int dir = -1;
		int cnt = 0;
		stuckdir = -1;
		FOR0(d, 4) {
			pos_t p = mypos.GetPos(d);
			if (m[p.y][p.x] == ' ') {
				pos_t p2 = p.GetPos(d);
				if (m[p2.y][p2.x] != ' ') {
					++cnt;
					dir = d;
				}
			}
		}
		if (cnt == 2) {
			answer.mSteps.push_back(dirc2[dir]);
			stuckdir = dir;
			return answer;
		}
	}

	if (stuckdir != -1 && tickDescription.mRequest.mTick == 1) {
		answer.mPlaceGrenade = true;
		answer.mSteps.push_back(dirc2[(stuckdir + 2) % 4]);
		return answer;
	}
*/
	bool onitem = false;
	bool ontomato = false;
	for (const auto& powerup : tickDescription.mPowerUps) {
		if (pos_t(powerup.mY, powerup.mX) == mypos && powerup.mRemainingTick < -1) {
			onitem = true;
			if (powerup.mType == PowerUp::Type::Tomato)
				ontomato = true;
		}
	}

	mAvoids = collectavoids(m, nextmap, mypos, tickDescription.mEnemyVampires, mEnemyPredict, tickDescription.mGrenades, mGameDescription.mGrenadeRadius + 1);

	if ((mAvoids & 48) == 48 && me.mPlacableGrenades > 0 && onitem) {
		mAvoids &= ~48;
		mPreferGrenade = 1;
		cerr << "prefer to protect self from encircled" << endl;
	}
	else if ((mAvoids & 16) && ontomato && me.mHealth == 2) // with 2 health, it can be more or less ok to stay (no matter what the timings are)
		mAvoids &= ~16;

	FOR0(d, 4) {
		pos_t p = mypos.GetPos(d);
		if (m[p.y][p.x] != ' ')
			continue;
		for (const auto& powerup : tickDescription.mPowerUps) {
			pos_t pp(powerup.mY, powerup.mX);
			if (pp == p && blockcnt(m, pp) == 2) {
				bool importantitem = powerup.mType == PowerUp::Type::Tomato && me.mHealth < 3 || powerup.mType == PowerUp::Type::Shoe;
				bool attackableenemy = false;
				bool dangerousbomber = false;
				bool dangerousbomberotherside = false;
				char c2;
				FOR0(ei, SZ(tickDescription.mEnemyVampires)) {
					const auto& enemy = tickDescription.mEnemyVampires[ei];
					pos_t ep(enemy.mY, enemy.mX);
					if (ep.GetDist(p) <= 1) {
						if (ep != p && (mEnemyPredict[enemy.mId].bombnexttoitem || mEnemyPredict[enemy.mId].doublebomber) && enemy.mPlacableGrenades >= 2 &&
							(!importantitem || me.mHealth == 1)) {
							if (me.mPlacableGrenades >= 2 && turn % 3 == 0) { // attack more than pick
								attackableenemy = true;
								continue;
							}
						}
						if (me.mPlacableGrenades >= 2 && (enemy.mHealth == 1 || !importantitem))
							attackableenemy = true;
					}
				}
				if (attackableenemy) {
					FOR0(d2, 4) {
						pos_t p2 = p.GetPos(d2);
						if (p2 != mypos && m[p2.y][p2.x] == ' ' && nextmap[p2.y][p2.x] != '.') {
							answer.mPlaceGrenade = true;
							answer.mSteps.push_back(dirc2[d]);
							answer.mSteps.push_back(dirc2[d2]);
							cerr << "Encircle with direct command" << dirc2[d] << dirc2[d2] << endl;
							return answer;
						}
					}
				}
				mAvoids &= ~(1 << d); // just go for the item
				break;
			}
		}
	}

	bool wantcharge = false;
	{
		if (me.mPlacableGrenades >= 1 && m[mypos.y][mypos.x] == ' ') {
			if (nextmap[mypos.y][mypos.x] == '.') {
				map_t test = m;
				test.bombrange.clear();
				bomb(test, test, mypos, me.mGrenadeRange);
				for (const auto& enemy : tickDescription.mEnemyVampires) {
					pos_t p(enemy.mY, enemy.mX);
					if (nextmap[p.y][p.x] != '.' && test[p.y][p.x] == '.') {
						mPreferGrenade = 1;
						cerr << "chainkill" << endl;
						break;
					}
				}
			}
			// todo: how to detected sackable enemy with me on neck?
			if (!mPreferGrenade && m[mypos.y][mypos.x] == ' ') {
				for (const auto& enemy : tickDescription.mEnemyVampires) {
					pos_t p(enemy.mY, enemy.mX);
					int block = blockcnt(m, p);
					if (p.GetDist(mypos) == 1 && block >= 2) {
						if (block == 3) { 
							mPreferGrenade = 1;
							cerr << "encircled" << endl;
						}
						else { // force move to other side to finish encapsulation?
							int dirtoward = mypos.y == enemy.mY ? (mypos.x < enemy.mX ? 1 : 3) : (mypos.y > enemy.mY ? 0 : 2);
							int dirtshift = (1 << dirtoward);
							pos_t p2 = p.GetPos(dirtoward);
							if (m[p2.y][p2.x] == ' ' && blockcnt(m, p2) == 3) {
								cerr << "encircled with sacked";
								mPreferGrenade = 1;
							} else {
								if (me.mPlacableGrenades == 1)
									continue;
	//							if (enemy.mPlacableGrenades > 0)
	//								continue;
								if ((mAvoids & dirtshift) == dirtshift)
									continue;
								mPreferGrenade = 2;
								cerr << "encircle attempt" << endl;
								mAvoids = 16;
								mAvoids |= (15 - dirtshift);
							}
						}
						break;
					}
					// sack is only chance do not force it?
				}
			}
		}
#if 0
		if (!mPreferGrenade && tickDescription.mPowerUps.empty() &&
			me.mGrenadeRange >= 3 && me.mPlacableGrenades >= 2 && me.mHealth >= 2 && 
			(me.mRunningShoesTick || tickDescription.mRequest.mTick > mGameDescription.mMaxTick / 2))
			wantcharge = true;
#endif
	}

	if (!tickDescription.mPowerUps.empty()) {

		mInPhase1 = false;

		vector<pos_t> targets;
		vector<int> closestenemydist;
		vector<int> closestenemy;
		int waitturn = 0;
		int oriwaitturn = 0;
		int lastturn = 0;
		int defendturn = 0;
		for (const auto& powerup : tickDescription.mPowerUps) {
			targets.push_back(pos_t(powerup.mY, powerup.mX));
			closestenemydist.push_back(MAXTURN + 1);
			closestenemy.push_back(-1);
			defendturn = powerup.mDefensTime;
			if (powerup.mRemainingTick < 0) {
				oriwaitturn = -powerup.mRemainingTick - 1;
				waitturn = -powerup.mRemainingTick - 1 + defendturn;
				lastturn = waitturn + 15;
			} else {
				oriwaitturn = 0;
				lastturn = powerup.mRemainingTick - 1;
				waitturn = defendturn;
			}
		}
		FOR0(ei, SZ(tickDescription.mEnemyVampires)) {
			const auto& enemy = tickDescription.mEnemyVampires[ei];
			getdist(m, targets, tickDescription, enemy);
			FOR0(i, SZ(targets)) {
				MINA2(closestenemydist[i], reaches[targets[i].y][targets[i].x].turn, closestenemy[i], ei);
				if (lastturn == 0 && pos_t(enemy.mY, enemy.mX) == targets[i])
					lastturn = 1;
			}
		}
		getdist(m, targets, tickDescription, me, mAvoids);

		int best = -1;
		int closest = MAXTURN + 1;
		bool bothreachableinwait = true;
		FOR0(i, SZ(targets)) {
			int reach = reaches[targets[i].y][targets[i].x].turn;
			if (reach > waitturn)
				bothreachableinwait = false;
		}
		FOR0(tr, 2) {
			FOR0(i, SZ(targets)) {
				int reach = reaches[targets[i].y][targets[i].x].turn;
				if (mypos == targets[i]) {
                    if (oriwaitturn == 0 && me.mRestCount >= defendturn)
						continue;
					best = i;
					waitturn = defendturn - me.mRestCount;
					break;
				}
				if (reach == 0 && mypos != targets[i] && waitturn == 0 && closestenemydist[i] == 0) { // to late
					bool enemyon = false;
					for (const auto& enemy : tickDescription.mEnemyVampires) {
						if (pos_t(enemy.mY, enemy.mX) == targets[i] && enemy.mRestCount >= defendturn) {
							enemyon = true;
							break;
						}
					}
					if (enemyon)
						continue;
				}
				if (me.mHealth == 1 && tickDescription.mEnemyVampires.size() > 2 && 
					closestenemydist[i] <= reach + 2 && tickDescription.mPowerUps[i].mType != PowerUp::Type::Tomato)
					continue;
				if ((closestenemydist[i] >= reach && lastturn >= reach) || (reach <= waitturn && 
					(!mEnemyPredict[tickDescription.mEnemyVampires[closestenemy[i]].mId].bombonitem || waitturn - reach > 4 || tr == 1)))
					MINA2(closest, reach, best, i);
			}
			if (best != -1)
				break;
		}

		if (best == -1) {
			cerr << "no target" << endl; 
			// we gonna fall to temporary...or should we go for closest bad target
			if (tickDescription.mRequest.mTick < 150 && tickDescription.mAllBats.size() >= 4) {
				cerr << "than we are still in phase1" << endl; 
				mInPhase1 = true;
			}
		}
		else {
			cerr << "closest target: " << targets[best] << endl;
			mReachDiff = closestenemydist[best] - closest;
			if (mReachDiff < 0 && !mEnemyPredict[tickDescription.mEnemyVampires[closestenemy[best]].mId].bombonitem)
				mReachDiff = 0; // we don't care
			if (show(m, mypos, targets[best])) {

				int stepcnt = me.mRunningShoesTick > 0 ? 3 : 2;
				pos_t p = mypos;
				int last = max(0, SZ(steps) - stepcnt);
				FORD(i, SZ(steps) - 1, last) {
					p = p.GetPos(steps[i]);
					if (m[p.y][p.x] != ' ' || i == last && nextmap[p.y][p.x] == '.')
						break;
					char c = dirc2[steps[i]];
				}
				cerr << endl;
				mPhase = ITEM;
				p = mypos;
				FORD(i, SZ(steps) - 1, 0) {
					p = p.GetPos(steps[i]);
					mPath.push_back(p);
				}
				int mydist = mypos.GetDist(targets[best]);
				if (mydist <= 2) {
					if (closestenemydist[best] == 0 && me.mPlacableGrenades > 0 && 
						m[mypos.y][mypos.x] == ' ' && waitturn > 0 && waitturn < 5 - (mydist ? 1 : 0)) {
						bool ok = true;
						// note: statistically it is not worth
						// except if we would know that we battle for the 1st place vs. someone else!

						if (mydist == 0) {
							int bomb = 0;
							int block = 0;
							FOR0(d, 4) {
								pos_t p = mypos.GetPos(d);
								if (m[p.y][p.x] >= '1' && m[p.y][p.x] <= '9') {
									++bomb;
									break;
								} else if (m[p.y][p.x] != ' ')
									++block;
							}
							if (bomb >= 1 && block >= 2) {
								int cnt = 0;
								for (const auto& enemy : tickDescription.mEnemyVampires) {
									if (pos_t(enemy.mY, enemy.mX) == mypos)
										++cnt;
								}
								if (cnt <= 1) {
									ok = false;
									mPreferGrenade = false;
								}
							}
						} 
						if (ok) {
							for (const auto& enemy : tickDescription.mEnemyVampires) {
								pos_t p(enemy.mY, enemy.mX); 
								int theirdist = p.GetDist(targets[best]);
								int reldist = p.GetDist(mypos);
								if (me.mPlacableGrenades > 0 && p != mypos && theirdist <= (enemy.mRunningShoesTick ? 3 : 2) &&
									mydist < theirdist && (mydist <= 1 || reldist == 1)) {
									mPreferGrenade = 1;
									cerr << "prefer grenade to protect item" << endl;
									break;
								}
							}
						}
					}
				}
				if (wantcharge && dobomb(tickDescription)) {
					mPreferGrenade = 1;
					cerr << "and prefer grenade to attack" << endl;
				}
/*				if (!mPreferGrenade && mydist <= 4 && mydist >= 2 && closestenemydist[best] > closest && mPath.size() <= mGameDescription.mGrenadeRadius && !(mAvoids & 16) && me.mPlacableGrenades > 1) { // prepare throwable
					mPreferGrenade = true;
					cerr << "and randomly put grenade to be thrown";
				} */
				return answer;
			}
		}
	}

	if (mInPhase1) {
//		bombseqbatcnt = true;
		bombtimeout = mTimeout.count();
//		vector<pos_t> seq;
// no phase1
		auto seq = bombsequence(m, mypos, me.mGrenadeRange, 30, true, mAvoids); // todo how many steps (not turns) ahead
		if (seq.empty())
			mInPhase1 = false;
		else {
			mPhase = PHASE1;
			mPath = seq;
			cerr << "phase1 seq";
			for(auto p : seq)
				cerr << p;
			cerr << ' ' << bestbombseqval << endl;
			return answer;
		}
	} 
	
	{
		cerr << "between items or charge" << endl;
		vector<pos_t> targets;
		vector<int> closestenemydist;
		vector<int> closestenemy;
		FOR0(y, SZ(m)) {
			FOR0(x, SZ(m)) {
				if (m[y][x] == ' ') { // we focus on directly reachable only
					targets.push_back(pos_t(y, x));
					closestenemydist.push_back(MAXTURN + 1);
					closestenemy.push_back(-1);
				}
			}
		}
		FOR0(e, SZ(tickDescription.mEnemyVampires)) {
			const auto& enemy = tickDescription.mEnemyVampires[e];
			getdist(m, vector<pos_t>(), tickDescription, enemy);
			if (enemy.mGrenadeRange == mGameDescription.mGrenadeRadius && !enemy.mRunningShoesTick) // only defend vs. collector enemy
				continue;
			FOR0(i, SZ(targets))
				MINA2(closestenemydist[i], reaches[targets[i].y][targets[i].x].turn, closestenemy[i], e);
		}
		vector<char> dirs;
		vector<char> bestdirs;
		int best = 0;
		bool has[23][23];
		MSET(has, 0);
		FOR(d11, -1, 3) {
			int d1 = (d11 + 5) % 5;
			pos_t p1 = mypos.GetPos(d1);
			if ((1 << d1) & mAvoids)
				continue;
			if (m[p1.y][p1.x] != ' ')
				continue;
			dirs.clear();
			if (d1 != 4)
				dirs.push_back(dirc2[d1]);
			FOR(d21, -1, 3) {
				int d2 = (d21 + 5) % 5;
				if (d1 == 4)
					d2 = 4;
				else if (d2 == (d1 + 2) % 4)
					continue;
				pos_t p2 = p1.GetPos(d2);
				if (m[p2.y][p2.x] != ' ')
					continue;
				if (d2 != 4)
					dirs.push_back(dirc2[d2]);
				FOR(d31, -1, 3) {
					int d3 = (d31 + 5) % 5;
					if (me.mRunningShoesTick == 0 && d3 == 0 || d2 == 4)
						d3 = 4;
					pos_t p3 = p2.GetPos(d3);
					if (m[p3.y][p3.x] != ' ' || nextmap[p3.y][p3.x] == '.')
						continue;
					if (has[p3.y][p3.x])
						continue;
					int cnt = 0;
					FOR0(d4, 3) {
						pos_t p4 = p3.GetPos(d4);
						if (m[p4.y][p4.x] == ' ')
							++cnt;
					}
					if (cnt <= 2)
						continue;
					has[p3.y][p3.x] = true;
					if (d3 != 4)
						dirs.push_back(dirc2[d3]);
					Vampire me2 = me;
					me2.mY = p3.y;
					me2.mX = p3.x;
					if (wantcharge) {
						int val = 0;
						for (auto& enemy : tickDescription.mEnemyVampires) {
							int d = p3.GetDist(pos_t(enemy.mY, enemy.mX));
							if (d == 0)
								d = 2;
							MAXA(val, 10000 - 100 * d + min(abs(p3.y - enemy.mY), abs(p3.x - enemy.mY)));
						}
						MAXA2(best, val, bestdirs, dirs);
					}
					else {
						getdist(m, vector<pos_t>(), tickDescription, me2);
						float cnt = 0;
						FOR0(i, SZ(targets)) {
							int reach = reaches[targets[i].y][targets[i].x].turn;
							if (closestenemydist[i] > reach + 2)
								cnt += 2;
							else if (closestenemydist[i] > reach + 1)
								cnt += 1.5;
							else if (closestenemydist[i] > reach)
								cnt += 1.2;
							else if (reach <= 7)
								++cnt;
						}
						int dq = (abs(SZ(m) / 2 - p3.y) + 1) * (abs(SZ(m) / 2 - p3.x) + 1); // prefer center
	/*					cerr << p3 << ' ';
						for(auto c : dirs)
							cerr << c;
						cerr << ' ' << cnt << ' ' << dq << endl; */
						MAXA2(best, cnt * 1000 - dq, bestdirs, dirs);
					}
					if (d3 != 4)
						dirs.pop_back();
				}
				if (d2 != 4)
					dirs.pop_back();
			}
			if (d1 != 4)
				dirs.pop_back();
		}
		if (best > 0) {
			if (wantcharge) {
				cerr << "charge! ";
				mPhase = CHARGE;
				if (dobomb(tickDescription)) {
					mPreferGrenade = true;
					cerr << "with grenade ";
				}
			} else {
				cerr << "between items, go closer ";
				mPhase = BETWEEN_ITEMS;
			}
			pos_t p = mypos;
			for(auto c : bestdirs) {
				p = p.GetPos(c == 'U' ? 0 : c == 'R' ? 1 : c == 'D' ? 2 : 3);
				mPath.push_back(p);
				cerr << c;
			}
			cerr << endl;

/*			if (!mPreferGrenade && mPath.empty() && !(mAvoids & 16) && me.mPlaceableGrenades > 1 && turn % 3 == 0) {
				mPreferGrenade = true;
				cerr << "randomly put grenade to be thrown";
			} */
			return answer;
		}
	}

    return answer;
}

// clang-format on