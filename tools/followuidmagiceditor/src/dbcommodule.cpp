// followuidmagiceditor: python module of read-only DBCOM magic queries, header-only, used by main.py
//
//     import followuidmagiceditor
//     followuidmagiceditor.magic_names()                    # ["物理攻击", ...] in magic ID order
//     followuidmagiceditor.magic_record("火球术")            # {"id", "name", "elem", "gfxList"}, raises KeyError
//     followuidmagiceditor.magic_gfx_entry("火球术", "运行")  # entry resolved through DBCOM_MAGICGFXENTRY, or None
//
// a gfx entry is a dict, gfxID is None if the entry has no gfx
// magic_gfx_entry() follows .ref, its "ref" is {"name", "stage"} of the referencing entry, or None

#include <string>
#include <string_view>
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include "dbcomid.hpp"

namespace nb = nanobind;

namespace
{
    nb::object u8Str(const char8_t *s)
    {
        if(s){
            return nb::str(reinterpret_cast<const char *>(s));
        }
        return nb::none();
    }

    std::u8string_view u8View(const std::string &s)
    {
        return {reinterpret_cast<const char8_t *>(s.data()), s.size()};
    }

    nb::object refDict(const MagicGfxEntryRef &ref)
    {
        if(!ref){
            return nb::none();
        }

        nb::dict d;
        d["name"    ] = u8Str(ref.name);
        d["stage"   ] = u8Str(ref.stage);
        d["modColor"] = ref.modColor;
        return d;
    }

    nb::dict gfxEntryDict(const MagicGfxEntry &entry)
    {
        nb::dict d;
        d["stage"     ] = u8Str(entry.stage);
        d["type"      ] = u8Str(entry.type);
        d["gfxID"     ] = (entry.gfxID == SYS_U32NIL) ? nb::none() : nb::cast(entry.gfxID);
        d["modColor"  ] = entry.modColor;
        d["frameCount"] = entry.frameCount;
        d["gfxDirType"] = entry.gfxDirType;
        d["onGround"  ] = entry.onGround;
        d["motion"    ] = entry.motion;
        d["loop"      ] = entry.loop;
        d["speed"     ] = entry.speed;
        d["ref"       ] = refDict(entry.ref);
        return d;
    }
}

NB_MODULE(followuidmagiceditor, m)
{
    m.doc() = "read-only DBCOM magic queries";

    m.def("magic_names", []()
    {
        nb::list names;
        for(uint32_t id = 1; id < DBCOM_MAGICENDID(); ++id){
            if(const auto &mr = DBCOM_MAGICRECORD(id)){
                names.append(u8Str(mr.name));
            }
        }
        return names;
    });

    m.def("magic_record", [](const std::string &name)
    {
        const auto magicID = DBCOM_MAGICID(name.c_str());
        if(!magicID){
            throw nb::key_error(name.c_str());
        }

        const auto &mr = DBCOM_MAGICRECORD(magicID);
        nb::list gfxList;

        for(const auto &entry: mr.gfxList){
            gfxList.append(gfxEntryDict(entry));
        }

        nb::dict d;
        d["id"     ] = magicID;
        d["name"   ] = u8Str(mr.name);
        d["elem"   ] = u8Str(mr.elem);
        d["gfxList"] = gfxList;
        return d;
    }, nb::arg("name"));

    m.def("magic_gfx_entry", [](const std::string &name, const std::string &stage) -> nb::object
    {
        if(const auto [entry, ref] = DBCOM_MAGICGFXENTRY(u8View(name), u8View(stage)); entry){
            auto d = gfxEntryDict(*entry);
            d["ref"] = ref ? refDict(*ref) : nb::none();
            return d;
        }
        return nb::none();
    }, nb::arg("name"), nb::arg("stage"));
}
