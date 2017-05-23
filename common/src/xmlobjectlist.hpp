/*
 * =====================================================================================
 *
 *       Filename: xmlobjectlist.hpp
 *        Created: 06/17/2015 06:24:14
 *  Last Modified: 05/20/2017 21:01:09
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
                    && m_XMLDoc.LoadFile(szFileName) == tinyxml2::XML_NO_ERROR){
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
                    && m_XMLDoc.Parse(szXMLContent) == tinyxml2::XML_NO_ERROR){
                bRes = bValidate ? Validate() : true;
            }

            Reset();
            return bRes;
        }

    private:
        bool ValidObjectNode(const tinyxml2::XMLElement *);
};
