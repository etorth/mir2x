#pragma once

template<typename T>
class SyncResQueue
{
    public:
        SyncResQueue();
        ~SyncResQueue();

    public:
        template<typename... ArgList>
        inline void PushNew(ArgList&&... stArgList)
        {
            if(!m_UsedQueue.empty()){
                auto p = &(*(m_UsedQueue.front()));
                p->Set(std::forward<ArgList>(stArgList)...);
                m_Queue.push(p);
                m_UsedQueue.pop();
            }else{
                m_Queue.push(new T(std::forward<ArgList>(stArgList)...));
            }
        }

        inline void Pop()
        {
            m_UsedQueue.push(m_Queue.front());
            m_Queue.pop();
        }
}
