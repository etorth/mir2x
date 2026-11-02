#include "progusedb.hpp"
#include "imageboard.hpp"
#include "baseframeboard.hpp"

extern ProgUseDB *g_progUseDB;

BaseFrameBoard::BaseFrameBoard(BaseFrameBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = [argW = std::move(args.w), this]{ return std::max<int>(Widget::evalSize(argW, this), 2 * m_cornerSize); },
          .h = [argH = std::move(args.h), this]{ return std::max<int>(Widget::evalSize(argH, this), 2 * m_cornerSize); },

          .parent = std::move(args.parent),
      }}

    , m_frame
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(m_frameTexID);
          },
      }}

    , m_frameBoard
      {{
          .getter = &m_frame,
          .vr
          {
              m_cornerSize,
              m_cornerSize,
              m_frame.w() - 2 * m_cornerSize,
              m_frame.h() - 2 * m_cornerSize,
          },

          .resize
          {
              [this]{ return w() - 2 * m_cornerSize; },
              [this]{ return h() - 2 * m_cornerSize; },
          },

          .parent{this},
      }}

    , m_close
      {{
          .x = [this]{ return w() - 51; },
          .y = [this]{ return h() - 53; },

          .texAniOpt = 0X0000001C,

          .onTrigger = [this](Widget *, int)
          {
              this->parent()->setShow(false);
          },

          .attrs
          {
              .moveOnFocus = false,
          },
          .parent{this},
      }}
{}
