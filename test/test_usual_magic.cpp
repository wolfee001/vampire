// clang-format off

#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "../usual_magic.h"

// legend:
// O bush
// P player (only at start)
// T target (only at start)
// 1-5 bomb with remaining time
// 6-0 bomb with range2
// -+* bat density
// . blast (with last step of the turn do not step there if blast is there on next turn map)


void testsim(map_t inp, map_t expect, bool dot = true)
{
    auto res = sim(inp, dot);
    ASSERT_EQ(res,  expect);
}

TEST(UsualMagic, UsualMagicSim)
{
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O O O",
				"O   O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O   O",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O-  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.  O",
				"O.O O",
				"O.  O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O-  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O   O",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}, false
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O2O O",
				"O-  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O-  O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O+  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.  O",
				"O.O O",
				"O-  O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O0O O",
				"O   O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O   O",
				"O9O O",
				"O   O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O6  O",
				"O O O",
				"O-  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O...O",
				"O.O O",
				"O.  O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O1- O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.  O",
				"O.O O",
				"O.. O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O1  O",
				"O+O O",
				"O1  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.. O",
				"O.O O",
				"O.. O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O1  O",
				"O1O O",
				"O+  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.. O",
				"O.O O",
				"O-  O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O   O",
				"O1O O",
				"O7 -O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.  O",
				"O.O O",
				"O...O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O- 7O",
				"O O2O",
				"O 12O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O...O",
				"O O.O",
				"O...O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O*17O",
				"O O O",
				"O0 8O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O+..O",
				"O.O.O",
				"O...O",
				"OOOOO"
			}
		}
		);
	testsim(
		{
			{
				"OOOOO",
				"O1  O",
				"O1O O",
				"O+  O",
				"OOOOO"
			}
		},
		{
			{
				"OOOOO",
				"O.. O",
				"O.O O",
				"O-  O",
				"OOOOO"
			}
		}
		);
}

TEST(UsualMagic, UsualMagicGetDist)
{
    std::vector<event_t> noevents;
    ASSERT_EQ(getdist(
		{ { 
            "OOOOO", 
            "OPT O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 2, 1, noevents), 0);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP TO",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 0);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP  O",
				"O OTO",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 1);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP -O",
				"O OTO",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 2);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP -O",
				"O OTO",
				"O  -O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 7);

	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"O  -O",
				"OPOTO",
				"O6  O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 3);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"O  -O",
				"OPOTO",
				"O1  O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 3);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP-TO",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}, 1, 6, 1, noevents), 0);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 12);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 1, 2, 2, noevents), 12);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 2, 2, 2, noevents), 12);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOOOO",
				"OP +T O",
				"O O O O",
				"O   * O",
				"O*O*O*O",
				"O     O",
				"OOOOOOO"
			}
		}, 1, 2, 2, noevents), 9);
	ASSERT_EQ(getdist(
		{
			{
				"OOOOOOO",
				"O    *O",
				"O O*O O",
				"O T+  O",
				"O O O O",
				"O *  PO",
				"OOOOOOO"
			}
		}, 1, 2, 2, noevents), 10);

	std::vector<event_t> events;
	events.push_back({ 1, 2, 1 });
	ASSERT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP  O",
				"O OTO",
				"O   O",
				"OOOOO"
			}
		}, 1, 1, 1, events), 1);

	map_t mp =
	{
		{
			"OOOOO",
			"O  -O",
			"OPOTO",
			"O1  O",
			"OOOOO"
		}
	};
	mp.bombrange =
	{
		{
			"OOOOO",
			"O   O",
			"O O O",
			"O2  O",
			"OOOOO"
		}
	};
	ASSERT_EQ(getdist(mp, 1, 2, 1, noevents), 3);

	events.clear();
	events.push_back({ 0, 4, 1 });
	events.push_back({ 1, 4, -1 });
    ASSERT_EQ(getdist(
		{ { 
            "OOOOO", 
            "OP*TO", 
            "O O2O", 
            "O 1 O", 
            "OOOOO" } }, 1, 2, 0, events), 3);
}

void checkgoodbombpos(map_t m, int r, int cnt)
{
	pos_t start;
	map_t orim = m;
	std::vector<pos_t> expect;
	for(size_t y = 0; y < m.size(); ++y) {
		for(size_t x = 0; x < m.size(); ++x) {
			if (m[y][x] == 'P') {
				m[y][x] = ' ';
				start = pos_t(static_cast<int>(y), static_cast<int>(x));
			}
			if (m[y][x] == 'T')
				m[y][x] = ' ';
		}
	}
	auto res = collectgoodbombpos(m, start, r);
	for(auto p : res.second)
		m[static_cast<size_t>(p.y)][static_cast<size_t>(p.x)] = 'T';
	m[static_cast<size_t>(start.y)][static_cast<size_t>(start.x)] = 'P';
	ASSERT_EQ(m, orim);
	ASSERT_EQ(res.first, cnt);
}

