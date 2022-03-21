/*
 * =====================================================================================
 *
 *       Filename: dbcomrecord.cpp
 *        Created: 07/30/2017 02:01:02
 *    Description: split from dbcom.hpp
 *                 here we include dbcomid.hpp means we have a copy of the
 *                 constexpr _Inn_XXXX[] declared in dbcomid.hpp
 *
 *                 then all reference from current uint(.cpp file) will use
 *                 the same copy of the _Inn_XXXX
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

#include <string_view>
#include "totype.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "dbcomrecord.hpp"

const static struct ItemRecordAssertor
{
    ItemRecordAssertor()
    {
        fflassert(!DBCOM_ITEMRECORD(nullptr));
        for(uint32_t i = 1; i < DBCOM_ITEMENDID(); ++i){
            const auto &ir = DBCOM_ITEMRECORD(i);
            fflassert(ir);

            for(uint32_t j = i + 1; j < DBCOM_ITEMENDID(); ++j){
                const auto &next_ir = DBCOM_ITEMRECORD(j);
                fflassert(next_ir);
                fflassert(!next_ir.isItem(ir.name));
            }
        }
    }
}
s_itemRecordAssertor {};

const static struct BuffActRecordAssertor
{
    BuffActRecordAssertor()
    {
        fflassert(!DBCOM_BUFFACTRECORD(nullptr));
        for(uint32_t i = 1; i < DBCOM_BUFFACTENDID(); ++i){
            const auto &bar = DBCOM_BUFFACTRECORD(i);
            fflassert(bar);

            for(uint32_t j = i + 1; j < DBCOM_BUFFACTENDID(); ++j){
                const auto &next_bar = DBCOM_BUFFACTRECORD(j);
                fflassert(next_bar);
                fflassert(!next_bar.isBuffAct(bar.name));
            }
        }
    }
}
s_buffActRecordAssertor {};

bool getClothGender(uint32_t itemID)
{
    if(const auto &ir = DBCOM_ITEMRECORD(itemID); ir && (to_u8sv(ir.type) == u8"衣服")){
        if(to_u8sv(ir.name).find(u8"（男）") != std::u8string_view::npos){
            return true;
        }
        else if(to_u8sv(ir.name).find(u8"（女）") != std::u8string_view::npos){
            return false;
        }
    }
    throw fflerror("invalid argument: itemID = %llu", to_llu(itemID));
}
