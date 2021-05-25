/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 08/31/2015 08:52:57 PM
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

#include <string>
#include <cstdint>
#include <FL/Fl.H>
#include "totype.hpp"
#include "fflerror.hpp"
#include "dbcomrecord.hpp"

int main(int argc, char *argv[])
{
    fflassert(argc == 2);
    const auto &mr = DBCOM_MAGICRECORD(to_u8cstr(argv[1]));

    fflassert(mr);
    fflassert(mr.getGfxEntry(u8"运行"));
    fflassert(mr.getGfxEntry(u8"运行").checkType(u8"跟随"));
    return 0;
}
