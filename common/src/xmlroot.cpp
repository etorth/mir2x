/*
 * =====================================================================================
 *
 *       Filename: xmlroot.cpp
 *        Created: 06/17/2015 06:25:24
 *  Last Modified: 07/18/2017 23:48:53
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
#include <stdexcept>
#include <system_error>
#include "xmlroot.hpp"

int XMLRoot::NodeAtoi(const char *szPath)
{
    if(szPath){
        auto pstNode = GetXMLNode(szPath);
        if(pstNode){
            auto szContent = pstNode->GetText();
            if(szContent){
                // TODO
                // here there would be a expense of std::string construction
                // this function is implemented by std::strtol so...
                return std::stoi(szContent);
            }
        }
    }

    throw std::error_code();
}

bool XMLRoot::NodeAtoi(const char *szPath, int *pRet, int nDefault)
{
    bool bRes = true;
    int  nRet = 0;
    try{
        nRet = NodeAtoi(szPath);
    }catch(...){
        bRes = false;
        nRet = nDefault;
    }
    if(pRet){ *pRet = nRet; }
    return bRes;
}

double XMLRoot::NodeAtof(const char *szPath)
{
    if(szPath){
        auto pstNode = GetXMLNode(szPath);
        if(pstNode){
            auto szContent = pstNode->GetText();
            if(szContent){
                // TODO
                // here there would be a expense of std::string construction
                // this function is implemented by std::strtod so...
                return std::stod(szContent);
            }
        }
    }

    throw std::error_code();
}

bool XMLRoot::NodeAtof(const char *szPath, double *pRet, double fDefault)
{
    bool   bRes = true;
    double fRet = 0.0;
    try{
        fRet = NodeAtof(szPath);
    }catch(...){
        bRes = false;
        fRet = fDefault;
    }
    if(pRet){ *pRet = fRet; }
    return bRes;
}

bool XMLRoot::NodeAtob(const char *szPath)
{
    if(szPath){
        auto pstNode = GetXMLNode(szPath);
        if(pstNode){
            auto szContent = pstNode->GetText();
            if(szContent){
                if(false
                        || !std::strcmp(szContent, "1")
                        || !std::strcmp(szContent, "ok")
                        || !std::strcmp(szContent, "Ok")
                        || !std::strcmp(szContent, "OK")
                        || !std::strcmp(szContent, "yes")
                        || !std::strcmp(szContent, "Yes")
                        || !std::strcmp(szContent, "YES")
                        || !std::strcmp(szContent, "true")
                        || !std::strcmp(szContent, "True")
                        || !std::strcmp(szContent, "TRUE")){
                    return true;
                }

                if(false
                        || !std::strcmp(szContent, "0")
                        || !std::strcmp(szContent, "no")
                        || !std::strcmp(szContent, "No")
                        || !std::strcmp(szContent, "NO")
                        || !std::strcmp(szContent, "false")
                        || !std::strcmp(szContent, "False")
                        || !std::strcmp(szContent, "FALSE")){
                    return false;
                }
            }
        }
    }

    throw std::error_code();
}

bool XMLRoot::NodeAtob(const char *szPath, bool *pRet, bool bDefault)
{
    bool   bRes = true;
    double bRet = false;
    try{
        bRet = NodeAtob(szPath);
    }catch(...){
        bRes = false;
        bRet = bDefault;
    }
    if(pRet){ *pRet = bRet; }
    return bRes;
}

const tinyxml2::XMLElement *XMLRoot::GetXMLNode(const char *szPath) const
{
    // well-defined path should be:
    // "root/basic/fullscreen/a/b/c/d/e"
    //
    if(szPath == nullptr){ return nullptr; }

    //to the first valid letter
    const char *pStart = szPath;
    while(*pStart == '/'){ pStart++; }
    if(pStart[0] == '\0'){ return nullptr; }

    // skip Root on head if possible
    // we don't need to check std::strlen(pStart) here :)
    if(true
            && (pStart[0] == 'r' || pStart[0] == 'R')
            && (pStart[1] == 'o' || pStart[1] == 'O')
            && (pStart[2] == 'o' || pStart[2] == 'O')
            && (pStart[3] == 't' || pStart[3] == 'T')){
        // ok, skip the "Root//////.."
        pStart += 4;
        while(*pStart == '/'){ pStart++; }
        if(pStart[0] == '\0'){ return nullptr; }
    }

    const char *pEnd    = nullptr;
    const auto *pstNode = m_XMLDoc.RootElement();

    while(true){
        // pStart should be valid here
        // 1. not nullptr
        // 2. pStart[0] != '\0'
        pEnd = std::strchr(pStart, '/');
        if(pEnd == nullptr){
            // to the very end, this is what we need
            return pstNode->FirstChildElement(pStart);
        }else{
            // continue
            pstNode = pstNode->FirstChildElement(
                    std::string(pStart, pEnd - pStart).c_str());

            // detected wrong path when parsing
            if(pstNode == nullptr){ return nullptr; }

            // get rid of all '//////'
            // this also takes care of "root/a/b/" which ends with '/'
            //
            pStart = pEnd;
            while(*pStart == '/'){ pStart++; }

            if(pStart[0] == '\0'){ return pstNode; }
        }
    }

    // make the compiler happy
    return nullptr;
}
