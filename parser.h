#pragma once

#include "models.h"

GameDescription parseGameDescription(const std::vector<std::string>& startInfos);

TickDescription parseTickDescription(const std::vector<std::string>& infos);

Answer parseAnswer(const std::vector<std::string>& message);
