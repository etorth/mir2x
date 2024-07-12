#pragma once
#include <list>
#include <deque>
#include <array>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <climits>
#include <optional>
#include <concepts>
#include <optional>
#include <SDL2/SDL.h>

#include "uidf.hpp"
#include "mathf.hpp"
#include "totype.hpp"
#include "colorf.hpp"
#include "fflerror.hpp"
#include "focustype.hpp"
#include "serdesmsg.hpp"
#include "actionnode.hpp"
#include "labelboard.hpp"
#include "motionnode.hpp"
#include "protocoldef.hpp"
#include "attachmagic.hpp"

class ProcessRun;
class ClientCreature
{
    protected:
        const uint64_t m_UID;

    protected:
        ProcessRun *m_processRun;

    protected:
        std::optional<SDHealth> m_sdHealth;
        std::optional<SDBuffIDList> m_sdBuffIDList;

    protected:
        std::unique_ptr<MotionNode> m_currMotion;

    protected:
        std::list<std::unique_ptr<AttachMagic>> m_attachMagicList;

    protected:
        uint32_t m_lastActive;
        uint32_t m_lastQuerySelf;

    protected:
        double m_lastUpdateTime = 0.0;
        double m_accuUpdateTime = 0.0;

    protected:
        LabelBoard m_nameBoard;

