/*
 * =====================================================================================
 *
 *       Filename: configuration.cpp
 *        Created: 6/17/2015 6:25:24 PM
 *  Last Modified: 03/10/2016 17:26:48
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
#include <cstdlib>
#include <cstring>
#include <string>

#include "configurationmanager.hpp"


bool ConfigurationManager::Init()
{
    if(m_XMLDoc.LoadFile("./configuration.xml") == 0){ return true; }
    if(m_XMLDoc.LoadFile("./configure.xml"    ) == 0){ return true; }
    if(m_XMLDoc.LoadFile("./config.xml"       ) == 0){ return true; }
    if(m_XMLDoc.LoadFile("./conf.xml"         ) == 0){ return true; }
    return false;
}

void ConfigurationManager::Release()
{
}

// const char *ConfigurationManager::ExtractText(const char *path)
// {
//     // well-defined path should be: "root/basic/fullscreen/a/b/c/d/e"
//     if(path == nullptr){ return nullptr; }
//
//     const char *pStart = path;
//     while(*pStart == '/'){ pStart++; }
//     if(pStart[0] == '\0'){ return nullptr; }
//
//     const char *pEnd = nullptr;
//     const auto *pEle = m_XMLDoc.RootElement();
//
//     while(true){
//         // pStart should be valid here
//         // 1. not nullptr
//         // 2. pStart[0] != '\0'
//         pEnd = std::strchr(pStart, '/');
//         if(pEnd == nullptr){
//             pEle = pEle->FirstChildElement(pStart);
//             return (pEle ? pEle->GetText() : nullptr);
//         }else{
//             pEle = pEle->FirstChildElement(std::string(pStart, pEnd - pStart).c_str());
//             if(pEle == nullptr){ return nullptr; }
//             pStart = pEnd;
//             while(*pStart == '/'){ pStart++; }
//             if(pStart[0] == '\0'){ return pEle->GetText(); }
//         }
//     }
//     return nullptr;
// }

const char *ConfigurationManager::GetString(const char *path)
{
    auto p = GetXMLElement(path);
    m_OperationFailed = (p == nullptr);
    return p ? p->GetText() : nullptr;
}

int ConfigurationManager::GetInt(const char *path)
{
    auto p = GetXMLElement(path);
    m_OperationFailed = (p == nullptr);
    return p ? std::atoi(p->GetText()) : 0;
}

bool ConfigurationManager::GetBool(const char *path)
{
    auto p = GetXMLElement(path);
    m_OperationFailed = (p == nullptr);

    return false
        || p == nullptr
        || !std::strcmp(p->GetText(), "yes")
        || !std::strcmp(p->GetText(), "YES")
        || !std::strcmp(p->GetText(), "ok")
        || !std::strcmp(p->GetText(), "OK")
        || !std::strcmp(p->GetText(), "1")
        ;
}

bool ConfigurationManager::Error()
{
    // set as true when last error occurs
    return m_OperationFailed;
}

const tinyxml2::XMLElement *ConfigurationManager::GetXMLElement(const char *path)
{
    // Won't set m_OperationFailed, check return pointer to verify
    //
    // well-defined path should be: "root/basic/fullscreen/a/b/c/d/e"
    if(path == nullptr){ return nullptr; }

    //to the first valid letter
    const char *pStart = path;
    while(*pStart == '/'){ pStart++; }
    if(pStart[0] == '\0'){ return nullptr; }

    // skip Root on head if possible
    // we don't need to check std::strlen(pStart) here :)
    if(true
            && (pStart[0] == 'r' || pStart[0] == 'R')
            && (pStart[1] == 'o' || pStart[1] == 'O')
            && (pStart[2] == 'o' || pStart[2] == 'O')
            && (pStart[3] == 't' || pStart[3] == 'T')
      ){
        pStart += 4;
        while(*pStart == '/'){ pStart++; }
        if(pStart[0] == '\0'){ return nullptr; }
    }

    const char *pEnd = nullptr;
    const auto *pEle = m_XMLDoc.RootElement();

    while(true){
        // pStart should be valid here
        // 1. not nullptr
        // 2. pStart[0] != '\0'
        pEnd = std::strchr(pStart, '/');
        if(pEnd == nullptr){
            return pEle->FirstChildElement(pStart);
        }else{
            pEle = pEle->FirstChildElement(std::string(pStart, pEnd - pStart).c_str());
            if(pEle == nullptr){ return nullptr; }
            pStart = pEnd;
            while(*pStart == '/'){ pStart++; }
            if(pStart[0] == '\0'){ return pEle; }
        }
    }
    return nullptr;
}

ConfigurationManager *GetConfigurationManager()
{
    static ConfigurationManager configurationManager;
    return &configurationManager;
}
