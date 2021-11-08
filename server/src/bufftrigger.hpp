#pragma once
#include <memory>
#include "dbcomid.hpp"
#include "fflerror.hpp"

class BattleObject;
class BaseBuffTrigger
{
    private:
        const uint32_t m_id;

    private:
        const int m_arg;

    public:
        BaseBuffTrigger(uint32_t id, int arg)
            : m_id([id]()
              {
                  fflassert(id > 0);
                  fflassert(id < DBCOM_BUFFTRIGGERENDID());
                  return id;
              }())
            , m_arg(arg)
        {}

    public:
            virtual ~BaseBuffTrigger() = default;

    public:
            virtual void runOnTrigger(BattleObject *, int) = 0;

    public:
            static std::unique_ptr<BaseBuffTrigger> createTrigger(uint32_t, int);

    public:
            uint32_t id() const
            {
                return m_id;
            }
};

template<uint32_t> class BuffTrigger;
template<> class BuffTrigger<DBCOM_BUFFTRIGGERID(u8"HP恢复")>: public BaseBuffTrigger
{
    public:
        BuffTrigger(int arg)
            : BaseBuffTrigger(DBCOM_BUFFTRIGGERID(u8"HP恢复"), arg)
        {}

    public:
        void runOnTrigger(BattleObject *, int);
};

template<> class BuffTrigger<DBCOM_BUFFTRIGGERID(u8"HP伤害")>: public BaseBuffTrigger
{
public:
    BuffTrigger(int arg)
        : BaseBuffTrigger(DBCOM_BUFFTRIGGERID(u8"HP伤害"), arg)
    {}

public:
    void runOnTrigger(BattleObject *, int);
};
