/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 05/23/2016 18:22:01
 *  Last Modified: 08/18/2017 15:30:04
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

#include "log.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"

extern MonoServer *g_MonoServer;
ServerObject::ServerObject()
    : m_UID(g_MonoServer->GetUID())
{
    // 1. link to the mono object pool
    //    never allocate ServerObject on stack
    //    MonoServer::EraseUID() will use *this* to call destructor
    g_MonoServer->LinkUID(m_UID, this);

    // 2. use std::call_once to register current class
    //    use empty vector {} as it's parents' class entry info
    //    since server object is the base class for all other classes need registration
    auto fnRegisterClass = [this]()
    {
        if(!RegisterClass<ServerObject>({})){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for ServerObject failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
}

static std::array<ServerObject::ClassEntryItem, 997> s_ClassEntryV;

// always use std::call_once() for class registration
// registration will exchange current Ready state as 1 then do registration if needed
// this could cause busy looping for other thread calling ClassFrom() call, so don't make registration too often
bool ServerObject::RegisterClass(size_t nCode, const char *pName, const std::vector<ClassCodeName> &rstParentEntry)
{
    if(pName){
        auto nEntryCode0 = nCode % s_ClassEntryV.size();
        auto nEntryCode1 = nEntryCode0;

        while(true){
            auto &rstCurrEntry = s_ClassEntryV[nEntryCode1];
            switch(auto nState = rstCurrEntry.Ready.exchange(1)){
                case 0:
                    {
                        rstCurrEntry.Entry = rstParentEntry;
                        rstCurrEntry.Entry.emplace_back(nCode, pName);

                        rstCurrEntry.Ready.store(2);
                        return true;
                    }
                case 1:
                    {
                        break;
                    }
                case 2:
                    {
                        condcheck(!rstCurrEntry.Entry.empty());
                        if(rstCurrEntry.Entry.back().Code == nCode){
                            bool bAgree = true;
                            if(rstParentEntry.size() + 1 == rstCurrEntry.Entry.size()){
                                for(size_t nIndex = 0; nIndex < rstParentEntry.size(); ++nIndex){
                                    if(false
                                            || rstParentEntry[nIndex].Code != rstCurrEntry.Entry[nIndex].Code
                                            || rstParentEntry[nIndex].Name != rstCurrEntry.Entry[nIndex].Name){
                                        bAgree = false;
                                        break;
                                    }
                                }
                            }

                            if(true
                                    && bAgree
                                    && rstCurrEntry.Entry.back().Code == nCode
                                    && rstCurrEntry.Entry.back().Name == pName){
                                rstCurrEntry.Ready.store(2);
                                return true;
                            }

                            // hoho don't agress with each other
                            // solution-1: use mine to overwrite his initialization
                            //          2: report an error
                            //
                            // I choose solution 2, reason:
                            // this is the only chance that an ``finished slot" may be accessed not in READ-ONLY method
                            // if we overwrite it then some other threads that already get the const ref could run into trouble

                            rstCurrEntry.Ready.store(2);

                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Initialization for slot %d doesn't agree with the existing one", (int)(nEntryCode1));
                            return false;
                        }

                        // reset it as finised and try next slot
                        // this slot has been taken for other classes
                        rstCurrEntry.Ready.store(2);
                        nEntryCode1 = (nEntryCode1 + 1) % s_ClassEntryV.size();
                        break;
                    }
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid slot status: (Slot = %d, State = %d)", (int)(nEntryCode1), nState);
                        return false;
                    }
            }

            // try out all slot and no one is valid now
            if(nEntryCode1 == nEntryCode0){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "No valid slot for class registration: (ClassName = %s, ClassCode = %llu)", pName, (unsigned long long)(nCode));
                return false;
            }
        }
    }else{ return false; }
}

// retrieve class registration information
// for class already registered in the table this return a const ref of it
const std::vector<ServerObject::ClassCodeName> &ServerObject::ClassEntry(size_t nClassCode)
{
    // empty parent class for non-valid
    // remember for ServerObject it self we have the return vector with size 1
    static std::vector<ClassCodeName> stNullEntry {};

    auto nEntryCode0 = nClassCode % s_ClassEntryV.size();
    auto nEntryCode1 = nEntryCode0;
    while(true){
        auto &rstCurrEntry = s_ClassEntryV[nEntryCode1];
        switch(auto nState = rstCurrEntry.Ready.load()){
            case 0:
                {
                    return stNullEntry;
                }
            case 1:
                {
                    // someone is doing initialization
                    // use std::call_once() to do initialization to avoid it
                    break;
                }
            case 2:
                {
                    // if an entry is ready
                    // then this entry should only be accessed in READ-ONLY mode
                    condcheck(!rstCurrEntry.Entry.empty());
                    if(rstCurrEntry.Entry.back().Code == nClassCode){
                        // we can do some verification if needed like
                        // 1. entry.Name is not empty
                        // 2. entry.Code should also exist in current table
                        return rstCurrEntry.Entry;
                    }else{
                        nEntryCode1 = (nEntryCode1 + 1) % s_ClassEntryV.size();
                    }
                    break;
                }
            default:
                {
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid slot status: (Slot = %d, State = %d)", (int)(nEntryCode1), nState);
                    return stNullEntry;
                }
        }

        if(nEntryCode1 == nEntryCode0){ return stNullEntry; }
    }

    // impossible, make the compiler happy
    return stNullEntry;
}