    protected:
        ClientCreature(uint64_t uid, ProcessRun *pRun)
            : m_UID(uid)
            , m_processRun(pRun)
            , m_lastActive(0)
            , m_lastQuerySelf(0)
            , m_nameBoard(DIR_UPLEFT, 0, 0, u8"ClientCreature", 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0XFF, 0X00))
        {
            if(!(m_UID && m_processRun)){
                throw fflerror("invalid argument: UID = %llu, processRun = %p", to_llu(m_UID), to_cvptr(m_processRun));
            }
        }

    public:
        virtual ~ClientCreature() = default;

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

        int type() const
        {
            return uidf::getUIDType(UID());
        }

    public:
        static SDL_Color focusColor(int focusChan)
        {
            switch(focusChan){
                case FOCUS_MOUSE : return colorf::RGBA2SDLColor(0XFF, 0X86, 0X00, 0XFF);
                case FOCUS_MAGIC : return colorf::RGBA2SDLColor(0X92, 0XC6, 0X20, 0XFF);
                case FOCUS_FOLLOW: return colorf::RGBA2SDLColor(0X00, 0XC6, 0XF0, 0XFF);
                case FOCUS_ATTACK: return colorf::RGBA2SDLColor(0XD0, 0X2C, 0X70, 0XFF);
                default          : return colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF);
            }
        }

    public:
        virtual bool canFocus(int pointX, int pointY) const
        {
            return getTargetBox().in(pointX, pointY);
        }

    public:
        uint32_t lastActive() const
        {
            return m_lastActive;
        }

        uint32_t lastQuerySelf() const
        {
            return m_lastQuerySelf;
        }

    public:
        int x() const
        {
            return std::get<0>(location());
        }

        int y() const
        {
            return std::get<1>(location());
        }

    public:
        const auto currMotion() const
        {
            if(m_currMotion){
                return m_currMotion.get();
            }
            throw fflerror("creature has no current motion: %p", to_cvptr(this));
        }

    public:
        virtual std::tuple<int, int> location() const = 0;

    public:
        virtual int getFrameCount(const MotionNode *) const = 0;

    public:
        int getFrameCountEx(const MotionNode *motionPtr) const
        {
            if(const int count = getFrameCount(motionPtr); count > 0){
                return count;
            }
            throw fflerror("invalid arguments: motion %d, direction %d", motionPtr->type, motionPtr->direction);
        }

    protected:
        virtual bool moveNextMotion() = 0;

    public:
        virtual bool parseAction(const ActionNode &) = 0;

    protected:
        virtual bool advanceMotionFrame();

    public:
        bool updateMotion(bool allowSwitchMotion)
        {
            if(m_currMotion->frame + 1 < getFrameCountEx(m_currMotion.get())){
                return advanceMotionFrame();
            }
            else if(allowSwitchMotion){
                return moveNextMotion();
            }
            else{
                return true; // stuck at last frame
            }
        }

    protected:
        void updateAttachMagic(double);

    public:
        virtual bool update(double) = 0;
        virtual void drawFrame(int, int, int, int, bool) = 0;

    public:
        virtual void draw(int viewX, int viewY, int focusMask)
        {
            drawFrame(viewX, viewY, focusMask, m_currMotion->frame, false);
        }

    public:
        virtual bool motionValid(const std::unique_ptr<MotionNode> &) const = 0;

    public:
        void updateHealth(SDHealth);

    public:
        void setBuffIDList(SDBuffIDList);

    public:
        const auto &getSDHealth()
        {
            return m_sdHealth;
        }

    public:
        const auto &getSDBuffIDList() const
        {
            return m_sdBuffIDList;
        }

    public:
        std::array<double, 2> getHealthRatio() const
        {
            if(m_sdHealth.has_value()){
                return
                {
                    (m_sdHealth.value().getMaxHP() > 0) ? to_df(m_sdHealth.value().hp) / m_sdHealth.value().getMaxHP() : 1.0,
                    (m_sdHealth.value().getMaxMP() > 0) ? to_df(m_sdHealth.value().mp) / m_sdHealth.value().getMaxMP() : 1.0,
                };
            }
            else{
                return { 1.0, 1.0};
            }
        }

    public:
        virtual bool deadFadeOut() = 0;

    public:
        virtual bool   alive() const;
        virtual bool  active() const;
        virtual bool visible() const;

    protected:
        virtual std::unique_ptr<MotionNode> makeIdleMotion() const = 0;

    public:
        AttachMagic *addAttachMagic(std::unique_ptr<AttachMagic> magicPtr)
        {
            m_attachMagicList.emplace_back(std::move(magicPtr));
            return m_attachMagicList.back().get();
        }

    public:
        bool checkUpdate(double);

    public:
        void querySelf();

    public:
        struct TargetBox
        {
            const int x = -1;
            const int y = -1;
            const int w = -1;
            const int h = -1;

            operator bool () const
            {
                return x >= 0 && y >= 0 && w > 0 && h > 0;
            }

            std::tuple<int, int> centerPLoc() const
            {
                return
                {
                    x + w / 2,
                    y + h / 2,
                };
            }

            std::tuple<int, int> targetPLoc() const
            {
                return
                {
                    x + w / 2,
                    y + h / 4,
                };
            }

            bool in(int argX, int argY) const
            {
                return this[0] && mathf::pointInRectangle(argX, argY, x, y, w, h);
            }
        };

    protected:
        static TargetBox getTargetBoxHelper(int startX, int startY, int bodyFrameW, int bodyFrameH)
        {
            const int maxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
            const int maxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;
            return
            {
                .x = startX + std::max<int>((bodyFrameW - maxTargetW) / 2, 0),
                .y = startY + std::max<int>((bodyFrameH - maxTargetH) / 2, 0),
                .w = std::min<int>(bodyFrameW, maxTargetW),
                .h = std::min<int>(bodyFrameH, maxTargetH),
            };
        }

    public:
        virtual TargetBox getTargetBox() const = 0;

    protected:
        class CallOnExitHelper final
        {
            private:
                std::function<void()> m_cb;

            public:
                CallOnExitHelper(std::function<void()> f)
                    : m_cb(std::move(f))
                {}

                ~CallOnExitHelper()
                {
                    if(m_cb){
                        m_cb();
                    }
                }
        };

    protected:
        bool isMonster(const char8_t *) const;

    public:
        virtual void setBuff(int, int);

    public:
        void playSoundEffect(uint32_t);
};
