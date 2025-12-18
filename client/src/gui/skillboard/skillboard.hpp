#pragma once
#include <list>
#include <array>
#include <memory>
#include "strf.hpp"
#include "mathf.hpp"
#include "raiitimer.hpp"
#include "dbcomid.hpp"
#include "widget.hpp"
#include "magicrecord.hpp"
#include "labelboard.hpp"
#include "imageboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "textshadowboard.hpp"
#include "skillboardconfig.hpp"

class SkillPage;
class ProcessRun;

class SkillBoard: public Widget
{
    public:
        friend class SkillPage;

    public:
        struct MagicIconGfx
        {
            const uint32_t magicID = 0;
            const uint32_t magicIcon = SYS_U32NIL;

            const int x = 0;
            const int y = 0;

            const bool passive = false;
        };

        constexpr static MagicIconGfx m_iconGfxList []
        {
            // not only for currnet user
            // list all possible magics for all types of user

            // 火
            {DBCOM_MAGICID(u8"火球术"  ), 0X05001000, 0, 0, false},
            {DBCOM_MAGICID(u8"大火球"  ), 0X05001004, 0, 1, false},
            {DBCOM_MAGICID(u8"焰天火雨"), 0X05001042, 0, 3, false},
            {DBCOM_MAGICID(u8"地狱火"  ), 0X05001008, 2, 1, false},
            {DBCOM_MAGICID(u8"火墙"    ), 0X05001015, 2, 2, false},
            {DBCOM_MAGICID(u8"爆裂火焰"), 0X05001016, 4, 3, false},

            // 冰
            {DBCOM_MAGICID(u8"冰月神掌"), 0X05001025, 0, 0, false},
            {DBCOM_MAGICID(u8"冰月震天"), 0X05001026, 0, 1, false},
            {DBCOM_MAGICID(u8"冰沙掌"  ), 0X05001028, 2, 2, false},
            {DBCOM_MAGICID(u8"冰咆哮"  ), 0X0500101F, 2, 3, false},
            {DBCOM_MAGICID(u8"魄冰刺"  ), 0X05001050, 2, 4, false}, // no original gfx, hand-made gfx

            // 雷
            {DBCOM_MAGICID(u8"霹雳掌"  ), 0X05001027, 0, 0, false},
            {DBCOM_MAGICID(u8"雷电术"  ), 0X0500100A, 0, 1, false},
            {DBCOM_MAGICID(u8"疾光电影"), 0X05001009, 2, 2, false},
            {DBCOM_MAGICID(u8"地狱雷光"), 0X05001017, 2, 3, false},
            {DBCOM_MAGICID(u8"怒神霹雳"), 0X05001040, 2, 4, false},

            // 风
            {DBCOM_MAGICID(u8"风掌"    ), 0X05001029, 0, 0, false},
            {DBCOM_MAGICID(u8"击风"    ), 0X0500102C, 0, 1, false},
            {DBCOM_MAGICID(u8"风震天"  ), 0X0500102B, 2, 2, false},
            {DBCOM_MAGICID(u8"龙卷风"  ), 0X0500102A, 2, 3, false},
            {DBCOM_MAGICID(u8"抗拒火环"), 0X05001007, 4, 1, false},
            {DBCOM_MAGICID(u8"魔法盾"  ), 0X0500101D, 4, 2, false},

            // 神圣
            {DBCOM_MAGICID(u8"治愈术"    ), 0X05001001, 0, 0, false},
            {DBCOM_MAGICID(u8"群体治愈术"), 0X0500101B, 0, 2, false},
            {DBCOM_MAGICID(u8"回生术"    ), 0X0500102D, 0, 3, false},
            {DBCOM_MAGICID(u8"月魂断玉"  ), 0X05001023, 2, 1, false},
            {DBCOM_MAGICID(u8"月魂灵波"  ), 0X05001024, 2, 2, false},
            {DBCOM_MAGICID(u8"云寂术"    ), 0X05001043, 4, 3, false},
            {DBCOM_MAGICID(u8"阴阳法环"  ), 0X05001045, 4, 4, false},

            // 暗黑
            {DBCOM_MAGICID(u8"施毒术"    ), 0X05001005, 0, 0, false},
            {DBCOM_MAGICID(u8"困魔咒"    ), 0X0500100F, 0, 1, false},
            {DBCOM_MAGICID(u8"幽灵盾"    ), 0X0500100D, 1, 1, false},
            {DBCOM_MAGICID(u8"神圣战甲术"), 0X0500100E, 1, 2, false},
            {DBCOM_MAGICID(u8"强魔震法"  ), 0X05001032, 1, 3, false},
            {DBCOM_MAGICID(u8"猛虎强势"  ), 0X05001037, 1, 4, false},
            {DBCOM_MAGICID(u8"隐身术"    ), 0X05001011, 2, 0, false},
            {DBCOM_MAGICID(u8"集体隐身术"), 0X05001012, 2, 1, false},
            {DBCOM_MAGICID(u8"妙影无踪"  ), 0X05001044, 2, 4, false},
            {DBCOM_MAGICID(u8"灵魂火符"  ), 0X0500100C, 3, 0, false},

            // 幻影
            {DBCOM_MAGICID(u8"召唤骷髅"    ), 0X0500103B, 0, 0, false},
            {DBCOM_MAGICID(u8"召唤神兽"    ), 0X0500101C, 0, 1, false},
            {DBCOM_MAGICID(u8"超强召唤骷髅"), 0X05001010, 0, 2, false},
            {DBCOM_MAGICID(u8"移花接玉"    ), 0X05001046, 0, 3, false},
            {DBCOM_MAGICID(u8"诱惑之光"    ), 0X05001013, 2, 0, false},
            {DBCOM_MAGICID(u8"圣言术"      ), 0X0500101E, 2, 1, false},
            {DBCOM_MAGICID(u8"凝血离魂"    ), 0X05001041, 2, 3, false},
            {DBCOM_MAGICID(u8"瞬息移动"    ), 0X05001014, 3, 0, false},
            {DBCOM_MAGICID(u8"异形换位"    ), 0X0500103A, 3, 1, false},

            // 无
            {DBCOM_MAGICID(u8"基本剑术"  ), 0X05001002, 0, 0, true },
            {DBCOM_MAGICID(u8"攻杀剑术"  ), 0X05001006, 0, 1, true },
            {DBCOM_MAGICID(u8"刺杀剑术"  ), 0X0500100B, 0, 2, false},
            {DBCOM_MAGICID(u8"半月弯刀"  ), 0X05001018, 1, 3, false},
            {DBCOM_MAGICID(u8"翔空剑法"  ), 0X05001021, 1, 4, false},
            {DBCOM_MAGICID(u8"十方斩"    ), 0X05001039, 1, 5, false},
            {DBCOM_MAGICID(u8"烈火剑法"  ), 0X05001019, 2, 4, false},
            {DBCOM_MAGICID(u8"莲月剑法"  ), 0X05001020, 2, 5, false},
            {DBCOM_MAGICID(u8"野蛮冲撞"  ), 0X0500101A, 3, 3, false},
            {DBCOM_MAGICID(u8"乾坤大挪移"), 0X0500103D, 3, 4, false},
            {DBCOM_MAGICID(u8"铁布衫"    ), 0X05001038, 3, 5, false},
            {DBCOM_MAGICID(u8"斗转星移"  ), 0X0500103E, 3, 6, false},
            {DBCOM_MAGICID(u8"破血狂杀"  ), 0X0500103C, 3, 7, false},
            {DBCOM_MAGICID(u8"精神力战法"), 0X05001003, 4, 0, true },
            {DBCOM_MAGICID(u8"空拳刀法"  ), 0X05001022, 4, 3, false},
        };

