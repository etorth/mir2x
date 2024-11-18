#pragma once
#include <deque>
#include <tinyxml2.h>

#include "xmlf.hpp"
#include "strf.hpp"
#include "utf8f.hpp"
#include "fflerror.hpp"
#include "xmlparagraphleaf.hpp"

class XMLParagraph
{
    private:
        std::unique_ptr<tinyxml2::XMLDocument> m_xmlDocument; // leaf node refers to it

    private:
        std::deque<XMLParagraphLeaf> m_leafList;

    public:
        XMLParagraph(const char *xmlString = nullptr)
            : m_xmlDocument(std::make_unique<tinyxml2::XMLDocument>(true, tinyxml2::PEDANTIC_WHITESPACE))
        {
            loadXML(xmlString ? xmlString : "<par/>");
        }

    public:
        ~XMLParagraph() = default;

    public:
        bool empty() const
        {
            return leafCount() == 0;
        }

    public:
        int leafCount() const
        {
            return to_d(m_leafList.size());
        }

    public:
        bool leafValid(int leaf) const
        {
            return leaf >= 0 && leaf < leafCount();
        }

    public:
        const auto &leafRef(int leaf) const
        {
            if(!leafValid(leaf)){
                throw fflerror("invalid leaf index: %d", leaf);
            }
            return m_leafList[leaf];
        }

        auto &leafRef(int leaf)
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->leafRef(leaf));
        }

    public:
        bool leafOffValid(int leaf, int leafOff) const
        {
            if(!leafValid(leaf)){
                return false;
            }
            return leafOff >= 0 && leafOff < leafRef(leaf).length();
        }

    public:
        const auto &backLeafRef() const
        {
            if(m_leafList.empty()){
                throw fflerror("no leaf");
            }
            return m_leafList.back();
        }

        auto &backLeafRef()
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->backLeafRef());
        }

    public:
        XMLParagraph *split(int, int);

    public:
        void loadXML(const char *);
        void loadXMLNode(const tinyxml2::XMLNode *);

    public:
        void Join(const XMLParagraph &);

    private:
        size_t insertXMLAfter(tinyxml2::XMLNode *, const char *);

    public:
        size_t insertLeafXML(int, const char *);

    public:
        size_t insertUTF8String(int, int, const char *);

    public:
        tinyxml2::XMLNode *CloneLeaf(tinyxml2::XMLDocument *pDoc, int leaf) const
        {
            return leafRef(leaf).xmlNode()->DeepClone(pDoc);
        }

    public:
        const tinyxml2::XMLNode *getXMLNode() const
        {
            return m_xmlDocument->RootElement();
        }

    public:
        std::string getXML() const
        {
            tinyxml2::XMLPrinter printer;
            m_xmlDocument->Accept(&printer);

            std::string result = printer.CStr();
            while(result.ends_with('\n')){
                result.pop_back();
            }
            return result;
        }

    public:
        void deleteToken(int, int, int);
        void deleteToken(int, int);

    public:
        void deleteLeaf(int);
        void deleteUTF8Char(int, int, int);

    public:
        std::tuple<int, int, int> prevLeafOff(int, int, int) const;
        std::tuple<int, int, int> nextLeafOff(int, int, int) const;

    public:
        void clear()
        {
            loadXML("<par/>");
        }

    public:
        std::string getRawString() const;

    public:
        size_t tokenCount() const;
};
