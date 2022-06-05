#pragma once
#include <cstdio>
#include <cstdlib>

#ifdef CONDCHECK
#define condcheck(expression)                                                                       \
    do{                                                                                             \
        if(!(expression)){                                                                          \
            std::fprintf(stderr, "condcheck failed: %s:%d: %s\n", __FILE__, __LINE__, #expression); \
            std::abort();                                                                           \
        }                                                                                           \
    }while(0)
#else
#define condcheck(expression)   \
    do{                         \
        if(!(expression)){      \
        }                       \
    }while(0)
#endif
