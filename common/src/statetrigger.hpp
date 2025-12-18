#pragma once
#include <list>
#include <utility>
#include <functional>
#include "fflerror.hpp"

class StateTrigger final
{
    private:
        std::list<std::function<bool()>> m_triggerList;

    public:
        void run()
        {
            for(auto p = m_triggerList.begin(); p != m_triggerList.end();){
                if(*p){
                    if((*p)()){
                        p = m_triggerList.erase(p);
                    }
                    else{
                        p++;
                    }
                }
                else{
                    throw fflerror("trigger is not executable");
                }
            }
        }

    public:
        void install(std::function<bool()> trigger)
        {
            m_triggerList.push_back(std::move(trigger));
        }

    public:
        bool empty() const
        {
            return m_triggerList.empty();
        }
};
