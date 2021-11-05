#pragma once

#include "models.h"

#include <iostream>
#include <map>

struct pos_t {
    int y;
    int x;
    pos_t(int _y = -1, int _x = -1)
        : y(_y)
        , x(_x)
    {
    }
    pos_t GetPos(int d) const;
    int GetDir(pos_t p2) const
    {
        if (p2.y < y) {
            return 0;
        }
        if (p2.x > x) {
            return 1;
        }
        if (p2.y > y) {
            return 2;
        }
        if (p2.x < x) {
            return 3;
        }
        return -1;
    }
    int GetDist(const pos_t& o) const
    {
        return abs(x - o.x) + abs(y - o.y);
    }
    bool operator==(const pos_t& o) const
    {
        return x == o.x && y == o.y;
    }
    bool operator<(const pos_t& o) const
    {
        return x < o.x || (x == o.x && y < o.y);
    }
    bool operator!=(const pos_t& o) const
    {
        return x != o.x || y != o.y;
    }
    bool isvalid() const
    {
        return x != -1;
    }
    friend std::ostream& operator<<(std::ostream& stream, const pos_t& p)
    {
        return stream << p.x << ":" << p.y << " ";
    }
};

class IMagic {
public:
    explicit IMagic(const GameDescription& gameDescription)
        : mGameDescription(gameDescription)
    {
    }

    virtual ~IMagic() = default;

    virtual Answer Tick(const TickDescription& tickDescription, const std::map<int, float>& points) = 0;

protected:
    GameDescription mGameDescription;
};