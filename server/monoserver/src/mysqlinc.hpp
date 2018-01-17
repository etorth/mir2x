/*
 * =====================================================================================
 *
 *       Filename: mysqlinc.hpp
 *        Created: 01/16/2018 19:09:23
 *  Last Modified: 01/16/2018 19:13:12
 *
 *    Description: select an implementation of MYSQL
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

#if defined(MIR2X_MYSQL_IMPL_MYSQL)

#include<mysql/mysql.h>

#elif defined(MIR2X_MYSQL_IMPL_MARIADB)

#include<mariadb/mysql.h>

#else

#error "No MYSQL implementation found..."

#endif
