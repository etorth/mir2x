/*
 * =====================================================================================
 *
 *       Filename: xmlroot.hpp
 *        Created: 06/17/2015 06:24:14
 *  Last Modified: 04/01/2016 16:36:36
 *
 *    Description: analyze specifically formatted XML
 *                      <ROOT>
 *                          <NODE>
 *                              ...
 *                          </NODE>
 *                      </ROOT>
 *                 start with root node <ROOT> and format as hierarchical text desc.
 *                 for XML with flat object list, use XMLObjectList
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
#include <tinyxml2.h>

class XMLRoot
{
    public:
        XMLRoot() = default;
       ~XMLRoot() = default;

    public:
       // throw exception when any error occurs
       int    NodeAtoi(const char *);
       bool   NodeAtob(const char *);
       double NodeAtof(const char *);

    public:
       // return false when any error occurs
       bool NodeAtoi(const char *,    int *,    int);
       bool NodeAtob(const char *,   bool *,   bool);
       bool NodeAtof(const char *, double *, double);

    public:
       const tinyxml2::XMLElement *GetXMLNode(const char *);

    public:
       //tiny API, maintains the validation by caller
       //
       bool Find(const char *szPath)
       {
           return GetXMLNode(szPath) != nullptr;
       }

       bool Load(const char *szFileName)
       {
           return szFileName && m_XMLDoc.LoadFile(szFileName) == tinyxml2::XML_NO_ERROR;
       }

       bool Parse(const char *szRawBuf)
       {
           return szRawBuf && m_XMLDoc.Parse(szRawBuf) == tinyxml2::XML_NO_ERROR;
       }

    protected:
       tinyxml2::XMLDocument m_XMLDoc;
};
