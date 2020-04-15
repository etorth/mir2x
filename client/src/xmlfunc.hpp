/*
 * =====================================================================================
 *
 *       Filename: xmlfunc.hpp
 *        Created: 12/11/2018 04:01:50
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
#include <vector>
#include <stdexcept>
#include <tinyxml2.h>
#include "strf.hpp"

namespace XMLFunc
{
    bool CheckTextLeaf (const tinyxml2::XMLNode *);
    bool CheckEmojiLeaf(const tinyxml2::XMLNode *);
    bool CheckImageLeaf(const tinyxml2::XMLNode *);
    bool CheckValidLeaf(const tinyxml2::XMLNode *);

    const char *FindAttribute(const tinyxml2::XMLNode *, const char *, bool);

    // 1. current input should be a xml leaf
    // 2. return next xml node if exists, or nullptr
    tinyxml2::XMLNode *GetNextLeaf(tinyxml2::XMLNode *);

    tinyxml2::XMLNode *GetNodeFirstLeaf(tinyxml2::XMLNode *);
    tinyxml2::XMLNode *GetTreeFirstLeaf(tinyxml2::XMLNode *);

    tinyxml2::XMLNode *GetNodeLastLeaf(tinyxml2::XMLNode *);
    tinyxml2::XMLNode *GetTreeLastLeaf(tinyxml2::XMLNode *);

    bool ValidTagName(const std::string &);
    bool ValidAttributeName(const std::string &);

    std::string BuildXMLString(
            const std::string &, // tag name
            const std::string &, // content
            const std::vector<std::pair<std::string, std::string>> &);
}
