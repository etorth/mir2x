/*
 * =====================================================================================
 *
 *       Filename: xmlparagraphleaf.hpp
 *        Created: 12/22/2018 07:38:04
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
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tinyxml2.h>
#include "strfunc.hpp"
#include "fflerror.hpp"

constexpr int LEAF_UTF8GROUP = 0;
constexpr int LEAF_IMAGE     = 1;
constexpr int LEAF_EMOJI     = 2;

struct xmlLeafData
{
    struct _currData
    {
    } currData;

    struct _accuData
    {
    } accuData;
};

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
                throw fflerror("leaf is not an utf8 string");
            }

            if(m_UTF8CharOff.empty()){
                throw fflerror("utf8 token off doesn't initialized");
            }

            return m_UTF8CharOff;
        }

        std::vector<size_t> &UTF8CharOffRef()
        {
            return const_cast<std::vector<size_t> &>(static_cast<const XMLParagraphLeaf *>(this)->UTF8CharOffRef());
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
                throw fflerror("leaf is not an image");
            }
            return m_U64Key;
        }

        uint32_t emojiU32Key() const
        {
            if(Type() != LEAF_EMOJI){
                throw fflerror("leaf is not an emoji");
            }
            return m_U64Key;
        }

        uint32_t PeekUTF8Code(size_t) const;

    public:
        void MarkEvent(int);

    public:
        std::optional<uint32_t>   Color() const;
        std::optional<uint32_t> BGColor() const;

    public:
        std::optional<uint8_t> Font()      const;
        std::optional<uint8_t> FontSize()  const;
        std::optional<uint8_t> FontStyle() const;

    public:
        const xmlLeafData *leafData() const
        {
            return reinterpret_cast<xmlLeafData *>(m_Node->GetUserData());
        }
};
