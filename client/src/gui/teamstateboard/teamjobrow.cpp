#include <string>
#include <utility>
#include "totype.hpp"
#include "teamjobrow.hpp"

TeamJobRow::TeamJobRow(TeamJobRow::InitArgs args)
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
                  .moveOnFocus = false,
              },
          },
          .parent = std::move(args.parent),
      }}

    , m_requirement(fflcheck(args.requirement))

    , m_allow
      {{
          .dir = DIR_LEFT,
          .y = args.h / 2,

          .label{.text = (std::u8string(u8"允许") + TeamRequirement::jobInfoList.at(args.jobIndex).name + u8"加入").c_str()},
          .getter = [this]{ return m_requirement->allow; },
          .setter = [this](bool value){ m_requirement->allow = value; },

          .parent{this},
      }}

    , m_setup
      {{
          .dir = DIR_LEFT,
          .x = 140,
          .y = args.h / 2,

          .textFunc = to_cstr(u8"[设置]"),
          .onTrigger = [onSetup = std::move(args.onSetup)](Widget *, int)
          {
              if(onSetup){
                  onSetup();
              }
          },

          .attrs
          {
              .active = [this]{ return m_requirement->allow; },
          },
          .parent{this},
      }}
{}
