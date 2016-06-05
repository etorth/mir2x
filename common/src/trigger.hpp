/*
 * =====================================================================================
 *
 *       Filename: trigger.hpp
 *        Created: 06/05/2016 02:03:44
 *  Last Modified: 06/05/2016 02:58:32
 *
 *    Description: I decide to make a general class with name trigger, trigger should
 *                 be driven by other loop, like actor message operation handling, or
 *                 time loop, etc, trigger won't occupy a thread to execute
 *
 *                 class Trigger will hold a operation queue, each operation is a
 *                 std::function<bool()>, when return true, means this operation has
 *                 been ``done" and should be deleted from the queue, otherwise it's
 *                 should be envaluated again next time when calling Execute().
 *
 *                 I don't plan to support ``disabled" state, an operation is either
 *                 active or done, when done we just remove it from the queue, if you
 *                 want to keep an operation in the queue but don't remove it till
 *                 some condition meets, then why not remove it and install again when
 *                 possible?
 *
 *                 but I support to get the raw handler by name, this could help. and
 *                 an operation couldn't get its name during invoking, if we need to
 *                 do this we should do it like this:
 *
 *                      std::string szOpName = "operation";
 *                      auto fnOp = [this, szOpName](){
 *                          // refer to szOpName
 *                      };
 *
 *                      Install(szOpName, fnOp);
 *
 *                 then you can refer to the operation name, retrieve the handler itself
 *                 and do whatever you want
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
#include <utility>
#include <functional>

class Trigger final
{
    private:
        // record for an operation
        // 1. operation name
        // 2. operation handler
        // 3. operation is active
        using OperationRecord = std::tuple<std::string, std::function<bool()>, bool>;

    private:
        std::vector<OperationRecord> m_OperationV;

    public:
        Trigger()
            : m_OperationV()
        {}

        ~Trigger() = default;

    public:
        void Execute()
        {
            size_t nIndex = 0;
            while(nIndex < m_OperationV.size()){
                if(true
                        && std::get<1>(m_OperationV[nIndex])    // callable
                        && std::get<2>(m_OperationV[nIndex])){  // active
                    // 1. invoke the operation handler
                    bool bDone = std::get<1>(m_OperationV[nIndex])();

                    // 2. mark it's status
                    //    TODO: when it's done, we set status as inactive
                    std::get<2>(m_OperationV[nIndex]) = !bDone;

                    // 3. next
                    nIndex++;
                    continue;
                }

                // ok current trigger handler should be deleted
                // we delete disabled & invalid handler everytime we found it

                std::swap(m_OperationV[nIndex], m_OperationV.back());
                m_OperationV.pop_back();
            }
        }

        // 1. inactive operation won't count
        // 2. I return a copy rather than a reference since the operation
        //    may soon be deleted
        std::function<bool()> Operation(const std::string &szOperationName)
        {
            for(auto &rstEle: m_OperationV){
                if(true
                        && (std::get<0>(rstEle) == szOperationName)   // name matching
                        && (std::get<1>(rstEle))                      // callable
                        && (std::get<2>(rstEle))){                    // still active
                    return std::get<1>(rstEle);
                }
            }

            return std::function<bool()>();
        }

        // just try to match, and overwrite the matched slot
        void Install(const std::string &szOperationName, const std::function<bool()> &fnTriggerOp)
        {
            for(auto &rstEle: m_OperationV){
                if(std::get<0>(rstEle) == szOperationName){
                    std::get<1>(rstEle) = fnTriggerOp;
                    std::get<2>(rstEle) = true;
                    return;
                }
            }
            m_OperationV.emplace_back(std::make_tuple(szOperationName, fnTriggerOp, true));
        }

        // anymous operation, can only be removed by return value
        void Install(const std::function<bool()> &fnTriggerOp)
        {
            std::string szRandomName;
            while(true){
                szRandomName.clear();
                for(int nCount = 0; nCount < 20; ++nCount){
                    szRandomName.push_back('A' + std::rand() % ('Z' - 'A' + 1));
                }

                if(!Installed(szRandomName)){ break; }
            }

            Install(szRandomName, fnTriggerOp);
        }

        // disabled or invalid handler won't count
        bool Installed(const std::string &szOperationName)
        {
            for(auto &rstEle: m_OperationV){
                if(true
                        && (std::get<0>(rstEle) == szOperationName)   // name matching
                        && (std::get<1>(rstEle))                      // callable
                        && (std::get<2>(rstEle))){                    // still active
                    return true;
                }
            }
            return false;
        }

        // uninstall a trigger, parameters
        // szOperationName: trigger to be uninstalled
        // bInside        : true if we only disable it and the pod will delete next time
        //                : false will make the trigger deleted immediately
        //
        // call Uninstall("Name", true) is the same as the operation return true
        void Uninstall(const std::string &szOperationName, bool bInside = true)
        {
            size_t nIndex = 0;
            while(nIndex < m_OperationV.size()){
                if(true
                        && (std::get<0>(m_OperationV[nIndex]) != szOperationName) // not the one
                        && (std::get<1>(m_OperationV[nIndex]))                    // callable
                        && (std::get<2>(m_OperationV[nIndex]))){                  // active
                    nIndex++;
                    continue;
                }

                // ok current trigger handler should be deleted
                // we delete disabled & invalid handler everytime we found it

                // 1. disable it always
                std::get<2>(m_OperationV[nIndex]) = false;

                // 2. if inside we have to jump to next
                if(bInside){ nIndex++; continue; }

                // 3. otherwise we delete now
                std::swap(m_OperationV[nIndex], m_OperationV.back());
                m_OperationV.pop_back();
            }
        }

        // uninstall all
        void Uninstall(bool bInside = true)
        {
            if(!bInside){ m_OperationV.clear(); return; }

            for(auto &rstEle: m_OperationV){
                Uninstall(std::get<0>(rstEle), true);
            }
        }
};
