/*
 * =====================================================================================
 *
 *       Filename: configurationmanager.hpp
 *        Created: 6/17/2015 6:24:14 PM
 *  Last Modified: 07/03/2015 8:07:31 PM
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

class ConfigurationManager
{
    private:
        ConfigurationManager() = default;

    private:
        tinyxml2::XMLDocument	m_XMLDoc;
        bool                    m_OperationFailed;

    public:
        bool Init();
        void Release();

    public:
        bool Error();

    private:
		const char *ExtractText(const char *path);

    public:
        const tinyxml2::XMLElement *GetXMLElement(const char *path);

    public:
        const char *GetString(const char *path);
        int         GetInt(const char *path);
        bool        GetBool(const char *path);

    public:
        friend ConfigurationManager *GetConfigurationManager();
};
