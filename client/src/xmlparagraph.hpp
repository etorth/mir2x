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
#include <optional>
#include <stdexcept>
#include <tinyxml2.h>

#include "xmlfunc.hpp"
#include "strfunc.hpp"
#include "utf8func.hpp"
#include "colorfunc.hpp"

constexpr int LEAF_UTF8GROUP = 0;
constexpr int LEAF_IMAGE     = 1;
constexpr int LEAF_EMOJI     = 2;

class XMLParagraphLeaf
{
    private:
        friend class XMLParapragh;

    private:
        const tinyxml2::XMLNode *m_Node;

    private:
        const int m_Type;

    private:
        const uint64_t m_U64Key;

    private:
        std::vector<size_t> m_UTF8CharOff;

    private:
        std::optional<uint32_t> m_Color;
        std::optional<uint32_t> m_BGColor;

    private:
        int m_Event;

    public:
        XMLParagraphLeaf(tinyxml2::XMLNode *);

    public:
        int Type() const
        {
            return m_Type;
        }

        const tinyxml2::XMLNode *Node() const
        {
            return m_Node;
        }

        tinyxml2::XMLNode *Node()
        {
            return const_cast<tinyxml2::XMLNode *>(static_cast<const XMLParagraphLeaf *>(this)->Node());
        }

        size_t Length() const
        {
            if(Type() == LEAF_UTF8GROUP){
                return UTF8CharOffRef().size();
            }
            return 1;
        }

        const std::vector<size_t> &UTF8CharOffRef() const
        {
            if(Type() != LEAF_UTF8GROUP){
                throw std::runtime_error(str_fflprintf(": Leaf is not an utf8 string"));
            }

            if(m_UTF8CharOff.empty()){
                throw std::runtime_error(str_fflprintf(": Utf8 token off doesn't initialized"));
            }

            return m_UTF8CharOff;
        }

        std::vector<size_t> &UTF8CharOffRef()
        {
            return const_cast<std::vector<size_t> &>(static_cast<const XMLParagraphLeaf *>(this)->UTF8CharOffRef());
        }

        std::optional<uint32_t> Color() const
        {
            return m_Color;
        }

        std::optional<uint32_t> BGColor() const
        {
            return m_BGColor;
        }

        const char *UTF8Text() const
        {
            if(Type() != LEAF_UTF8GROUP){
                return nullptr;
            }
            return Node()->Value();
        }

        uint64_t ImageU64Key() const
        {
            if(Type() != LEAF_IMAGE){
                throw std::runtime_error(str_fflprintf(": Leaf is not an image"));
            }
            return m_U64Key;
        }

        uint64_t EmojiU64Key() const
        {
            if(Type() != LEAF_IMAGE){
                throw std::runtime_error(str_fflprintf(": Leaf is not an emoji"));
            }
            return m_U64Key;
        }

        uint32_t PeekUTF8Code(size_t) const;

    public:
        void MarkEvent(int);
};

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
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *pDoc)
        {
            return m_XMLDocument.RootElement()->DeepClone(pDoc);
        }

    public:
        tinyxml2::XMLNode *Clone(tinyxml2::XMLDocument *, size_t, size_t, size_t);

    public:
        tinyxml2::XMLNode *CloneLeaf(tinyxml2::XMLDocument *pDoc, size_t nLeaf) const
        {
            return LeafRef(nLeaf).Node()->DeepClone(pDoc);
        }

    public:
        std::string PrintXML() const
        {
            tinyxml2::XMLPrinter stPrinter;
            m_XMLDocument.Accept(&stPrinter);
            return std::string(stPrinter.CStr());
        }

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
        void Clear()
        {
            m_LeafList.clear();
            m_XMLDocument.Clear();
        }
};
