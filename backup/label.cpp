#include <utf8.h>
#include "label.hpp"
#include "devicemanager.hpp"
#include "fonttexturemanager.hpp"

// every attribute that will change a lot, give a method: SetXXX()
// for merely change or static attribute, set it in constructor

Label::Label(const FONTINFO &stFontInfo, const SDL_Color &stColor, const char *szInfo)
    : Widget()
    , m_Content(szInfo)
{
    // TODO: when failed in loading, set to default font
    //       and failed again, set texture to "XXXXXXXXX"
    //
    m_CharBoxCache.FontInfo = stFontInfo;
    m_CharBoxCache.Color    = stColor;
	Compile();
}

Label::~Label()
{}

void Label::Compile()
{
	m_Line.clear();

    TOKENBOX stTokenBox;

    const char *pStart = m_Content.c_str();
    const char *pEnd   = pStart;

    while(*pEnd != '\0'){
        pStart = pEnd;
        utf8::unchecked::advance(pEnd, 1);
        std::memset(stTokenBox.UTF8CharBox.Data, 0, 8);
        if(pEnd - pStart == 1 && (*pStart == '\n' || *pStart == '\t' || *pStart == '\r')){
            // continue;
            stTokenBox.UTF8CharBox.Data[0] = ' ';
        }else{
            std::memcpy(stTokenBox.UTF8CharBox.Data, pStart, pEnd - pStart);
        }
        LoadUTF8CharBoxCache(stTokenBox);
        PushBack(stTokenBox);
    }
	SetTokenBoxStartX();
}

void Label::PushBack(TOKENBOX &stTokenBox)
{
    stTokenBox.State.W1 = 0;
    stTokenBox.State.W2 = 0;
    m_Line.push_back(stTokenBox);
}

void Label::SetTokenBoxStartX()
{
    int nCurrentX = 0;
    for(auto &stTokenBox: m_Line){
        nCurrentX += stTokenBox.State.W1;
        stTokenBox.Cache.StartX = nCurrentX;
        nCurrentX += stTokenBox.Cache.W;
        nCurrentX += stTokenBox.State.W2;
    }
    m_W = nCurrentX;
}

void Label::LoadUTF8CharBoxCache(TOKENBOX &stTokenBox)
{
    std::memcpy(m_CharBoxCache.Data, stTokenBox.UTF8CharBox.Data, 8);

    stTokenBox.UTF8CharBox.Cache.Texture[0] = GetFontTextureManager()->RetrieveTexture(m_CharBoxCache);
    SDL_QueryTexture(stTokenBox.UTF8CharBox.Cache.Texture[0],
            nullptr, nullptr, &(stTokenBox.Cache.W), &(stTokenBox.Cache.H));
    stTokenBox.Cache.H1     = stTokenBox.Cache.H;
    stTokenBox.Cache.H2     = 0;
    stTokenBox.Cache.StartY = 0;

    m_H = (std::max)(m_W, stTokenBox.Cache.H);
}

void Label::Draw()
{
    int nX = X();
    int nY = Y();
    for(auto &stTokenBox: m_Line){
        SDL_Rect stDstRect = {
            nX + stTokenBox.Cache.StartX,
            nY + stTokenBox.Cache.StartY,
            stTokenBox.Cache.W,
            stTokenBox.Cache.H
        };
        SDL_RenderCopy(
                GetDeviceManager()->GetRenderer(),
                stTokenBox.UTF8CharBox.Cache.Texture[0],
                nullptr, &stDstRect);
    }
}

const char *Label::Content()
{
    // never keep this pointer
    // C++ explicitly state that c_str()
    // should return the internal memory of std::string
    // keep it, you may change it, or when std::string got updated
    // this point is invalid, it's dangerous
    return m_Content.c_str();
}

void Label::SetContent(const char *szInfo)
{
    m_Content = szInfo;
    Compile();
}

void Label::Update(Uint32)
{}

bool Label::HandleEvent(SDL_Event &)
{
    return true;
}
