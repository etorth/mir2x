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

    m_W = m_layout.W();
    m_H = m_layout.H();
}
