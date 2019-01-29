/*
 * =====================================================================================
 *
 *       Filename: database.hpp
 *        Created: 08/31/2015 10:45:48 PM
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

#pragma once
#include "dbbase.hpp"

#if defined(MIR2X_ENABLE_SQLITE3)
    #include "dbengine_sqlite3.hpp"
#endif

#if defined(MIR2X_ENABLE_MYSQL)
    #include "dbengine_mysql.hpp"
#endif
