/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.hpp
 *        Created: 06/17/2015 06:24:14
 *  Last Modified: 01/19/2018 22:57:59
 *
 *    Description: analyze specifically formatted XML
 *                      <ROOT>
 *                          <OBJECT TYPE="T0"></OBJECT>
 *                          <OBJECT TYPE="T1"></OBJECT>
 *                          <OBJECT TYPE="T2"></OBJECT>
 *                          <OBJECT TYPE="T3"></OBJECT>
 *                      </ROOT>
 *                 start with root node <ROOT> and format as flat object list,
 *                 for hierarchical text desc use XMLRoot
 *
 *                 keep a internal vector for all objects at loading
 *                 and won't use Fetch() anymore
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
#include <tinyxml2.h>

class XMLObjectList final
{
    private:
        tinyxml2::XMLDocument m_XMLDoc;

    public:
        XMLObjectList()
            : m_XMLDoc()
        {}

    public:
        ~XMLObjectList() = default;

    public:
        tinyxml2::XMLElement *Root()
        {
            return m_XMLDoc.RootElement();
        }

        const tinyxml2::XMLElement *Root() const
        {
            return m_XMLDoc.RootElement();
        }

    public:
        tinyxml2::XMLElement *FirstElement()
        {
            if(auto pRoot = Root()){
                return pRoot->FirstChildElement();
            }
            return nullptr;
        }

        const tinyxml2::XMLElement *FirstElement() const
        {
            if(auto pRoot = Root()){
                return pRoot->FirstChildElement();
            }
            return nullptr;
        }

    public:
        std::string Print() const
        {
            tinyxml2::XMLPrinter stPrinter;
            m_XMLDoc.Print(&stPrinter);
            return std::string(stPrinter.CStr());
        }

    public:
        bool Add(const std::vector<std::pair<std::string, std::string>> &, const char *);

    public:
        bool Load(const char *szFileName, bool bValidate = true)
        {
            bool bRes = false;
            if(true
                    && szFileName
                    && m_XMLDoc.LoadFile(szFileName) == tinyxml2::XML_SUCCESS){
                bRes = bValidate ? ValidateXML() : true;
            }
            return bRes;
        }

        bool Parse(const char *szXMLContent, bool bValidate = true)
        {
            bool bRes = false;
            if(true
                    && szXMLContent
                    && m_XMLDoc.Parse(szXMLContent) == tinyxml2::XML_SUCCESS){
                bRes = bValidate ? ValidateXML() : true;
            }
            return bRes;
        }

    private:
        bool ValidateXML();

    private:
        bool ValidObjectNode(const tinyxml2::XMLElement *);
};

enum XMLObjectType: int
{
    OBJECTTYPE_NONE      = 0,
    OBJECTTYPE_RETURN    = 1,
    OBJECTTYPE_PLAINTEXT = 2,
    OBJECTTYPE_EVENTTEXT = 3,
    OBJECTTYPE_EMOTICON  = 4,
};

namespace XMLObject
{
    // move this part out of XMLObjectList
    // XMLObjectList won't take care of specific object types
    int ObjectType(const tinyxml2::XMLElement &);
}
