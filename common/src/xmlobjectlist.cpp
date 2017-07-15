/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.cpp
 *        Created: 06/17/2015 06:25:24
 *  Last Modified: 07/14/2017 21:21:20
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
#include <string>
#include <cstring>
#include <cstdlib>
#include "xmlobjectlist.hpp"

bool XMLObjectList::Validate()
{
    Reset();

    // 1. should have a root
    if(const auto pRoot = Root()){
        // 2. should named as "root"
        if(true
                && std::strcmp(pRoot->Name(), "ROOT")
                && std::strcmp(pRoot->Name(), "Root")
                && std::strcmp(pRoot->Name(), "root")){ return false; }

        // 3. content check
        //    could be an empty list or flat list
        if(pRoot->NoChildren()){ return true; }

        // now it's a non-empty table
        // make sure every nodes are <OBJECT> nodes

        auto pNode0 = pRoot->FirstChild();
        auto pNode1 = Fetch();

        while(true){
            // 1. both are null
            //    return true since we reach the end
            if(!pNode0 && !pNode1){ return true; }
            // 2. both are non-empty and equal
            //    to make sure all elements are objects
            //    one by NextSibling() and the other by NextSiblingElement("OBJECT")
            if(true
                    && pNode0
                    && pNode1
                    && pNode0 == pNode1
                    && ValidObjectNode((tinyxml2::XMLElement *)(pNode0))){
                pNode0 = pNode0->NextSibling();
                pNode1 = Fetch();
                continue;
            }
            return false;
        }
    }
    return false;
}

void XMLObjectList::Reset()
{
    // used before calling Fetch()
    // reset the current object to be the first object in the list
    // if empty set m_CurrentObject as empty

    if(const auto pRoot = Root()){
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
    //    call it after Reset() or Fetch()
    auto pLastObject = m_CurrentObject;

    // 2. move forward
    if(pLastObject){
        m_CurrentObject = nullptr;
        if(false){
        }else if((m_CurrentObject = pLastObject->NextSiblingElement("object"))){
        }else if((m_CurrentObject = pLastObject->NextSiblingElement("Object"))){
        }else if((m_CurrentObject = pLastObject->NextSiblingElement("OBJECT"))){
        }else{}
    }

    // 3. return the old element ptr
    return pLastObject;
}

bool XMLObjectList::Add(const std::vector<std::pair<std::string, std::string>> &rstAttrV, const char *szContent)
{
    // possible input for 
    // 1. <OBJECT TYPE="PLAINTEXT">Text</OBJECT>
    // 2. <OBJECT TYPE="EVENTTEXT">Text</OBJECT>
    //
    // 3. <OBJECT TYPE="EMOTICON"/>
    // 4. <OBJECT TYPE="EMOTICON"></OBJECT>

    // reject to insert empty object as
    // 1. <OBJECT/>
    // 2. <OBJECT></OBJECT>

    if(true
            && (( rstAttrV.empty()))
            && ((!szContent || !std::strlen(szContent)))){ return false; }

    // TODO
    // currently I design this class for unverisal usage
    // should I put additional check here to only allow attributes like
    //
    // <OBJECT TYPE = "RETURN"   ></OBJECT>
    // <OBJECT TYPE = "EMOTICON" ></OBJECT>
    // <OBJECT TYPE = "PLAINTEXT"></OBJECT>
    // <OBJECT TYPE = "EVENTTEXT"></OBJECT>

    auto *pObject        = m_XMLDoc.NewElement("Object");
    auto *pObjectContent = m_XMLDoc.NewText(szContent ? szContent : "");

    if(true
            && pObject
            && pObjectContent
            && pObject->InsertEndChild(pObjectContent)){

        // 1. setup all attributes for this object
        for(const auto &stInst: rstAttrV){
            pObject->SetAttribute(stInst.first.c_str(), stInst.second.c_str());
        }

        // 2. insert the new object to the root
        auto pRoot = m_XMLDoc.RootElement();
        if(pRoot){
            return pRoot->InsertEndChild(pObject);
        }else{
            pRoot = m_XMLDoc.NewElement("Root");
            if(true
                    && pRoot
                    && m_XMLDoc.InsertEndChild(pRoot)){
                pRoot = m_XMLDoc.RootElement();
                return true
                    && pRoot
                    && pRoot->InsertEndChild(pObject);
            }
        }
    }

    // I won't delete the memory allocated if failed
    // free all of them during class destruction
    return false;
}

std::string XMLObjectList::Print()
{
    if(Validate()){
        tinyxml2::XMLPrinter stPrinter;
        m_XMLDoc.Print(&stPrinter);
        return std::string(stPrinter.CStr());
    }

    // TODO
    // solution-1. return an empty string
    // solution-2. return an empty object list
    return std::string("<Root></Root>"); 
}

bool XMLObjectList::ValidObjectNode(const tinyxml2::XMLElement *pElement)
{
    // check each object node, requirements
    // 1. could be empty, like
    //      <object type="return"  ></object>
    //      <object type="emoticon"></object>
    // 2. shouldn't be hierarchical, following are invalid examples
    //      <object><b>bold</b></object>
    //      <object>hello<b>bold</b></object>

    if(pElement){
        if(pElement->NoChildren()){ return true; }
        if(!pElement->GetText()){ return false; }
        if(pElement->ToText() && !pElement->ToText()->NoChildren()){ return false; }

        return true;
    }

    return false;
}
