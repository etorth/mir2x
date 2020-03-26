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
        std::vector<XMLParagraphLeaf> m_LeafList;

    public:
        XMLParagraph();

    public:
        ~XMLParagraph() = default;

    public:
        bool empty() const
        {
            return LeafCount() == 0;
        }

    public:
        int LeafCount() const
        {
            return (int)(m_LeafList.size());
        }

    public:
        bool LeafValid(int nLeaf) const
        {
            return nLeaf >= 0 && nLeaf < LeafCount();
        }

    public:
        const auto &leafRef(int nLeaf) const
        {
            if(!LeafValid(nLeaf)){
                throw std::invalid_argument(str_fflprintf(": Invalid leaf index: %zu", nLeaf));
            }
            return m_LeafList[nLeaf];
        }

        auto &leafRef(int nLeaf)
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->leafRef(nLeaf));
        }

    public:
        const auto &BackleafRef() const
        {
            if(m_LeafList.empty()){
                throw std::invalid_argument(str_fflprintf(": No leaf"));
            }
            return m_LeafList.back();
        }

        auto &BackleafRef()
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->BackleafRef());
        }

    public:
        XMLParagraph Break();

    public:
        void loadXML(const char *);
        void loadXMLNode(const tinyxml2::XMLNode *);

    public:
        void Join(const XMLParagraph &);

    public:
        void InsertNode();
        void InsertLeaf();

    public:
        void InsertUTF8Char(int, int, const char *);

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
            return leafRef(nLeaf).Node()->DeepClone(pDoc);
        }

    public:
        std::string PrintXML() const
        {
            tinyxml2::XMLPrinter stPrinter;
            m_XMLDocument.Accept(&stPrinter);
            return std::string(stPrinter.CStr());
        }

    public:
        void Delete(int, int, int);

    public:
        void DeleteLeaf(int);
        void DeleteUTF8Char(int, int, int);

    public:
        std::tuple<int, int, int> PrevLeafOff(int, int, int) const;
        std::tuple<int, int, int> NextLeafOff(int, int, int) const;

    private:
        const tinyxml2::XMLNode *LeafCommonAncestor(int, int) const;

    private:
        void Clear()
        {
            m_LeafList.clear();
            m_XMLDocument.Clear();
        }
};
