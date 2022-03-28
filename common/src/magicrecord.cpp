#include <memory>
#include "dbcomid.hpp"
#include "magicrecord.hpp"

std::pair<const MagicGfxEntry &, const MagicGfxEntryRef &> MagicRecord::getGfxEntry(const char8_t *stage) const
{
    if(stage && stage[0]){
        for(const auto &entry: gfxList){
            if(entry.ref && entry.ref.checkStage(stage)){
                if(const auto &mr = DBCOM_MAGICRECORD(entry.ref.name)){
                    if(const auto &[refGfxEntry, refGfxEntryFrom] = mr.getGfxEntry(entry.ref.stage); refGfxEntry){
                        return {refGfxEntry, entry.ref};
                    }
                }
            }

            if(entry && entry.checkStage(stage)){
                return {entry, entry.ref};
            }
        }
    }

    const static MagicGfxEntry s_emptyGfx {};
    return {s_emptyGfx, s_emptyGfx.ref};
}
