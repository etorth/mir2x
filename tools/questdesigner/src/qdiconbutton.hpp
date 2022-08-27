#pragma once
#include <atomic>
#include <memory>
#include <cstdint>
#include <FL/Fl_Button.H>
#include <FL/Fl_Shared_Image.H>
#include "fflerror.hpp"

class QD_IconButton: public Fl_Button
{
    private:
        class Inn_ScaleIcon: public Fl_Shared_Image
        {
            public:
                Inn_ScaleIcon(Fl_Image *img)
                    : Fl_Shared_Image([]() -> std::string
                      {
                          static std::atomic<uint64_t> s_counter {0};
                          return std::string("inn_icon_index_") + std::to_string(s_counter++);
                      }().c_str(),

                      [img]()
                      {
                          fflassert(img);
                          return img;
                      }())
                {}
        };

    private:
        std::unique_ptr<Inn_ScaleIcon> m_icon;

    public:
        QD_IconButton(int argX, int argY, int argW, int argH, const char *argLabel = 0)
            : Fl_Button(argX, argY, argW, argH, argLabel)
        {}

    public:
        void image(Fl_Image *img)
        {
            // fluid calls o->image()
            // but Fl_Widget::image() is not a virtual function

            m_icon = std::make_unique<Inn_ScaleIcon>(img);
            Fl_Button::image(m_icon.get());
            if(m_icon){
                m_icon->scale(w(), h(), 0, 1);
            }
        }

    public:
        void resize(int argX, int argY, int argW, int argH) override
        {
            Fl_Button::resize(argX, argY, argW, argH);
            if(m_icon){
                m_icon->scale(argW, argH, 0, 1);
            }
        }

    public:
        void draw() override
        {
            Fl_Button::draw();
            if(m_icon){
                m_icon->draw(x(), y(), w(), h(), 0, 0);
            }
        }
};
