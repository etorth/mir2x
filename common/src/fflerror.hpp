#pragma once
#include <sstream>
#include <stdexcept>
#include "strf.hpp"

#define fflerror(...) std::runtime_error(str_ffl() + ": " + str_printf(__VA_ARGS__))
#define fflassert(...) do{if(__VA_ARGS__){}else{throw fflerror("assertion failed: %s", #__VA_ARGS__);}}while(0)

#define bad_reach() fflerror("bad reach")
#define bad_value(n) fflerror("bad value: %s", dynamic_cast<std::ostringstream &>((std::ostringstream() << (n))).str().c_str())
