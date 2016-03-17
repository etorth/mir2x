/*
 * =====================================================================================
 *
 *       Filename: xmlconf.hpp
 *        Created: 03/16/2016 23:57:57
 *  Last Modified: 03/17/2016 00:00:58
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
        }

       ~XMLConf() = default;
};
