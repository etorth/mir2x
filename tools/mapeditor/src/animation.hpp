#pragma once
#include <tuple>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <FL/Fl_PNG_Image.H>
#include "rawbuf.hpp"

class Animation
{
    private:
        struct InnFrame
        {
            int dx = 0;
            int dy = 0;

            // AnimationDB uses initializer_list to initialize its internal vector<Animation>, see AnimationDB::AnimationDB(...)
            // it requests Animation to be copy-constructable
            std::shared_ptr<Fl_Image> image {};
        };

    private:
        std::vector<InnFrame> m_frameList;

    public:
        Animation(std::initializer_list<std::tuple<int, int, std::initializer_list<uint8_t>>> ilist)
        {
            for(const auto &[dx, dy, data]: ilist){
                const Rawbuf imgData(data);
                m_frameList.push_back(InnFrame
                {
                    .dx = dx,
                    .dy = dy,
                    .image = std::shared_ptr<Fl_Image>(Fl_PNG_Image(nullptr, imgData.data(), imgData.size()).copy()),
                });
            }
        }

    public:
        size_t frameCount() const
        {
            return m_frameList.size();
        }

        std::tuple<int, int, Fl_Image *> frame(size_t index) const
        {
            return
            {
                m_frameList.at(index).dx,
                m_frameList.at(index).dy,
                m_frameList.at(index).image.get(),
            };
        }
};
