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
#include <optional>
#include <stdexcept>
#include <tinyxml2.h>
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
        std::optional<std::vector<size_t>> m_UTF8CharOff;

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

            // if I support the following code
            // then I have to make m_UTF8CharOff as mutable
            //
            //    if(!m_UTF8CharOff.has_value()){
            //        m_UTF8CharOff.emplace(....)
            //    }
            //    return m_UTF8CharOff.value();
            //
            // this is too much
            // so always allocate m_UTF8CharOff in ctor and keep Type() const

            if(!m_UTF8CharOff.has_value()){
                throw std::runtime_error(str_fflprintf(": Utf8 token off doesn't initialized"));
            }
            return m_UTF8CharOff.value();
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

    public:
        static bool CheckXMLNodeAsLeaf(const tinyxml2::XMLNode *);
};
