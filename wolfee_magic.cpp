#include "wolfee_magic.h"

#include "simulator.h"
#include <algorithm>

std::vector<pos_t> GetChainAttackBombSequenceForGaborAndKovi(
    const std::vector<pos_t>& path, int itemTick, const GameDescription& gd, const TickDescription& tickDesc)
{
    std::vector<pos_t> retVal;

    const auto& isBetweenBush = [](const pos_t& pos) -> bool {
        if (pos.x % 2 == 0 && pos.y % 2 != 0) {
            return true;
        }
        if (pos.x % 2 != 0 && pos.y % 2 == 0) {
            return true;
        }
        return false;
    };

    const auto& isPerpendicular = [](const pos_t& pos1, const pos_t& pos2) -> bool {
        if (pos_t(pos1.y - 1, pos1.x - 1) == pos2) {
            return true;
        }
        if (pos_t(pos1.y - 1, pos1.x + 1) == pos2) {
            return true;
        }
        if (pos_t(pos1.y + 1, pos1.x - 1) == pos2) {
            return true;
        }
        if (pos_t(pos1.y + 1, pos1.x + 1) == pos2) {
            return true;
        }
        return false;
    };

    if (isBetweenBush(path.back())) {
        // ha az utolso lepesben kanyarodunk, es nincs eleg ciponk, akkor tuti nincs megoldas
        if (path.size() > 2 && isPerpendicular(path.back(), path[path.size() - 3])) {
            if (path.size() / 3 > tickDesc.mMe.mRunningShoesTick) {
                return {};
            }
        }
    }

    const auto tickDistance = [](size_t first, size_t last, int shoeCount) {
        size_t dist = last - first;
        int shoeStep = shoeCount * 3;
        if (shoeStep >= dist) {
            return dist / 3 + (((dist % 3) != 0) ? 1 : 0);
        }
        const size_t distRemaining = dist - shoeStep;
        return shoeCount + distRemaining / 2 + (((distRemaining % 2) != 0) ? 1 : 0);
    };

    Simulator simulator(gd);
    simulator.SetState(tickDesc);
    const std::vector<Simulator::BlowArea> alreadyBlowAreas = simulator.GetBlowAreas();
    std::vector<bool> bestCandidate = {};
    int bestCandidateTick = 9001;
    Simulator::Area forbiddenArea(gd.mMapSize);
    forbiddenArea.insert(1, 1);
    forbiddenArea.insert(1, 2);
    forbiddenArea.insert(2, 1);
    forbiddenArea.insert(1, gd.mMapSize - 1 - 1);
    forbiddenArea.insert(1, gd.mMapSize - 1 - 2);
    forbiddenArea.insert(2, gd.mMapSize - 1 - 1);
    forbiddenArea.insert(gd.mMapSize - 1 - 1, 1);
    forbiddenArea.insert(gd.mMapSize - 1 - 2, 1);
    forbiddenArea.insert(gd.mMapSize - 1 - 1, 2);
    forbiddenArea.insert(gd.mMapSize - 1 - 1, gd.mMapSize - 1 - 1);
    forbiddenArea.insert(gd.mMapSize - 1 - 1, gd.mMapSize - 1 - 2);
    forbiddenArea.insert(gd.mMapSize - 1 - 2, gd.mMapSize - 1 - 1);

    for (int usedBombCount = 1; usedBombCount <= std::min(tickDesc.mMe.mPlacableGrenades, 5); ++usedBombCount) {
        std::vector<bool> places(path.size());
        for (size_t i = 0; i < usedBombCount; ++i) {
            places[places.size() - 1 - i] = true;
        }
        do {
            TickDescription td;
            for (size_t index = 0; index < places.size(); ++index) {
                if (places[index]) {
                    Grenade g;
                    g.mX = path[index].x;
                    g.mY = path[index].y;
                    g.mRange = tickDesc.mMe.mGrenadeRange;
                    g.mTick = 5;
                    td.mGrenades.push_back(g);
                }
            }
            simulator.SetState(td);
            const std::vector<Simulator::BlowArea> bas = simulator.GetBlowAreas();
            if (bas.size() != 1) {
                continue;
            }
            if (!bas[0].mArea.find(path.back().x, path.back().y)) {
                continue;
            }

            int sc = tickDesc.mMe.mRunningShoesTick;
            int tickCount = 0;
            int firstTickCount = 0;
            bool isGoodCandidate = true;
            int bc = 0;
            int prevBomb = 0;
            for (size_t nextBomb = 0; nextBomb < places.size(); ++nextBomb) {
                if (!places[nextBomb]) {
                    continue;
                }
                const int ticks = tickDistance(prevBomb, nextBomb, sc);

                if (bc == 0) {
                    int tickDecrease = 0;
                    for (const auto& area : alreadyBlowAreas) {
                        if (area.mArea.find(path[nextBomb].x, path[nextBomb].y)) {
                            tickDecrease = 6 - area.mTickCount; // 6, mert csak a kovetkezo tickben fog megjelenni (5tel)
                            break;
                        }
                    }
                    if (itemTick + ticks > -6 + tickDecrease) {
                        isGoodCandidate = false;
                        break;
                    }
                    firstTickCount = ticks;
                }

                tickCount += ticks;
                if (forbiddenArea.find(path[nextBomb].x, path[nextBomb].y)) {
                    isGoodCandidate = false;
                    break;
                }
                // // only the first bomb could be in an already built area
                // for (const auto& area : alreadyBlowAreas) {
                //     if (area.mArea.find(path[nextBomb].x, path[nextBomb].y)) {
                //         isGoodCandidate = false;
                //         break;
                //     }
                // }

                if (bc == usedBombCount - 1) {
                    if (tickCount - firstTickCount > 4) {
                        isGoodCandidate = false;
                    }
                    if ((path[nextBomb].x % 2) != 0 && (path[nextBomb].y % 2) != 0 && sc == 0 && nextBomb == path.size() - 2) {
                        isGoodCandidate = false;
                    }
                    if (nextBomb == places.size() - 1) {
                        isGoodCandidate = false;
                    }
                    break;
                }
                bc++;
                prevBomb = nextBomb;
            }

            if (isGoodCandidate) {
                if (tickCount < bestCandidateTick) {
                    bestCandidateTick = tickCount;
                    bestCandidate = places;
                }
            }
        } while (std::next_permutation(places.begin(), places.end()));

        if (!bestCandidate.empty()) {
            break;
        }
    }

    for (size_t i = 0; i < bestCandidate.size(); ++i) {
        if (bestCandidate[i]) {
            retVal.push_back(path[i]);
        }
    }

    return retVal;
}