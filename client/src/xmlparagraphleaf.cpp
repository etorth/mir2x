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
#include "bevent.hpp"
#include "xmlfunc.hpp"
#include "utf8func.hpp"
#include "colorfunc.hpp"
#include "xmlparagraphleaf.hpp"

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
                  throw std::invalid_argument(str_fflprintf(": Not a utf8 string: %s", Node()->Value()));
              }
              return LEAF_UTF8GROUP;
          }

          if(XMLFunc::CheckEmojiLeaf(Node())){
              return LEAF_EMOJI;
          }

          if(XMLFunc::CheckImageLeaf(Node())){
              return LEAF_IMAGE;
          }

          throw std::invalid_argument(str_fflprintf(": Invalid argument: node type not recognized"));
      }())
    , m_U64Key(0)
    , m_UTF8CharOff()
    , m_Event(BEVENT_OFF)
{
    if(Type() == LEAF_UTF8GROUP){
        m_UTF8CharOff = UTF8Func::BuildUTF8Off(UTF8Text());
    }

    try{
        if(const auto *pszBGColor = XMLFunc::FindAttribute(Node(), "font_bgcolor", true)){
            m_BGColor.emplace(ColorFunc::String2RGBA(pszBGColor));
        }
    }catch(...){}
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
