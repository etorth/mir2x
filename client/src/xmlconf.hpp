/*
 * =====================================================================================
 *
 *       Filename: xmlconf.hpp
 *        Created: 03/16/2016 23:57:57
 *  Last Modified: 03/19/2016 03:46:28
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
#include "xmlext.hpp"

class XMLConf: public XMLExt
{
    public:
        XMLConf()
            : XMLExt()
        {
            extern XMLConf *g_XMLConf;
            if(g_XMLConf){
                throw std::runtime_error("one instance only for XMLConf please");
            }

            if(false){
            }else if(!m_XMLDoc.LoadFile("./conf.xml"         )){
            }else if(!m_XMLDoc.LoadFile("./configure.xml"    )){
            }else if(!m_XMLDoc.LoadFile("./configuration.xml")){
            }else{
                throw std::runtime_error("no configuration file find");
            }
        }

       ~XMLConf() = default;
};
