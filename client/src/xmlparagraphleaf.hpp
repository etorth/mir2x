#pragma once
#include <cctype>
#include <vector>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tinyxml2.h>
#include "strf.hpp"
#include "fflerror.hpp"

constexpr int LEAF_UTF8STR = 0;
constexpr int LEAF_IMAGE   = 1;
constexpr int LEAF_EMOJI   = 2;

class XMLParagraphLeaf
{
    private:
        friend class XMLParapragh;

    private:
        tinyxml2::XMLNode *m_node;

    private:
        int m_type;

    private:
        uint64_t m_u64Key;

    private:
        std::vector<int> m_utf8CharOff;
        std::optional<std::unordered_map<std::string, std::string>> m_attrListOpt;

    private:
        std::optional<uint32_t> m_fontColor;
        std::optional<uint32_t> m_fontBGColor;

    private:
        int m_event;

    public:
        explicit XMLParagraphLeaf(tinyxml2::XMLNode *);

    public:
        int type() const
        {
            return m_type;
        }

        const tinyxml2::XMLNode *xmlNode() const
        {
            return m_node;
        }

        tinyxml2::XMLNode *xmlNode()
        {
            return const_cast<tinyxml2::XMLNode *>(static_cast<const XMLParagraphLeaf *>(this)->xmlNode());
        }

        int length() const
        {
            if(type() == LEAF_UTF8STR){
                return to_d(utf8CharOffRef().size());
            }
            return 1;
        }

        const std::vector<int> &utf8CharOffRef() const
        {
            if(type() != LEAF_UTF8STR){
                throw fflerror("leaf is not an utf8 string");
            }

            if(m_utf8CharOff.empty()){
                throw fflerror("utf8 token off doesn't initialized");
            }

            return m_utf8CharOff;
        }

        std::vector<int> &utf8CharOffRef()
        {
            return const_cast<std::vector<int> &>(static_cast<const XMLParagraphLeaf *>(this)->utf8CharOffRef());
        }

        const char *UTF8Text() const
        {
            if(type() != LEAF_UTF8STR){
                return nullptr;
            }
            return xmlNode()->Value();
        }

        uint64_t ImageU64Key() const
        {
            if(type() != LEAF_IMAGE){
                throw fflerror("leaf is not an image");
            }
            return m_u64Key;
        }

        uint32_t emojiU32Key() const
        {
            if(type() != LEAF_EMOJI){
                throw fflerror("leaf is not an emoji");
            }
            return m_u64Key;
        }

        uint32_t peekUTF8Code(int) const;

    public:
        int markEvent(int);

    public:
        std::optional<bool> wrap() const;

    public:
        std::optional<uint32_t>   color() const;
        std::optional<uint32_t> bgColor() const;

    public:
        std::optional<uint8_t> font()      const;
        std::optional<uint8_t> fontSize()  const;
        std::optional<uint8_t> fontStyle() const;

    public:
        template<typename T> T *leafData() const
        {
            return reinterpret_cast<T *>(m_node->GetUserData());
        }

    public:
        const std::unordered_map<std::string, std::string> *hasEvent() const
        {
            return m_attrListOpt.has_value() ? std::addressof(m_attrListOpt.value()) : nullptr;
        }

    public:
        std::tuple<tinyxml2::XMLNode *, tinyxml2::XMLNode *> split(int, tinyxml2::XMLDocument &, tinyxml2::XMLDocument &);
};
