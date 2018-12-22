/*
 * =====================================================================================
 *
 *       Filename: xmlparagraphleaf.hpp
 *        Created: 12/21/2018 02:43:29
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
#include "strfunc.hpp"
#include "colorfunc.hpp"

constexpr int LEAF_NONE      = 0;  // we allow invalid node in XMLDocument
constexpr int LEAF_UTF8GROUP = 1;
constexpr int LEAF_IMAGE     = 2;
constexpr int LEAF_EMOJI     = 4;

class XMLParagraphLeaf
{
    private:
        friend class XMLParapragh;

    private:
        int m_Type;

    private:
        const tinyxml2::XMLNode *m_Node;

    private:
        std::vector<size_t> m_UTF8CharOff;

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
            return m_UTF8CharOff;
        }

        std::vector<size_t> &UTF8CharOffRef()
        {
            return const_cast<std::vector<size_t> &>(static_cast<const XMLParagraphLeaf *>(this)->UTF8CharOffRef());
        }

        uint32_t BGColor() const
        {
            return ColorFunc::RED + 128;
        }

        const char *Text() const
        {
            if(Type() != LEAF_UTF8GROUP){
                return nullptr;
            }
            return Node()->Value();
        }

        uint64_t ImageU64Key() const
        {
            return 0;
        }

        uint64_t EmojiU64Key() const
        {
            return 0;
        }

        uint32_t PeekUTF8Code(size_t) const;

    public:
        void MarkEvent(int);

    public:
        static bool CheckXMLNodeAsLeaf(const tinyxml2::XMLNode *);
};
