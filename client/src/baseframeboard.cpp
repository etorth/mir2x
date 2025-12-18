#include "imageboard.hpp"
#include "baseframeboard.hpp"

extern PNGTexDB *g_progUseDB;

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

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              this->parent()->setShow(false);
          },

          .attrs
          {
              .inst
              {
                  .moveOnFocus = false,
              }
          },
          .parent{this},
      }}
{}
