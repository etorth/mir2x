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

#include "xmlfunc.hpp"
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
        bool Empty() const
        {
            return LeafCount() == 0;
        }

    public:
        size_t LeafCount() const
        {
            return m_LeafList.size();
        }

    public:
        bool LeafValid(size_t nLeaf) const
        {
            return nLeaf < LeafCount();
        }

    public:
        const auto &LeafRef(size_t nLeaf) const
        {
            if(!LeafValid(nLeaf)){
                throw std::invalid_argument(str_fflprintf(": Invlaid leaf index: %zu", nLeaf));
            }
            return m_LeafList[nLeaf];
        }

        auto &LeafRef(size_t nLeaf)
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->LeafRef(nLeaf));
        }

    public:
        const auto &BackLeafRef() const
        {
            if(m_LeafList.empty()){
                throw std::invalid_argument(str_fflprintf(": No leaf"));
            }
            return m_LeafList.back();
        }

        auto &BackLeafRef()
        {
            return const_cast<XMLParagraphLeaf &>(static_cast<const XMLParagraph *>(this)->BackLeafRef());
        }

    public:
        XMLParagraph Break();

    public:
        void LoadXML(const char *);

    public:
        void Join(const XMLParagraph &);

    public:
        void InsertNode();
        void InsertLeaf();

    public:
        void InsertUTF8Char(size_t, size_t, const char *);

    public:
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *);
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *, size_t, size_t, size_t);

    public:
        tinyxml2::XMLNode *CloneLeaf(tinyxml2::XMLDocument *pDoc, size_t nLeaf) const
        {
            return LeafRef(nLeaf).Node()->DeepClone(pDoc);
        }

    public:
        std::string PrintXML() const;

    public:
        void Delete(size_t, size_t, size_t);

    public:
        void DeleteLeaf(size_t);
        void DeleteUTF8Char(size_t, size_t, size_t);

    public:
        std::tuple<size_t, size_t, size_t> PrevLeafOff(size_t, size_t, size_t) const;
        std::tuple<size_t, size_t, size_t> NextLeafOff(size_t, size_t, size_t) const;

    private:
        const tinyxml2::XMLNode *LeafCommonAncestor(size_t, size_t) const;

    private:
        void Clear();
};
