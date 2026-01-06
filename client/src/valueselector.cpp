#include "valueselector.hpp"

ValueSelector::ValueSelector(ValueSelector::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_input
      {{
          .w = std::move(args.input.w),
          .h = std::move(args.h),

          .enableIME = std::move(args.input.enableIME),

          .font   = std::move(args.input.font),
          .cursor = std::move(args.input.cursor),

          .onCR = [this]
          {
          },

          .validate = []
          {

          },
      }}

    , m_up
      {{
          .w = [this]
          {

          },

          .h = [this]{ return h() / 2; },

          .triangle
          {
              .dir = DIR_UP,
          },
      }}

    , m_down
      {{
          .w = [this]{ return       m_up.w(); },
          .h = [this]{ return h() - m_up.h(); },

          .triangle
          {
              .dir = DIR_DOWN,
          },
      }}

    , m_vflex
      {{
          .childList
          {
              {&m_up  , false},
              {&m_down, false},
          },
      }}

    , m_hflex
      {{
          .childList
          {
              {&m_input, false},
              {&m_vflex, false},
          },
      }}

    , m_frame
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](Widget::ROIMap roiMap) const
          {
              colorf::drawFrame(0, 0, w(), h(), colorf::GRAY_A255, 2, roiMap);
          },

          .parent = Widget::WADPair{&m_hflex, false},
      }}
{}
