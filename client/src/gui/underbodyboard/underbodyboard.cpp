#include "strf.hpp"
#include "hero.hpp"
#include "underbodyboard.hpp"

UnderBodyBoard::UnderBodyBoard(UnderBodyBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::nullopt,
          .h = std::nullopt,

          .parent = std::move(args.parent),
      }}

    , m_hero(fflcheck(args.hero))
    , m_name
      {{
          .textFunc = [this] -> std::string
          {
              return m_hero->getName();
          },

          .font
          {
              .color = [this]{ return m_hero->getNameColor(); },
          },

          .attrs
          {
              .show = [this]{ return !m_name.empty(); },
          },
      }}

    , m_guild
      {{
          .textFunc = [this] -> std::string
          {
              if(const auto &guildName = m_hero->getGuildName(); guildName.empty()){
                  return {};
              }
              else if(const auto &title = m_hero->getGuildTitle(); title.empty()){
                  return guildName;
              }
              else{
                  return str_printf("%s[%s]", guildName.c_str(), title.c_str());
              }
          },

          .font
          {
              .color = [this]{ return m_hero->getNameColor(); },
          },

          .attrs
          {
              .show = [this]{ return !m_guild.empty(); },
          },
      }}

    , m_spouse
      {{
          .textFunc = [] -> std::string
          {
              return {}; // no marriage yet
          },

          .attrs
          {
              .show = [this]{ return !m_spouse.empty(); },
          },
      }}

    , m_titleList
      {{
          .align = ItemAlign::CENTER, // titles granted by GM, none yet
      }}

    , m_flex
      {{
          .align = ItemAlign::CENTER,
          .childList
          {
              {std::addressof(m_name     ), false},
              {std::addressof(m_guild    ), false},
              {std::addressof(m_spouse   ), false},
              {std::addressof(m_titleList), false},
          },

          .parent
          {
              .widget = this,
              .autoDelete = false,
          },
      }}
{
    m_titleList.setShow([this]{ return m_titleList.hasShowItem(); });
}
