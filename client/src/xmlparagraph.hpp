/*
 * =====================================================================================
 *
 *       Filename: xmlparagraph.hpp
 *        Created: 12/11/2018 04:16:22
 *    Description:
 *
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

#include "xmlfunc.hpp"
#include "strfunc.hpp"
#include "utf8func.hpp"
#include "colorfunc.hpp"
#include "xmlparagraphleaf.hpp"

class XMLParagraph
{
    private:
        tinyxml2::XMLDocument m_XMLDocument;

    private:
        std::vector<XMLParagraphLeaf> m_leafList;

    public:
        XMLParagraph();

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
            return (int)(m_leafList.size());
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
                throw std::invalid_argument(str_fflprintf(": No leaf"));
            }
            return m_leafList.back();
        }

        auto &backLeafRef()
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->backLeafRef());
        }

    public:
        XMLParagraph Break();

    public:
        void loadXML(const char *);
        void loadXMLNode(const tinyxml2::XMLNode *);

    public:
        void Join(const XMLParagraph &);

    private:
        void insertXMLAfter(tinyxml2::XMLNode *, const char *);

    public:
        void insertLeafXML(int, const char *);

    public:
        void insertUTF8String(int, int, const char *);

    public:
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *pDoc)
        {
            return m_XMLDocument.RootElement()->DeepClone(pDoc);
        }

    public:
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *, int, int, int);

    public:
        tinyxml2::XMLNode *CloneLeaf(tinyxml2::XMLDocument *pDoc, int nLeaf) const
        {
            return leafRef(nLeaf).xmlNode()->DeepClone(pDoc);
        }

    public:
        std::string PrintXML() const
        {
            tinyxml2::XMLPrinter stPrinter;
            m_XMLDocument.Accept(&stPrinter);
            return std::string(stPrinter.CStr());
        }

    public:
        void deleteToken(int, int, int);
        void deleteToken(int, int);

    public:
        void deleteLeaf(int);
        void deleteUTF8Char(int, int, int);

    public:
        std::tuple<int, int, int> PrevLeafOff(int, int, int) const;
        std::tuple<int, int, int> NextLeafOff(int, int, int) const;

    private:
        const tinyxml2::XMLNode *leafCommonAncestor(int, int) const;

    private:
        void Clear()
        {
            m_leafList.clear();
            m_XMLDocument.Clear();
        }
};
