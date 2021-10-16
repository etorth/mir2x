#include "magicrecord.hpp"
#include "dbcomrecord.hpp"

const MagicGfxEntry &MagicRecord::getGfxEntry(const char8_t *stage) const
{
    if(stage && stage[0]){
        for(const auto &entry: gfxList){
            if(entry.ref && entry.ref.checkStage(stage)){
                if(const auto &mr = DBCOM_MAGICRECORD(entry.ref.name)){
                    if(const auto &refGfxEntry = mr.getGfxEntry(entry.ref.stage)){
                        return refGfxEntry;
                    }
                }
            }

            if(entry && entry.checkStage(stage)){
                return entry;
            }
        }
    }

    const static MagicGfxEntry s_emptyGfxEntry {};
    return s_emptyGfxEntry;
}
