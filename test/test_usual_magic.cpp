// clang-format off

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


void testsim(map_t inp, map_t expect)
{
    auto res = sim(inp);
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
				"O...O",
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
				"O-O O",
				"O-  O",
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
    EXPECT_EQ(getdist(
		{ { 
            "OOOOO", 
            "OPT O", 
            "O O O", 
            "O   O", 
            "OOOOO" } }, 1, 2, 1, noevents), 0);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP TO",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 0);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP  O",
				"O OTO",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 1);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP -O",
				"O OTO",
				"O   O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 2);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP -O",
				"O OTO",
				"O  -O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 7);

	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"O  -O",
				"OPOTO",
				"O6  O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 3);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"O  -O",
				"OPOTO",
				"O1  O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 3);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP-TO",
				"O O O",
				"O   O",
				"OOOOO"
			}
		}, 1, 6, 1, noevents), 0);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 1, 2, 1, noevents), 12);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 1, 2, 2, noevents), 12);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O  +O",
				"OOOOO"
			}
		}, 2, 2, 2, noevents), 8);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP +O",
				"O OTO",
				"O 4+O",
				"OOOOO"
			}
		}, 2, 2, 2, noevents), 8);
	EXPECT_EQ(getdist(
		{
			{
				"OOOOO",
				"OP -O",
				"O OTO",
				"O  -O",
				"OOOOO"
			}
		}, 2, 2, 2, noevents), 7);
	EXPECT_EQ(getdist(
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
	EXPECT_EQ(getdist(
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
	EXPECT_EQ(getdist(
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
	EXPECT_EQ(getdist(mp, 1, 2, 1, noevents), 3);
}