    private:
        ProcessRun *m_processRun;

    private:
        int m_selectedTabIndex =  0;
        int m_cursorOnTabIndex = -1;

    private:
        SkillBoardConfig m_config;

    private:
        ImageBoard m_bg;

    private:
        Widget m_pageCanvas;

    private:
        std::vector<SkillPage *> m_skillPageList;

    private:
        std::vector<TritexButton *> m_tabButtonList;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_closeButton;

    public:
        SkillBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawTabName(Widget::ROIMap) const;
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        static Widget::ROI getPageRectange()
        {
            return {18, 44, 304, 329};
        }

    public:
        static int tabElem(int tabIndex)
        {
            fflassert(tabIndex >= 0);
            fflassert(tabIndex <= 7);

            if(tabIndex >= 0 && tabIndex <= 6){
                return MET_BEGIN + tabIndex;
            }
            else{
                return MET_NONE;
            }
        }

        int selectedElem() const
        {
            return tabElem(m_selectedTabIndex);
        }

        int cursorOnElem() const
        {
            return tabElem(m_cursorOnTabIndex);
        }

    public:
        auto & getConfig(this auto && self)
        {
            return self.m_config;
        }

    private:
        static int getSkillPageIndex(uint32_t magicID)
        {
            if(!magicID){
                return -1;
            }

            const auto &mr = DBCOM_MAGICRECORD(magicID);
            if(!mr){
                return -1;
            }

            const int elemID = magicElemID(mr.elem);
            if(elemID == MET_NONE){
                return 7;
            }
            else if(elemID >= MET_BEGIN && elemID < MET_END){
                return elemID - MET_BEGIN;
            }
            else{
                return -1;
            }
        }

    public:
        static const MagicIconGfx *getMagicIconGfx(uint32_t magicID)
        {
            for(const auto &magicGfx: m_iconGfxList){
                if(magicGfx.magicID == magicID){
                    return std::addressof(magicGfx);
                }
            }
            return nullptr;
        }
};
