/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.cpp
 *        Created: 06/17/2015 06:25:24
 *  Last Modified: 04/01/2016 21:21:17
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
#include "xmlobjectlist.hpp"
#include "log.hpp"

// assume XMLDocument is already loaded
bool XMLObjectList::Validate()
{
    const auto pRoot = Root();
    // 1. root is valid
    if(!pRoot){ return false; }

    // 2. root named as "Root"
    if(true
            && std::strcmp(pRoot->Name(), "ROOT")
            && std::strcmp(pRoot->Name(), "Root")
            && std::strcmp(pRoot->Name(), "root")){
        return false;
    }

    // 3. if <ROOT></ROOT> menas empty list, it's OK
    if(pRoot->NoChildren()){ return true; }

    // now it's a non-empty table
    // make sure every nodes are <OBJECT> nodes
    //
    // 4. compare all nodes
    auto pNode0 = pRoot->FirstChild();
    auto pNode1 = Fetch();

    while(true){
        // 1. both are null, return true
        if(!pNode0 && !pNode1){ return true; }
        // 2. both are non-empty and equal
        if(true
                && pNode0
                && pNode1
                && pNode0 == pNode1
                && ValidObjectNode((tinyxml2::XMLElement *)pNode0)){
            pNode0 = pNode0->NextSibling();
            pNode1 = Fetch();
            continue;
        }
        return false;
    }

    // make the compiler happy
    return false;
}

// we don't put too much logic in this function, since Reset() is only for Fetch(), no matter
// it's success or not, we just exam return of Fetch() ?= nullptr, and it's safe enough
//
void XMLObjectList::Reset()
{
    const auto pRoot = Root();
    if(pRoot){
        m_CurrentObject = nullptr;
        if(false){
        }else if((m_CurrentObject = pRoot->FirstChildElement("object"))){
        }else if((m_CurrentObject = pRoot->FirstChildElement("Object"))){
        }else if((m_CurrentObject = pRoot->FirstChildElement("OBJECT"))){
        }else{}
    }
}

const tinyxml2::XMLElement *XMLObjectList::Fetch()
{
    // 1. make a copy
    auto pOldObject = m_CurrentObject;

    // 2. move forward
    if(pOldObject){
        m_CurrentObject = nullptr;
        if(false){
        }else if((m_CurrentObject = pOldObject->NextSiblingElement("object"))){
        }else if((m_CurrentObject = pOldObject->NextSiblingElement("Object"))){
        }else if((m_CurrentObject = pOldObject->NextSiblingElement("OBJECT"))){
        }else{}
    }

    // 3. return the old element ptr
    return pOldObject;
}

bool XMLObjectList::ValidObjectNode(const tinyxml2::XMLElement *pElement)
{
    if(pElement){
        // 1. not as <OBJECT><b>bold</b></OBJECT>
        if(!pElement->GetText()){ return false; }
        // 2. not as <OBJECT>hello<b>bold</b></OBJECT>
        if(pElement->ToText() && pElement->ToText()->NoChildren()){ return true; }
    }
    return false;
}

void XMLObjectList::Add(const std::vector<
        std::pair<std::string, std::string>> & rstAttrV, const char *szContent)
{
    // 1. find last OBJECT
    if(!szContent || std::strlen(szContent) == 0){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "invalid node name");
        return;
    }

    auto *pElement = m_XMLDoc.NewElement(szContent);
    auto *pText    = m_XMLDoc.NewText(szContent);

    if(pElement && pText){
        pElement->InsertEndChild(pText);
        for(const auto &stInst: rstAttrV){
            pElement->SetAttribute(stInst.first.c_str(), stInst.second.c_str());
        }
        pElement->SetName("Object");

        auto pRoot = m_XMLDoc.RootElement();
        if(pRoot){
            pRoot->InsertEndChild(pElement);
        }
    }
}
