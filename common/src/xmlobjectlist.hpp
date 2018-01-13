/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.hpp
 *        Created: 06/17/2015 06:24:14
 *  Last Modified: 01/12/2018 18:29:47
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

    private:
        const tinyxml2::XMLElement *m_CurrentObject;

    public:
        XMLObjectList()
            : m_XMLDoc()
            , m_CurrentObject(nullptr)
        {}

       ~XMLObjectList() = default;

    public:
        const tinyxml2::XMLElement *Root()
        {
            return m_XMLDoc.RootElement();
        }

        void Reset();
        const tinyxml2::XMLElement *Fetch();

    public:
        bool Validate();

    public:
        std::string Print();
        bool Add(const std::vector<std::pair<std::string, std::string>> &, const char *);

    public:
        bool Load(const char *szFileName, bool bValidate = true)
        {
            bool bRes = false;
            if(true
                    && szFileName
                    && m_XMLDoc.LoadFile(szFileName) == tinyxml2::XML_SUCCESS){
                bRes = bValidate ? Validate() : true;
            }

            Reset();
            return bRes;
        }

        bool Parse(const char *szXMLContent, bool bValidate = true)
        {
            bool bRes = false;
            if(true
                    && szXMLContent
                    && m_XMLDoc.Parse(szXMLContent) == tinyxml2::XML_SUCCESS){
                bRes = bValidate ? Validate() : true;
            }

            Reset();
            return bRes;
        }

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
