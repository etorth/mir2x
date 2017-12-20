/*
 * =====================================================================================
 *
 *       Filename: condcheck.hpp
 *        Created: 08/18/2017 14:59:38
 *  Last Modified: 12/06/2017 14:44:28
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

// #ifdef CONDCHECK
//
// // make __LINE__ as a string
// // use the double expansion trick
//
// #define __CONDCHECK_STR_exp_1(x) #x
// #define __CONDCHECK_STR_exp_2(x) __CONDCHECK_STR_exp_1(x)
// #define __CONDCHECK_STR_LINE     __CONDCHECK_STR_exp_2(__LINE__)
//
// #define static_condcheck(expression) static_assert((expression), "static_condcheck failed: " __FILE__ ":" __CONDCHECK_STR_LINE ": " #expression)
//
// #else
// #define static_condcheck(expression)
// #endif
