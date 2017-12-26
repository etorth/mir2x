/*
 * =====================================================================================
 *
 *       Filename: serverobject.hpp
 *        Created: 04/13/2016 20:04:39
 *  Last Modified: 12/25/2017 03:12:19
 *
 *    Description: basis of all objects in monoserver, with
 *
 *                   --ID()
 *                   --Active()
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
#include <atomic>
#include <string>
#include <cstdint>
#include <cstddef>
#include "invardata.hpp"
#include "uidrecord.hpp"

class ServerObject
{
    public:
        struct ClassCodeName
        {
            std::size_t Code;
            std::string Name;

            ClassCodeName(size_t nCode, const char *pName)
                : Code(nCode)
                , Name(pName ? pName : "")
            {}
        };

        struct ClassEntryItem
        {
            std::atomic<int> Ready;
            std::vector<ClassCodeName> Entry;

            ClassEntryItem()
                : Ready{0}
                , Entry()
            {}
        };

    private:
        const uint32_t m_UID;

    public:
        explicit ServerObject();
        virtual ~ServerObject() = default;

    public:
        uint32_t UID() const
        {
            return m_UID;
        }

    public:
        const char *ClassName() const
        {
            return typeid(*this).name();
        }

        size_t ClassCode() const
        {
            return typeid(*this).hash_code();
        }

    public:
        virtual InvarData GetInvarData() const
        {
            return {};
        }

    public:
        static const std::vector<ClassCodeName> &ClassEntry(size_t);

    public:
        const std::vector<ClassCodeName> &ClassEntry() const
        {
            return ServerObject::ClassEntry(ClassCode());
        }

    public:
        template<typename T> bool ClassFrom() const
        {
            for(const auto &rstCodeName: ClassEntry()){
                if(true
                        && rstCodeName.Name == typeid(T).name()
                        && rstCodeName.Code == typeid(T).hash_code()){
                    return true;
                }
            }
            return false;
        }

    protected:
        bool RegisterClass(size_t, const char *, const std::vector<ClassCodeName> &);

        template<typename T> bool RegisterClass(const std::vector<ClassCodeName> &rstParentEntry)
        {
            using RT = typename std::remove_cv<T>::type;
            if(std::is_same<RT, ServerObject>::value){
                if(rstParentEntry.empty()){
                    return RegisterClass(typeid(ServerObject).hash_code(), typeid(ServerObject).name(), {});
                }
            }else{
                if(true
                        &&  std::is_base_of<ServerObject, RT>::value
                        && !rstParentEntry.empty()){
                    return RegisterClass(typeid(RT).hash_code(), typeid(RT).name(), rstParentEntry);
                }
            }
            return false;
        }

        template<typename U, typename V> bool RegisterClass()
        {
            using RU = typename std::remove_cv<U>::type;
            using RV = typename std::remove_cv<V>::type;

            return true
                && !std::is_same   <RU,           RV>::value
                &&  std::is_base_of<ServerObject, RU>::value
                &&  std::is_base_of<ServerObject, RV>::value
                &&  std::is_base_of<RV,           RU>::value
                &&  RegisterClass<RU>(ClassEntry(typeid(RV).hash_code()));
        }

        // RegisterClass() is used in constructor only
        // and RegisterClass(*this) is ``using virtual function in constructor"
        template<typename T> bool RegisterClass(const T &rstT) = delete;
};
