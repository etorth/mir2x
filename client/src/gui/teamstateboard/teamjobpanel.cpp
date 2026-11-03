#include <format>
#include <utility>
#include <charconv>
#include <algorithm>
#include <string_view>
#include "colorf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "teamjobpanel.hpp"

extern SDLDevice *g_sdlDevice;
TeamJobPanel::TeamJobPanel(TeamJobPanel::InitArgs args)
    : Widget
      {{
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = args.w,
          .h = args.h,

          .attrs
          {
              .inst
              {
                  .show = std::move(args.show),
                  .moveOnFocus = false,
              },
          },
          .parent = std::move(args.parent),
      }}

    , m_jobIndex(fflcheck(args.jobIndex, args.jobIndex < TeamRequirement::jobInfoList.size()))
    , m_requirement(fflcheck(args.requirement))

    , m_title
      {{
          .dir = DIR_LEFT,
          .y = m_titleH / 2,
          .textFunc = std::string(to_cstr(TeamRequirement::jobInfoList.at(args.jobIndex).name)) + to_cstr(u8" 加入要求"),
          .font
          {
              .color = colorf::YELLOW_A255,
          },
          .parent{this},
      }}

    , m_minLevel{{.dir = DIR_LEFT, .x =   0, .y = m_titleH + m_inputRowH / 2, .labelFirst = u8"最低等级", .inputSize{40, 20}, .parent{this}}}
    , m_maxLevel{{.dir = DIR_LEFT, .x = 135, .y = m_titleH + m_inputRowH / 2, .labelFirst = u8"最高等级", .inputSize{40, 20}, .parent{this}}}
    , m_minStat
      {{
          .dir = DIR_LEFT,
          .x = 270,
          .y = m_titleH + m_inputRowH / 2,
          .labelFirst = (std::u8string(u8"最低") + TeamRequirement::jobInfoList.at(args.jobIndex).stat).c_str(),
          .inputSize{40, 20},
          .parent{this},
      }}

    , m_magicLabel
      {{
          .dir = DIR_LEFT,
          .y = m_magicLabelY,
          .textFunc = [this]
          {
              return std::format("需要技能（已选{}）", m_requirement->magicIDList.size());
          },
          .parent{this},
      }}

      // fixed size, auto-size scans all children per call
      // ScrollContainer queries it per child per event, which is O(N^2)

    , m_magicCanvas
      {{
          .w = args.w - 12,
          .h = 0, // set by buildMagicList()

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
              },
          },
      }}

    , m_magicScroll
      {{
          .y = m_magicViewY,

          .vpw = args.w - 12,
          .vph = (args.h - m_magicViewY - 36 - 4) / m_magicRowH * m_magicRowH + 4, // show full rows only

          .getter = &m_magicCanvas,

          .hBar = false,
          .hScroll = false,

          .fgDrawFunc = [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_reset
      {{
          .y = args.h - 28,

          .textFunc = to_cstr(u8"重置"),
          .onTrigger = [this](Widget *, int)
          {
              resetInput();
          },

          .parent{this},
      }}

    , m_back
      {{
          .dir = DIR_UPRIGHT,
          .x = args.w,
          .y = args.h - 28,

          .textFunc = to_cstr(u8"返回"),
          .onTrigger = [onBack = std::move(args.onBack)](Widget *, int)
          {
              if(onBack){
                  onBack();
              }
          },

          .parent{this},
      }}
{
    for(auto input: {&m_minLevel, &m_maxLevel, &m_minStat}){
        setupNumberInput(*input);
    }
    buildMagicList();
}

void TeamJobPanel::buildMagicList()
{
    const auto jobName = TeamRequirement::jobInfoList.at(m_jobIndex).name;
    const int columnW = (m_magicCanvas.w() - 8) / m_magicColumnCount;

    int count = 0;
    for(uint32_t magicID = 1; magicID < DBCOM_MAGICENDID(); ++magicID){
        const auto &mr = DBCOM_MAGICRECORD(magicID);
        if(!mr || !mr.req.job || std::u8string_view(mr.req.job) != jobName){
            continue;
        }

        new CheckLabel
        {{
            .x = 6 + (count % m_magicColumnCount) * columnW,
            .y = 4 + (count / m_magicColumnCount) * m_magicRowH,

            .label{.text = mr.name},
            .getter = [this, magicID]{ return m_requirement->magicIDList.contains(magicID); },
            .setter = [this, magicID](bool value)
            {
                if(value){
                    m_requirement->magicIDList.insert(magicID);
                }
                else{
                    m_requirement->magicIDList.erase(magicID);
                }
            },

            .parent{&m_magicCanvas, true},
        }};
        count++;
    }
    m_magicCanvas.setH(8 + (count + m_magicColumnCount - 1) / m_magicColumnCount * m_magicRowH);
}

void TeamJobPanel::resetInput()
{
    *m_requirement = TeamJobRequirement
    {
        .allow = m_requirement->allow,
    };

    for(auto input: {&m_minLevel, &m_maxLevel, &m_minStat}){
        input->inputLine()->clear();
    }
}

void TeamJobPanel::applyInput()
{
    m_requirement->minLevel = parseNumberInput(m_minLevel);
    m_requirement->maxLevel = parseNumberInput(m_maxLevel);
    m_requirement->minStat  = parseNumberInput(m_minStat );
}

void TeamJobPanel::setupNumberInput(TextInput &input)
{
    input.inputLine()->setValidateFunc([](std::string currInput, std::string newInput)
    {
        return currInput.size() + newInput.size() <= 3 && std::ranges::all_of(newInput, [](char ch){ return ch >= '0' && ch <= '9'; });
    });
}

int TeamJobPanel::parseNumberInput(const TextInput &input)
{
    const auto s = input.inputLine()->getRawString();
    int result = 0;

    std::from_chars(s.data(), s.data() + s.size(), result);
    return result;
}
