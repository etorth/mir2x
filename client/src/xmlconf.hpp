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
