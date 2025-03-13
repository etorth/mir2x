#pragma once
#include <map>
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include "mathf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "wilanitimer.hpp"

class BaseArea: public Fl_Box
{
    protected:
        WilAniTimer m_aniTimer;

    protected:
        std::unique_ptr<Fl_Image> m_lightImge;

    private:
        std::unordered_map<uint32_t, std::unique_ptr<Fl_Image>> m_coverList;

    protected:
        int m_mouseX = 0; // reset in Fl_Box::handle()
        int m_mouseY = 0;

    public:
        BaseArea(int argX, int argY, int argW, int argH)
            : Fl_Box(argX, argY, argW, argH)
            , m_lightImge([]() -> Fl_Image *
              {
                  const int r = 200;
                  const int size = 1 + (r - 1) * 2;
                  const uint32_t color = 0X001286FF;
                  std::vector<uint32_t> imgBuf(size * size, 0X00000000);

                  for(int x = 0; x < size; ++x){
                      for(int y = 0; y < size; ++y){
                          if(const auto r2 = mathf::LDistance2<int>(x - r + 1, y - r + 1, 0, 0); r2 <= r * r){
                              const auto alpha    = to_u32(0X02 + std::lround(0X2F * (1.0 - to_f(r2) / (r * r))));
                              const auto alpha255 = std::min<uint32_t>(alpha, 255);
                              imgBuf[x + y * size] = (alpha255 << 24) | (color & 0X00FFFFFF);
                          }
                      }
                  }
                  return Fl_RGB_Image((uchar *)(imgBuf.data()), size, size, 4, 0).copy();
              }())
        {}

    public:
        virtual std::tuple<int, int> offset() const = 0;

    public:
        void drawImage(Fl_Image *, int, int);
        void drawImage(Fl_Image *, int, int, int, int, int, int);

    public:
        void drawText(int, int, const char *, ...);
        void drawImageCover(Fl_Image *, int, int, int, int);

    public:
        void drawCircle(int, int, int);

    public:
        void drawLine(int, int, int, int);
        void drawLoop(int, int, int, int, int, int);
        void drawRectangle(int, int, int, int);

    public:
        void clear();

    public:
        void update(uint32_t loopTime)
        {
            m_aniTimer.update(loopTime);
        }

    public:
        void fillGrid     (int, int, int, int, uint32_t);
        void fillRectangle(int, int, int, int, uint32_t);

    private:
        Fl_Image *retrieveImageCover(uint32_t);
        Fl_Image *createImageCover(int, int, uint32_t);

    public:
        virtual std::optional<std::tuple<size_t, size_t>> getROISize() const = 0;

    public:
        std::tuple<size_t, size_t> getScrollPixelCount() const
        {
            const auto roiSize = getROISize();
            if(!roiSize.has_value()){
                return {0, 0};
            }

            const auto [xpCount, ypCount] = roiSize.value();
            return
            {
                [xpCount, this]() -> size_t
                {
                    if(to_d(xpCount) > w()){
                        return to_d(xpCount) - w();
                    }
                    return 0;
                }(),

                [ypCount, this]() -> float
                {
                    if(to_d(ypCount) > h()){
                        return to_d(ypCount) - h();
                    }
                    return 0;
                }(),
            };
        }

        std::tuple<float, float> getScrollPixelRatio(int dx, int dy) const
        {
            const auto [xpCount, ypCount] = getScrollPixelCount();
            return
            {
                (xpCount > 0) ? to_f(dx) / xpCount : 0.0,
                (ypCount > 0) ? to_f(dy) / ypCount : 0.0,
            };
        }

    protected:
        std::tuple<int, int> mouseGrid() const
        {
            const auto [offsetX, offsetY] = offset();
            return
            {
                (m_mouseX - x() + offsetX) / SYS_MAPGRIDXP,
                (m_mouseY - y() + offsetY) / SYS_MAPGRIDYP,
            };
        }
};
