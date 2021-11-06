#include "i_magic.h"

void IMagic::SetTickTimeout(std::chrono::milliseconds millis)
{
    mTimeout = millis;
}