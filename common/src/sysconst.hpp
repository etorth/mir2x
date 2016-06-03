/*
 * =====================================================================================
 *
 *       Filename: sysconst.hpp
 *        Created: 04/11/2016 22:24:56
 *  Last Modified: 06/02/2016 11:42:22
 *
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
#include <cstdint>

const int SYS_OBJMAXW     = 48;
const int SYS_OBJMAXH     = 96;
const int SYS_MAXR        = 20;
const int SYS_MAPGRIDXP   = 48;
const int SYS_MAPGRIDYP   = 32;
const int SYS_MAPVISIBLEW = 500;
const int SYS_MAPVISIBLEH = 400;

const char *SYS_MAPNAME(uint32_t);
