/*
 * =====================================================================================
 *
 *       Filename: xmlparagraphleaf.cpp
 *        Created: 12/22/2018 07:38:31
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

#include <utf8.h>
#include <tinyxml2.h>

#include "log.hpp"
#include "bevent.hpp"
#include "xmlfunc.hpp"
#include "utf8func.hpp"
#include "colorfunc.hpp"
#include "xmlparagraphleaf.hpp"

extern Log *g_Log;

XMLParagraphLeaf::XMLParagraphLeaf(tinyxml2::XMLNode *pNode)
    : m_Node([pNode]()
      {
          if(pNode == nullptr){
              throw std::invalid_argument(str_fflprintf(": Invalid argument: (nullptr)"));
          }

          if(!XMLFunc::CheckValidLeaf(pNode)){
              throw std::invalid_argument(str_fflprintf(": Invalid argument: not a valid XMParagraph leaf"));
          }
          return pNode;
      }())
    , m_Type([this]()
      {
          if(XMLFunc::CheckTextLeaf(Node())){
              if(!utf8::is_valid(Node()->Value(), Node()->Value() + std::strlen(Node()->Value()))){
                  throw fflerror("not a utf8 string: %s", Node()->Value());
              }
              return LEAF_UTF8GROUP;
          }

          if(XMLFunc::CheckEmojiLeaf(Node())){
              return LEAF_EMOJI;
          }

          if(XMLFunc::CheckImageLeaf(Node())){
              return LEAF_IMAGE;
          }

          throw fflerror("invalid argument: node type not recognized");
      }())
    , m_U64Key([this]() -> uint64_t
      {
          switch(m_Type){
              case LEAF_EMOJI:
                  {
                      return 0;
                  }
              default:
                  {
                      return 0;
                  }
          }
      }())
    , m_UTF8CharOff()
    , m_Event(BEVENT_OFF)
{
    if(Type() == LEAF_UTF8GROUP){
        m_UTF8CharOff = UTF8Func::BuildUTF8Off(UTF8Text());
    }
}

void XMLParagraphLeaf::MarkEvent(int nEvent)
{
    switch(nEvent){
        case BEVENT_ON:
        case BEVENT_OFF:
        case BEVENT_DOWN:
            {
                break;
            }
        default:
            {
                throw std::invalid_argument(str_fflprintf(": Invalid event: %d", nEvent));
            }
    }
    m_Event = nEvent;
}

uint32_t XMLParagraphLeaf::PeekUTF8Code(size_t nLeafOff) const
{
    if(nLeafOff >= Length()){
        throw std::invalid_argument(str_fflprintf(": Provided LeafOff exceeds leaf length: %zu", Length()));
    }

    if(Type() != LEAF_UTF8GROUP){
        throw std::runtime_error(str_fflprintf(": Try peek utf8 code from a leaf with type: %d", Type()));
    }

    return UTF8Func::PeekUTF8Code(Node()->Value() + UTF8CharOffRef()[nLeafOff]);
}

std::optional<uint32_t> XMLParagraphLeaf::Color() const
{
    if(const auto pszColor = XMLFunc::FindAttribute(Node(), "font_color", true)){
        try{
            return ColorFunc::String2RGBA(pszColor);
        }catch(...){}
    }
    return {};
}

std::optional<uint32_t> XMLParagraphLeaf::BGColor() const
{
    if(const auto pszBGColor = XMLFunc::FindAttribute(Node(), "font_bgcolor", true)){
        try{
            return ColorFunc::String2RGBA(pszBGColor);
        }catch(...){}
    }
    return {};
}

std::optional<uint8_t> XMLParagraphLeaf::Font() const
{
    if(const auto pszFont = XMLFunc::FindAttribute(Node(), "font", true)){
        try{
            if(auto nFontIndex = std::atoi(pszFont); (nFontIndex < 0 || nFontIndex > 255)){
                throw std::runtime_error(str_fflprintf(": Invalid font index, not an uint8_t: %d", nFontIndex));
            }else{
                return (uint8_t)(nFontIndex);
            }
        }catch(const std::exception &e){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught exception: %s", e.what());
        }catch(...){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught unknown exception");
        }
    }
    return {};
}

std::optional<uint8_t> XMLParagraphLeaf::FontSize() const
{
    if(const auto pszFontSize = XMLFunc::FindAttribute(Node(), "font_size", true)){
        try{
            if(auto nFontSize = std::atoi(pszFontSize); (nFontSize < 0 || nFontSize > 255)){
                throw std::runtime_error(str_fflprintf(": Invalid font size, not an uint8_t: %d", nFontSize));
            }else{
                return (uint8_t)(nFontSize);
            }
        }catch(const std::exception &e){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught exception: %s", e.what());
        }catch(...){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught unknown exception");
        }
    }
    return {};
}

std::optional<uint8_t> XMLParagraphLeaf::FontStyle() const
{
    if(const auto pszFontStyle = XMLFunc::FindAttribute(Node(), "font_style", true)){
        try{
            if(auto nFontStyle = std::atoi(pszFontStyle); (nFontStyle < 0 || nFontStyle > 255)){
                throw std::runtime_error(str_fflprintf(": Invalid font style, not an uint8_t: %d", nFontStyle));
            }else{
                return (uint8_t)(nFontStyle);
            }
        }catch(const std::exception &e){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught exception: %s", e.what());
        }catch(...){
            g_Log->AddLog(LOGTYPE_DEBUG, "Caught unknown exception");
        }
    }
    return {};
}
