/*
 * =====================================================================================
 *
 *       Filename: querytype.hpp
 *        Created: 09/21/2017 22:31:02
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
enum QueryType: int
{
    QUERY_NONE    = 0,
    QUERY_PENDING = 1,
    QUERY_OK      = 2,
    QUERY_ERROR   = 3,
};
