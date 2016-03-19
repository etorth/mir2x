/*
 * =====================================================================================
 *
 *       Filename: xmlext.hpp
 *        Created: 06/17/2015 06:24:14
 *  Last Modified: 03/19/2016 03:46:59
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


#pragma once
#include <tinyxml2.h>

class XMLExt
{
    public:
        XMLExt() = default;
       ~XMLExt() = default;

    public:
       int    NodeAtoi(const char *);
       double NodeAtof(const char *);
       bool   NodeAtob(const char *);

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
