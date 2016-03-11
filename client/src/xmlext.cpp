/*
 * =====================================================================================
 *
 *       Filename: xmlext.cpp
 *        Created: 06/17/2015 06:25:24
 *  Last Modified: 03/10/2016 21:25:03
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

int XMLExt::NodeAtoi(const char *szPath)
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

    throw std::invalid_argument("invalid argument for XMLExt::NodeAtoi()");
}

double XMLExt::NodeAtof(const char *szPath)
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

    throw std::invalid_argument("invalid argument for XMLExt::NodeAtof()");
}

bool XMLExt::NodeAtob(const char *szPath)
{
    if(szPath){
        auto pstNode = GetXMLNode(szPath);
        if(pstNode){
            auto szContent = pstNode->GetText();
            if(szContent){
                if(false
                        || !std::strcmp(p->GetText(), "1")
                        || !std::strcmp(p->GetText(), "ok")
                        || !std::strcmp(p->GetText(), "Ok")
                        || !std::strcmp(p->GetText(), "OK")
                        || !std::strcmp(p->GetText(), "yes")
                        || !std::strcmp(p->GetText(), "Yes")
                        || !std::strcmp(p->GetText(), "YES")
                        || !std::strcmp(p->GetText(), "true")
                        || !std::strcmp(p->GetText(), "True")
                        || !std::strcmp(p->GetText(), "TRUE")){
                    return true;
                }

                if(false
                        || !std::strcmp(p->GetText(), "0")
                        || !std::strcmp(p->GetText(), "no")
                        || !std::strcmp(p->GetText(), "No")
                        || !std::strcmp(p->GetText(), "NO")
                        || !std::strcmp(p->GetText(), "false")
                        || !std::strcmp(p->GetText(), "False")
                        || !std::strcmp(p->GetText(), "FALSE")){
                    return false;
                }
                throw std::invalid_argument("invalid node text for boolen value conversion");
            }
        }
    }

    throw std::invalid_argument("invalid argument for XMLExt::NodeAtob()");
}

const tinyxml2::XMLElement *XMLExt::GetXMLNode(const char *szPath)
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
