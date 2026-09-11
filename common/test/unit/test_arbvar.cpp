#include <cmath>
#include <cstdio>
#include <stdexcept>
#include "mathf.hpp"

namespace
{
    void require(bool condition, const char *message)
    {
        if(!condition){
            throw std::runtime_error(message);
        }
    }

    void requireNear(double actual, double expected, double epsilon, const char *message)
    {
        require(std::abs(actual - expected) <= epsilon, message);
    }
}

int main()
{
    try{
        mathf::ARBVar never(0.0);
        require(never.prob() == 0.0, "zero-probability ARBVar changed prob");
        require(never.count() == 0, "zero-probability ARBVar starts with nonzero count");
        require(never.aprob() == 0.0, "zero-probability ARBVar has nonzero adjusted probability");
        require(!never.holdroll(), "zero-probability ARBVar holdroll succeeded");
        for(int i = 1; i <= 5; ++i){
            require(!never.roll(), "zero-probability ARBVar roll succeeded");
            require(never.count() == i, "zero-probability ARBVar did not count failures");
            require(never.aprob() == 0.0, "zero-probability ARBVar adjusted probability grew");
        }

        mathf::ARBVar always(1.0);
        require(always.prob() == 1.0, "one-probability ARBVar changed prob");
        require(always.count() == 0, "one-probability ARBVar starts with nonzero count");
        require(always.aprob() == 1.0, "one-probability ARBVar adjusted probability is not one");
        for(int i = 0; i < 5; ++i){
            require(always.holdroll(), "one-probability ARBVar holdroll failed");
            require(always.roll(), "one-probability ARBVar roll failed");
            require(always.count() == 0, "one-probability ARBVar did not reset count");
        }

        mathf::ARBVar half(0.5);
        requireNear(half.aprob(), 0.302103027701, 1.0e-12, "half-probability ARBVar table lookup changed");
        require(half.count() == 0, "holdroll-free table check changed count");

        std::puts("ARBVar passed: probability table, boundary rolls, and state transitions.");
        return 0;
    }
    catch(const std::exception &e){
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
