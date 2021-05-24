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
#include "xmlroot.hpp"
#include "fflerror.hpp"

class XMLConf: public XMLRoot
{
    public:
        XMLConf(): XMLRoot()
        {
            if(load("conf.xml") || load("configure.xml") || load("configuration.xml")){
                return;
            }
            throw fflerror("not valid configuration file found");
        }
};
