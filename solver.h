#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "models.h"
#include "usual_magic.h"

class solver {
    static std::vector<std::pair<int, int>> line2d(std::pair<int, int> from, const std::pair<int, int>& to);

public:
    void startMessage(const std::vector<std::string>& startInfos);
    std::vector<std::string> processTick(const std::vector<std::string>& infos);

private:
    GameDescription mGameDescription;
    TickDescription mTickDescription;
    std::unique_ptr<UsualMagic> mMagic;
};

#endif // SOLVER_H_INCLUDED
