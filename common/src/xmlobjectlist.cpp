/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.cpp
 *        Created: 06/17/2015 06:25:24
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

bool XMLObjectList::ValidateXML()
{
    // check if current loading XML is a object list
    // reject if no "ROOT" or has hierarchical structure

    if(const auto pRoot = Root()){
        if(true
                && std::strcmp(pRoot->Name(), "ROOT")
                && std::strcmp(pRoot->Name(), "Root")
                && std::strcmp(pRoot->Name(), "root")){
            return false;
        }

        // now it's a non-empty table
        // make sure every nodes are <OBJECT> nodes

        for(auto pCurrObject = FirstElement(); pCurrObject; pCurrObject = pCurrObject->NextSiblingElement()){
            if(!ValidObjectNode(pCurrObject)){
                return false;
            }
        }
        return true;
    }
    return false;
}

bool XMLObjectList::Add(const std::vector<std::pair<std::string, std::string>> &rstAttrList, const char *szContent)
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
            && (( rstAttrList.empty()))
            && ((!szContent || !std::strlen(szContent)))){
        return false;
    }

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
        for(const auto &stInst: rstAttrList){
            pObject->SetAttribute(stInst.first.c_str(), stInst.second.c_str());
        }

        // 2. insert the new object to the root
        if(auto pRoot = Root()){
            return pRoot->InsertEndChild(pObject);
        }else{
            pRoot = m_XMLDoc.NewElement("Root");
            if(true
                    && pRoot
                    && m_XMLDoc.InsertEndChild(pRoot)){
                pRoot = Root();
                return true
                    && pRoot
                    && pRoot->InsertEndChild(pObject);
            }
        }
    }

    // even if the insertion failed
    // don't need to free the memory since m_XMLDoc will take care of it

    return false;
}

bool XMLObjectList::ValidObjectNode(const tinyxml2::XMLElement *pElement)
{
    // 1. should named as "OBJECT"
    // 2. should have no child or one text child

    if(pElement){
        if(true
                && std::strcmp(pElement->Name(), "OBJECT")
                && std::strcmp(pElement->Name(), "Object")
                && std::strcmp(pElement->Name(), "object")){
            return false;
        }

        if(pElement->NoChildren()){
            return true;
        }

        if(!pElement->GetText()){
            return false;
        }

        auto pText = pElement->ToText();
        if(true
                &&  pText
                && !pText->NoChildren()){
            return false;
        }

        return true;
    }
    return false;
}

int XMLObject::ObjectType(const tinyxml2::XMLElement &rstObject)
{
    if(true
            && (rstObject.Attribute("Type") == nullptr)
            && (rstObject.Attribute("TYPE") == nullptr)
            && (rstObject.Attribute("type") == nullptr)){
        return OBJECTTYPE_PLAINTEXT;
    }

    if(false
            || rstObject.Attribute("TYPE", "PLAINTEXT")
            || rstObject.Attribute("TYPE", "PlainText")
            || rstObject.Attribute("TYPE", "Plaintext")
            || rstObject.Attribute("TYPE", "plainText")
            || rstObject.Attribute("TYPE", "plaintext")
            || rstObject.Attribute("Type", "PLAINTEXT")
            || rstObject.Attribute("Type", "PlainText")
            || rstObject.Attribute("Type", "Plaintext")
            || rstObject.Attribute("Type", "plainText")
            || rstObject.Attribute("Type", "plaintext")
            || rstObject.Attribute("type", "PLAINTEXT")
            || rstObject.Attribute("type", "PlainText")
            || rstObject.Attribute("type", "Plaintext")
            || rstObject.Attribute("type", "plainText")
            || rstObject.Attribute("type", "plaintext")){
        return OBJECTTYPE_PLAINTEXT;
    }

    if(false
            || rstObject.Attribute("TYPE", "EVENTTEXT")
            || rstObject.Attribute("TYPE", "EventText")
            || rstObject.Attribute("TYPE", "Eventtext")
            || rstObject.Attribute("TYPE", "eventText")
            || rstObject.Attribute("TYPE", "eventtext")
            || rstObject.Attribute("Type", "EVENTTEXT")
            || rstObject.Attribute("Type", "EventText")
            || rstObject.Attribute("Type", "Eventtext")
            || rstObject.Attribute("Type", "eventText")
            || rstObject.Attribute("Type", "eventtext")
            || rstObject.Attribute("type", "EVENTTEXT")
            || rstObject.Attribute("type", "EventText")
            || rstObject.Attribute("type", "Eventtext")
            || rstObject.Attribute("type", "eventText")
            || rstObject.Attribute("type", "eventtext")){
        return OBJECTTYPE_EVENTTEXT;
    }

    if(false
            || rstObject.Attribute("TYPE", "RETURN")
            || rstObject.Attribute("TYPE", "Return")
            || rstObject.Attribute("TYPE", "return")
            || rstObject.Attribute("Type", "RETURN")
            || rstObject.Attribute("Type", "Return")
            || rstObject.Attribute("Type", "return")
            || rstObject.Attribute("type", "RETURN")
            || rstObject.Attribute("type", "Return")
            || rstObject.Attribute("type", "return")){
        return OBJECTTYPE_RETURN;
    }

    if(false
            || rstObject.Attribute("TYPE", "Emoticon")
            || rstObject.Attribute("TYPE", "emoticon")
            || rstObject.Attribute("TYPE", "EMOTICON")
            || rstObject.Attribute("Type", "Emoticon")
            || rstObject.Attribute("Type", "emoticon")
            || rstObject.Attribute("Type", "EMOTICON")
            || rstObject.Attribute("type", "Emoticon")
            || rstObject.Attribute("type", "emoticon")
            || rstObject.Attribute("type", "EMOTICON")){
        return OBJECTTYPE_EMOTICON;
    }

    return OBJECTTYPE_NONE;
}
