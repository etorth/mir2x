/*
 * =====================================================================================
 *
 *       Filename: layoutboard.cpp
 *        Created: 03/25/2020 22:29:47
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

#include "fflerror.hpp"
#include "layoutboard.hpp"

void layoutBoard::loadXML(const char *xmlString)
{
    if(!xmlString){
        throw fflerror("null xmlString");
    }
    m_layout.loadXML(xmlString);

    m_w = m_layout.W();
    m_h = m_layout.H();
}

void layoutBoard::addParXML(int loc, const std::array<int, 4> &margin, const char *xmlString)
{
    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml failed: %s", xmlString ? xmlString : "(null)");
    }

    m_layout.addPar(loc, margin, xmlDoc.RootElement());
    m_w = m_layout.W();
    m_h = m_layout.H();
}
