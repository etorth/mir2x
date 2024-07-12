// analyze specifically formatted XML
//      <ROOT>
//          <NODE>
//              ...
//          </NODE>
//      </ROOT>
// start with root node <ROOT> and format as hierarchical text desc.

#pragma once
#include <optional>
#include <tinyxml2.h>

class XMLRoot
{
    private:
        tinyxml2::XMLDocument m_xmlDoc;

    public:
        /* ctor */  XMLRoot(): m_xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE) {}
        /* dtor */ ~XMLRoot() = default;

    public:
        // pase node
        // return {} is path is invalid
        // throw if path exists but failed to convert to expected type

        std::optional<int  > to_d   (const char *) const;
        std::optional<float> to_f   (const char *) const;
        std::optional<bool > to_bool(const char *) const;

    public:
        const tinyxml2::XMLElement *getXMLNode(const char *) const;

    public:
        bool has(const char *nodePath) const
        {
            return getXMLNode(nodePath) != nullptr;
        }

    public:
        bool load(const char *);
};
