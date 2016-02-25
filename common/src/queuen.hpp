/*
 * =====================================================================================
 *
 *       Filename: queuen.hpp
 *        Created: 02/25/2016 01:01:40
 *  Last Modified: 02/25/2016 01:37:58
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */


// for(stQN.Reset(); !stQN.End(); stQN.Next())
// {
//     stQN.Current()
// }

typename<int N, typename T>
class QueueN final
{
    public:
        QueueN()
            : m_Head(0)
            , m_Size(0)
            , m_Current(0)
        {
        }

        ~QueueN() = default;

        bool Size()
        {
            return m_Size;
        }

        void Reset()
        {
            m_Current = m_Head;
        }

        bool End()
        {
            return m_Current == m_Head + m_Size;
        }

        T &Current()
        {
            return m_CircleQ[m_Current];
        }

        void Forward()
        {
            m_Current = (m_Current + 1) % (N + 1);
        }

    private:
        std::array<N, T> m_CircleQ;
        int              m_Head;

};
