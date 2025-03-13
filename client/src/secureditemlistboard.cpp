#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "purchaseboard.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SecuredItemListBoard::SecuredItemListBoard(int argX, int argY, ProcessRun *runProc, Widget *widgetPtr, bool autoDelete)
    : ItemListBoard
      {
          argX,
          argY,
          widgetPtr,
          autoDelete,
      }
    , m_processRun(runProc)
{}

const SDItem &SecuredItemListBoard::getItem(size_t index) const
{
    if(index < itemCount()){
        return m_itemList.at(index);
    }

    const static SDItem nullItem;
    return nullItem;
}

std::u8string SecuredItemListBoard::getGridHeader(size_t index) const
{
    if(const auto &item = getItem(index)){
        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir);

        if(ir.packable() && (item.count > 0)){
            return to_u8cstr(str_ksep(item.count));
        }
    }
    return {};
}

std::u8string SecuredItemListBoard::getGridHoverLayout(size_t index) const
{
    if(const auto &item = getItem(index)){
        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir);

        return str_printf(
            u8R"###( <layout>  )###""\n"
            u8R"###(     %s    )###""\n"
            u8R"###(     %s    )###""\n"
            u8R"###( </layout> )###""\n",

            xmlf::toParString("【名称】%s", ir.name).c_str(),
            xmlf::toParString("%s", str_haschar(ir.description) ? ir.description : u8"暂无描述").c_str()
        );
    }
    return {};
}

void SecuredItemListBoard::onSelect()
{
    if(m_selectedPageGrid.has_value()){
        fflassert(m_selectedPageGrid.value() < itemCount());
        if(const auto &item = getItem(m_selectedPageGrid.value())){
            m_processRun->requestRemoveSecuredItem(item.itemID, item.seqID);
        }
    }
}
