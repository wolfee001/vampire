#pragma once

#include <iostream>

#ifdef CHECK_ENABLED
#define CHECK(what, ...)                                                                                                                                       \
    if (!(what)) {                                                                                                                                             \
        std::cerr << __VA_ARGS__ << std::endl;                                                                                                                 \
        throw std::runtime_error(__VA_ARGS__);                                                                                                                 \
    }
#else
#define CHECK(what, ...)                                                                                                                                       \
    if (!(what)) {                                                                                                                                             \
        std::cerr << __VA_ARGS__ << std::endl;                                                                                                                 \
    }
#endif
