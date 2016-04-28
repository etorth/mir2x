/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *  Last Modified: 04/28/2016 00:28:44
 *
 *    Description: split monoserver into actor-code and non-actor code
 *                 put all actor code in this class
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

#pragma once
#include <vector>
#include "transponder.hpp"

class ServiceCore: public Transponder
{
    private:
        typedef struct _PlayerRecord {
            uint32_t GUID;
            _PlayerRecord()
                : GUID(0)
            {}
        }PlayerRecord;

        std::vector<PlayerRecord> m_PlayerV;

    public:
        ServiceCore();
        virtual ~ServiceCore();

    public:
        void Operate(const MessagePack &, const Theron::Address &);

    protected:
        bool LoadMap(uint32_t);
};
