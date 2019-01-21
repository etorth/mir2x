/*
 * =====================================================================================
 *
 *       Filename: sqlinc.hpp
 *        Created: 01/21/2019 09:00:31
 *    Description: 
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

#if defined(MIR2X_ENABLE_MYSQL)
    #include "mysqlinc.hpp"
#endif

#if defined(MIR2X_ENABLE_SQLITE3)
    #include "sqlite3.hpp"
#endif
