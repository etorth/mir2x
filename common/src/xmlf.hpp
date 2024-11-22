#pragma once
#include <vector>
#include <stdexcept>
#include <tinyxml2.h>

namespace xmlf
{
    bool checkNodeName(const tinyxml2::XMLNode *, const char *, bool /* exact */ = false);

    bool checkTextLeaf (const tinyxml2::XMLNode *);
    bool checkEmojiLeaf(const tinyxml2::XMLNode *);
    bool checkImageLeaf(const tinyxml2::XMLNode *);
    bool checkValidLeaf(const tinyxml2::XMLNode *);

    const char *findAttribute(const tinyxml2::XMLNode *, const char *, bool);

    bool hasChild(tinyxml2::XMLNode *, tinyxml2::XMLNode *);

    tinyxml2::XMLNode *getNextLeaf(tinyxml2::XMLNode *, tinyxml2::XMLNode * = nullptr);
    tinyxml2::XMLNode *getNodeFirstLeaf(tinyxml2::XMLNode *);
    tinyxml2::XMLNode *getNodeLastLeaf(tinyxml2::XMLNode *);

    bool validTagName(const std::string &);
    bool validAttributeName(const std::string &);

    std::string buildXMLString(
            const std::string &, // tag name
            const std::string &, // content
            const std::vector<std::pair<std::string, std::string>> &);

    // to support <, >, / in xml string
    // don't directly pass the raw string to addParXML
    std::string toParString(const char *, ...);
    std::string toString(const tinyxml2::XMLNode *);
}
