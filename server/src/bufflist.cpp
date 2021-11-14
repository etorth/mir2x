#include <vector>
#include "bufflist.hpp"

std::tuple<uint32_t, uint32_t> BuffList::rollAttackModifier()
{
    return {0, 0};
}

#define _decl_func_has_buff_act(T, F, isT) std::vector<T *> F(const char8_t *name) \
{ \
    fflassert(str_haschar(name)); \
    std::vector<T *> result; \
 \
    for(auto &p: m_buffList){ \
        for(auto &[tpsCount, ptr]: p->m_runList){ \
            const auto &bar = DBCOM_BUFFACTRECORD(name); \
            fflassert(bar); \
 \
            if(bar.isT() && bar.isBuffAct(name)){ \
                auto actp = dynamic_cast<T *>(ptr.get()); \
                fflassert(actp); \
                result.push_back(actp); \
            } \
        } \
    } \
    return result; \
}

_decl_func_has_buff_act(BaseBuffActAura             , BuffList::hasAura             , isAura             )
_decl_func_has_buff_act(BaseBuffActController       , BuffList::hasController       , isController       )
_decl_func_has_buff_act(BaseBuffActTrigger          , BuffList::hasTrigger          , isTrigger          )
_decl_func_has_buff_act(BaseBuffActAttributeModifier, BuffList::hasAttributeModifier, isAttributeModifier)
_decl_func_has_buff_act(BaseBuffActAttackModifier   , BuffList::hasAttackModifier   , isAttackModifier   )
_decl_func_has_buff_act(BaseBuffActSpellModifier    , BuffList::hasSpellModifier    , isSpellModifier    )

#undef _decl_func_has_buff_act
