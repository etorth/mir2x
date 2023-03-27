#include <utf8.h>
#include <tinyxml2.h>

#include "log.hpp"
#include "xmlf.hpp"
#include "utf8f.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "xmlparagraphleaf.hpp"

extern Log *g_log;

XMLParagraphLeaf::XMLParagraphLeaf(tinyxml2::XMLNode *pNode)
    : m_node([pNode]()
      {
          if(pNode == nullptr){
              throw fflerror("invalid argument: (nullptr)");
          }

          if(!xmlf::checkValidLeaf(pNode)){
              throw fflerror("invalid argument: not a valid XMParagraph leaf");
          }
          return pNode;
      }())
    , m_type([this]()
      {
          if(xmlf::checkTextLeaf(xmlNode())){
              if(!utf8::is_valid(xmlNode()->Value(), xmlNode()->Value() + std::strlen(xmlNode()->Value()))){
                  throw fflerror("not a utf8 string: %s", xmlNode()->Value());
              }
              return LEAF_UTF8GROUP;
          }

          if(xmlf::checkEmojiLeaf(xmlNode())){
              return LEAF_EMOJI;
          }

          if(xmlf::checkImageLeaf(xmlNode())){
              return LEAF_IMAGE;
          }

          throw fflerror("invalid argument: node type not recognized");
      }())
    , m_u64Key([this]() -> uint64_t
      {
          switch(m_type){
              case LEAF_EMOJI:
                  {
                      int emojiId = 0;
                      if((m_node->ToElement()->QueryIntAttribute("id", &emojiId) != tinyxml2::XML_SUCCESS) &&
                         (m_node->ToElement()->QueryIntAttribute("Id", &emojiId) != tinyxml2::XML_SUCCESS) &&
                         (m_node->ToElement()->QueryIntAttribute("ID", &emojiId) != tinyxml2::XML_SUCCESS)){

                          // no id attribute
                          // use default emojiId zero
                      }
                      return emojiId;
                  }
              default:
                  {
                      return 0;
                  }
          }
      }())
    , m_event(BEVENT_OFF)
{
    if(type() == LEAF_UTF8GROUP){
        m_utf8CharOff = utf8f::buildUTF8Off(UTF8Text());
        if(auto par = m_node->Parent(); par && par->ToElement()){
            std::string tagName = par->ToElement()->Name();
            std::transform(tagName.begin(), tagName.end(), tagName.begin(), [](unsigned char c)
            {
                return std::tolower(c);
            });

            if(tagName == "event"){
                std::unordered_map<std::string, std::string> attrList;
                for(auto attrPtr = par->ToElement()->FirstAttribute(); attrPtr; attrPtr = attrPtr->Next()){
                    std::string attrName = attrPtr->Name();
                    std::transform(attrName.begin(), attrName.end(), attrName.begin(), [](unsigned char c)
                    {
                        return std::tolower(c);
                    });
                    attrList.emplace(std::move(attrName), attrPtr->Value());
                }
                m_attrListOpt = std::move(attrList);
            }
        }
    }

    // pre-save for color() and bgColor(), these two get called very frequently in drawEx()
    // colorf::string2RGBA() is expensive and shouldn't get called in tight loop

    if(const auto colorStr = xmlf::findAttribute(xmlNode(), "color", true)){
        m_fontColor = colorf::string2RGBA(colorStr);
    }

    if(const auto bgColorStr = xmlf::findAttribute(xmlNode(), "bgcolor", true)){
        m_fontBGColor = colorf::string2RGBA(bgColorStr);
    }
}

int XMLParagraphLeaf::markEvent(int event)
{
    switch(event){
        case BEVENT_ON:
        case BEVENT_OFF:
        case BEVENT_DOWN:
            {
                std::swap(m_event, event);
                return event;
            }
        default:
            {
                throw fflerror("invalid event: %d", event);
            }
    }
}

uint32_t XMLParagraphLeaf::peekUTF8Code(int leafOff) const
{
    if(leafOff >= length()){
        throw fflerror("provided LeafOff exceeds leaf length: %d", length());
    }

    if(type() != LEAF_UTF8GROUP){
        throw fflerror("try peek utf8 code from a leaf with type: %d", type());
    }

    return utf8f::peekUTF8Code(xmlNode()->Value() + utf8CharOffRef()[leafOff]);
}

std::optional<uint32_t> XMLParagraphLeaf::color() const
{
    if(m_fontColor.has_value()){
        return m_fontColor;
    }

    if(hasEvent()){
        switch(m_event){
            case BEVENT_ON  : return colorf::GREEN   + colorf::A_SHF(255);
            case BEVENT_OFF : return colorf::YELLOW  + colorf::A_SHF(255);
            case BEVENT_DOWN: return colorf::MAGENTA + colorf::A_SHF(255);
            default: throw fflerror("invalid leaf event: %d", m_event);
        }
    }
    return {};
}

std::optional<uint32_t> XMLParagraphLeaf::bgColor() const
{
    return m_fontBGColor;
}

std::optional<uint8_t> XMLParagraphLeaf::font() const
{
    if(const auto fontStr = xmlf::findAttribute(xmlNode(), "font", true)){
        if(const auto val = std::atoi(fontStr); val >= 0 && val < 256){
            return to_u8(val);
        }
        else{
            throw fflerror("invalid font index, not an uint8_t: %d", val);
        }
    }
    return {};
}

std::optional<uint8_t> XMLParagraphLeaf::fontSize() const
{
    if(const auto sizeStr = xmlf::findAttribute(xmlNode(), "size", true)){
        if(const auto val = std::atoi(sizeStr); val >= 0 && val < 256){
            return to_u8(val);
        }
        else{
            throw fflerror("invalid font size, not an uint8_t: %d", val);
        }
    }
    return {};
}

std::optional<uint8_t> XMLParagraphLeaf::fontStyle() const
{
    if(const auto fontStyleStr = xmlf::findAttribute(xmlNode(), "style", true)){
        if(const auto val = std::atoi(fontStyleStr); val >= 0 && val < 256){
            return to_u8(val);
        }
        else{
            throw fflerror("invalid font style, not an uint8_t: %d", val);
        }
    }
    return {};
}

std::optional<bool> XMLParagraphLeaf::wrap() const
{
    if(const auto wrapStr = xmlf::findAttribute(xmlNode(), "wrap", true)){
        return to_bool(wrapStr);
    }
    return {};
}
