#pragma once

#include <string>

#include "../models.h"

std::string CreateMessage(const std::vector<std::string>& message);
std::vector<std::string> ParseMessage(const std::string& message);
std::vector<std::string> CreateInfo(const TickDescription& tick, int player);
