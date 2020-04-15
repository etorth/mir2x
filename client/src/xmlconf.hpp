/*
 * =====================================================================================
 *
 *       Filename: xmlconf.hpp
 *        Created: 03/16/2016 23:57:57
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
#include "log.hpp"
#include "xmlroot.hpp"

class XMLConf: public XMLRoot
{
    public:
        XMLConf()
            : XMLRoot()
        {
            extern XMLConf *g_XMLConf;
            if(g_XMLConf){
                extern Log *g_Log;
                g_Log->addLog(LOGTYPE_FATAL, "Multiple initialization for XMLConf");
            }

            if(false){
            }else if(!m_XMLDoc.LoadFile("./conf.xml"         )){
            }else if(!m_XMLDoc.LoadFile("./configure.xml"    )){
            }else if(!m_XMLDoc.LoadFile("./configuration.xml")){
            }else{
                extern Log *g_Log;
                g_Log->addLog(LOGTYPE_FATAL, "No configuration file found.");
                // TODO
                // if there is no configuration
                // we need to generate one with default value and remind user to edit it
                // then we can make this error as non-fatal
            }
        }

       ~XMLConf() = default;
};
