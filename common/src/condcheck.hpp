/*
 * =====================================================================================
 *
 *       Filename: condcheck.hpp
 *        Created: 08/18/2017 14:59:38
 *  Last Modified: 12/25/2017 00:33:27
 *
 *    Description: don't use assert
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

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