TEST(UsualMagic, CollectGoodBombPos)
{
    checkgoodbombpos(
		{ { 
            "OOOOO", 
            "OP  O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 8);
    checkgoodbombpos(
		{ { 
            "OOOOO", 
            "OPT-O", 
            "O OTO", 
            "O   O", 
            "OOOOO" } }, 1, 7);
    checkgoodbombpos(
		{ { 
            "OOOOO", 
            "OPT+O", 
            "O OTO", 
            "O   O", 
            "OOOOO" } }, 1, 7);
    checkgoodbombpos(
		{ { 
            "OOOOO", 
            "OPT-O", 
            "O O O", 
            "O+  O", 
            "OOOOO" } }, 1, 3);
    checkgoodbombpos(
		{ { 
            "OOOOO", 
            "OTT-O", 
            "OPOTO", 
            "O  TO", 
            "OOOOO" } }, 2, 7);
	checkgoodbombpos(
		{
			{
				"OOOOOOO",
				"OP    O",
				"O O+O O",
				"O  T  O",
				"O O+O O",
				"O     O",
				"OOOOOOO"
			}
		}, 1, 19);
}

void checkbombsequence(map_t m, int r, int maxstep)
{
	pos_t start;
	map_t orim = m;
	std::vector<pos_t> expect;
	for(size_t y = 0; y < m.size(); ++y) {
		for(size_t x = 0; x < m.size(); ++x) {
			if (m[y][x] == 'P') {
				m[y][x] = ' ';
				start = pos_t(static_cast<int>(y), static_cast<int>(x));
			}
		}
	}
	int repeat = 0;
	for(int i = 1; i < 9; ++i) {
		bool found = false;
		int later = 0;
		for(size_t y = 0; y < m.size() && !found; ++y) {
			for(size_t x = 0; x < m.size() && !found; ++x) {
				if (m[y][x] == '0' + i) {
					m[y][x] = ' ';
					for(int j = 0; j <= repeat; ++j)
						expect.push_back(pos_t(static_cast<int>(y), static_cast<int>(x)));
					found = true;
					break;
				}
				else if (m[y][x] == '0' + i + 1)
					later = 1;
				else if (m[y][x] == '0' + i + 2)
					later = 2;
			}
		}
		if (!found) {
			if (!later)
				break;
			repeat = later;
			if (repeat == 2)
				++i;
		} else
			repeat = 0;
	}
	auto res = bombsequence(m, start, r, maxstep, false);
	ASSERT_EQ(res, expect);
}

// tests should have clear winner
TEST(UsualMagic, BombSequence)
{
	// note: all these tests are for freecell count, not batcount
    checkbombsequence(
		{ { 
            "OOOOO", 
            "OP1-O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 6);

   checkbombsequence(
		{ { 
            "OOOOO", 
            "OP2+O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 12);
    checkbombsequence(
		{ { 
            "OOOOO", 
            "OP3*O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 18);
    checkbombsequence(
		{ { 
            "OOOOO", 
            "OP1-O", 
            "O*O2O", 
            "O  -O", 
            "OOOOO" } }, 1, 12);
	checkbombsequence(
		{
			{
				"OOOOOOO",
				"OP 1 +O",
				"O O-O O",
				"O+ 2 +O",
				"O O-O O",
				"O  3- O",
				"OOOOOOO"
			}
		}, 1, 18);
	checkbombsequence(
		{
			{
				"OOOOOOO",
				"OP 1 +O",
				"O O-O O",
				"O* 3 *O",
				"O O+O O",
				"O     O",
				"OOOOOOO"
			}
		}, 1, 18);
	checkbombsequence(
		{
			{
				"OOOOOOO",
				"OP ++ O",
				"O3O+O O",
				"O*    O",
				"O O O O",
				"O     O",
				"OOOOOOO"
			}
		}, 1, 18);
}

int checkavoid(map_t m)
{
	pos_t start;
	std::vector<pos_t> enemies;
	for(size_t y = 0; y < m.size(); ++y) {
		for(size_t x = 0; x < m.size(); ++x) {
			if (m[y][x] == 'P') {
				m[y][x] = ' ';
				start = pos_t(static_cast<int>(y), static_cast<int>(x));
			}
			else if (m[y][x] == 'e') {
				m[y][x] = ' ';
				enemies.push_back(pos_t(static_cast<int>(y), static_cast<int>(x)));
			}
			else if (m[y][x] == 'E') {
				m[y][x] = ' ';
				enemies.push_back(pos_t(static_cast<int>(y), static_cast<int>(x)));
				enemies.push_back(pos_t(static_cast<int>(y), static_cast<int>(x)));
			}
		}
	}
	auto nextmap = sim(m);
	int res = collectavoids(m, nextmap, start, enemies);
	return res;
}

TEST(UsualMagic, Avoids)
{
	ASSERT_EQ(checkavoid(
		{ { 
            "OOOOO", 
            "O P3O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }) & 16, 0);
	ASSERT_EQ(checkavoid(
		{ { 
            "OOOOO", 
            "O PeO", 
            "O O O", 
            "O   O", 
            "OOOOO" } }) & 16, 0);
	ASSERT_EQ(checkavoid(
		{ { 
            "OOOOO", 
            "O  EO", 
            "O OPO", 
            "O   O", 
            "OOOOO" } }) & 16, 16);
	ASSERT_EQ(checkavoid(
		{ { 
            "OOOOO", 
            "O  EO", 
            "O OPO", 
            "O   O", 
            "OOOOO" } }) & 1, 1);
	ASSERT_EQ(checkavoid(
		{ { 
            "OOOOO", 
            "O  EO", 
            "O OPO", 
            "O 2 O", 
            "OOOOO" } }), 0);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O     O",
				"O O O O",
				"OE  P-O",
				"O O O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 26, 18);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O     O",
				"OEO-O O",
				"O  P -O",
				"O O-O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 26, 18);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O     O",
				"O O-O O",
				"OE  P-O",
				"O O-O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 26, 18);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O     O",
				"O O-O O",
				"O E P-O",
				"O O-O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 24, 0);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O     O",
				"OEO O O",
				"O   P-O",
				"O O O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 24, 0);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O  -  O",
				"O O O O",
				"O -P- O",
				"OEO O O",
				"O     O",
				"OOOOOOO"
			}
		}) & 21, 0);
	ASSERT_EQ(checkavoid(
		{
			{
				"OOOOOOO",
				"O  -  O",
				"O O O O",
				"O -P- O",
				"O O O O",
				"O E   O",
				"OOOOOOO"
			}
		}) & 21, 17);
}