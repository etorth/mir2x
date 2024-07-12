#pragma once
#include <list>
#include <array>
#include <memory>
#include "strf.hpp"
#include "mathf.hpp"
#include "widget.hpp"
#include "raiitimer.hpp"
#include "labelboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"
#include "labelshadowboard.hpp"

class ProcessRun;
class SkillBoard: public Widget
{
    public:
        struct MagicIconGfx
        {
            const uint32_t magicID = 0;
            const uint32_t magicIcon = SYS_U32NIL;

            const int x = 0;
            const int y = 0;

            const bool passive = false;

            operator bool () const
            {
                return magicID != 0;
            }
        };

    public:
        class SkillBoardConfig final
        {
            private:
                struct MagicConfig
                {
                    int level = -1;
                    std::optional<char> key;
                };

            private:
                std::unordered_map<uint32_t, MagicConfig> m_learnedMagicList;

            public:
                std::optional<char> getMagicKey  (uint32_t) const;
                std::optional<int > getMagicLevel(uint32_t) const;

            public:
                bool hasMagicID(uint32_t magicID) const
                {
                    return m_learnedMagicList.count(magicID) > 0;
                }

            public:
                void setMagicLevel(uint32_t, int);
                void setMagicKey  (uint32_t, std::optional<char>);

            public:
                auto getMagicKeyList() const
                {
                    std::vector<std::tuple<uint32_t, char>> result;
                    for(const auto &p: m_learnedMagicList){
                        if(p.second.key.has_value()){
                            result.emplace_back(p.first, p.second.key.value());
                        }
                    }

                    std::sort(result.begin(), result.end());
                    return result;
                }

            public:
                uint32_t key2MagicID(char key) const
                {
                    for(const auto &p: m_learnedMagicList){
                        if(p.second.key.has_value() && p.second.key.value() == key){
                            return p.first;
                        }
                    }
                    return 0;
                }
        };

    private:
        class MagicIconButton: public Widget
        {
            // +-+-----+
            // |A|     |
            // +-+     |
            // |       |
            // +-------+-+
            //         |1|
            //         +-+

            private:
                const uint32_t m_magicID;

            private:
                SkillBoardConfig * const m_config;
                ProcessRun       * const m_processRun;

            private:
                TritexButton m_icon;

            public:
                MagicIconButton(int, int, uint32_t, SkillBoardConfig *, ProcessRun *, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void drawEx(int, int, int, int, int, int) const override;

            public:
                bool processEvent(const SDL_Event &, bool) override;

            public:
                bool cursorOn() const
                {
                    return m_icon.getState() != BEVENT_OFF;
                }

            public:
                uint32_t magicID() const
                {
                    return m_magicID;
                }
        };

        class SkillPage: public Widget
        {
            private:
                SkillBoardConfig * const m_config;
                ProcessRun       * const m_processRun;

            private:
                const uint32_t m_pageImage;
                std::vector<SkillBoard::MagicIconButton *> m_magicIconButtonList;

            public:
                SkillPage(uint32_t, SkillBoardConfig *, ProcessRun *proc, Widget *widgetPtr = nullptr, bool autoDelete = false);

            public:
                void addIcon(uint32_t argMagicID)
                {
                    for(auto buttonCPtr: m_magicIconButtonList){
                        if(buttonCPtr->magicID() == argMagicID){
                            return;
                        }
                    }

                    fflassert(DBCOM_MAGICRECORD(argMagicID));
                    const auto &iconGfx = SkillBoard::getMagicIconGfx(argMagicID);

                    fflassert(iconGfx);
                    m_magicIconButtonList.push_back(new SkillBoard::MagicIconButton
                    {
                        iconGfx.x * 60 + 12,
                        iconGfx.y * 65 + 13,
                        argMagicID,
                        m_config,
                        m_processRun,
                        this,
                        true,
                    });
                }

            public:
                void drawEx(int, int, int, int, int, int) const override;

            public:
                const auto &getMagicIconButtonList() const
                {
                    return m_magicIconButtonList;
                }
        };

    private:
        SkillBoardConfig m_config;
        std::vector<SkillPage *> m_skillPageList;

    private:
        // no need to introduce a new type
        // use two tritex button to micmic the tab button
        int m_selectedTabIndex =  0;
        int m_cursorOnTabIndex = -1;
        std::vector<TritexButton *> m_tabButtonList;

    private:
        TexSlider m_slider;

    private:
        TritexButton m_closeButton;

    private:
        ProcessRun *m_processRun;

    public:
        SkillBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawTabName() const;
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    private:
        static std::array<int, 4> getPageRectange()
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
        /* */ SkillBoardConfig &getConfig()       { return m_config; }
        const SkillBoardConfig &getConfig() const { return m_config; }

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
        static const auto &getMagicIconGfxList()
        {
            const static std::vector<MagicIconGfx> s_iconGfxList
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
                {DBCOM_MAGICID(u8"精神力战法"), 0X05001003, 4, 0, false},
                {DBCOM_MAGICID(u8"空拳刀法"  ), 0X05001022, 4, 3, false},
            };
            return s_iconGfxList;
        }

        static const MagicIconGfx &getMagicIconGfx(uint32_t magicID)
        {
            for(const auto &magicGfx: getMagicIconGfxList()){
                if(magicGfx.magicID == magicID){
                    return magicGfx;
                }
            }

            const static MagicIconGfx s_emptyGfx {};
            return s_emptyGfx;
        }
};
