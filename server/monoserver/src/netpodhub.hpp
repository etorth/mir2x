/*
 * =====================================================================================
 *
 *       Filename: netpodhub.hpp
 *        Created: 05/24/2016 22:47:34
 *  Last Modified: 05/24/2016 22:53:29
 *
 *    Description: wrapper for SessionHub
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


class NetPodHub
{
    public:
        NetPod Get(uint32_t nNetID)
        {
            NetPod stNP;
            if(true
                    && nNetID > 0
                    && nNetID < m_SRV.size()
                    && m_SRV[nNetID].Session){
                stNP.m_Session = m_SRV[nNetID].Session;
                stNP.m_Lock    = &(m_SRV[nNetID].Lock);
            }

            return std::move(stNP);
        }
};
