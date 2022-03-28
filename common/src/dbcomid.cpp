#include <cstdint>
#include "dbcomid.hpp"
#include "fflerror.hpp"

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
