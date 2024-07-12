#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include "wilimagepackage.hpp"

class ImageMapDB
{
    private:
        const WILIMAGEINFO *m_currImageInfo = nullptr;

    private:
        std::vector<uint32_t> m_emptyImage;
        std::vector<std::unique_ptr<WilImagePackage>> m_packageList;

    public:
        ImageMapDB(const char *pathName)
        {
            m_packageList.reserve(256);
            for(int i = 0;; ++i){
                if(const auto fileName = dbName(i)){
                    try{
                        m_packageList.push_back(std::make_unique<WilImagePackage>(pathName, fileName));
                    }
                    catch(...){
                        m_packageList.push_back({}); // mute failure
                    }
                }
                else{
                    break;
                }
            }
        }

    public:
        const WILIMAGEINFO *setIndex(uint8_t fileIndex, uint16_t imageIndex)
        {
            fflassert(to_uz(fileIndex) < m_packageList.size());
            fflassert(m_packageList.at(fileIndex));
            return m_currImageInfo = m_packageList.at(fileIndex)->setIndex(imageIndex);
        }

        const WILIMAGEINFO *setIndex(uint32_t imageIndex)
        {
            return setIndex(to_u8(imageIndex >> 16), to_u16(imageIndex));
        }

    public:
        void extract(std::function<void(uint8_t, uint16_t, const void *, size_t, size_t)> op, bool removeMosaic)
        {
            fflassert(op);
            for(uint8_t fileIndex = 0; to_uz(fileIndex) < m_packageList.size(); ++fileIndex){
                if(!m_packageList[fileIndex]){
                    continue;
                }

                for(uint16_t imageIndex = 0; to_uz(imageIndex) < m_packageList[fileIndex]->indexCount(); ++imageIndex){
                    if(const auto [imgBuf, imgW, imgH] = decode(fileIndex, imageIndex, removeMosaic); imgBuf){
                        op(fileIndex, imageIndex, imgBuf, imgW, imgH);
                    }
                }
            }
        }

    public:
        const WILIMAGEINFO *currImageInfo() const
        {
            return m_currImageInfo;
        }

        std::tuple<const uint32_t *, size_t, size_t> decode(uint8_t fileIndex, uint16_t imageIndex, bool removeMosaic)
        {
            if(const auto imgInfo = setIndex(fileIndex, imageIndex)){
                if(const auto layer = m_packageList.at(fileIndex)->decode(true, removeMosaic, false); layer[0]){
                    return
                    {
                        layer[0],
                        imgInfo->width,
                        imgInfo->height,
                    };
                }
                else{
                    m_emptyImage.resize(imgInfo->width * imgInfo->height, 0X00000000);
                    return
                    {
                        m_emptyImage.data(),
                        imgInfo->width,
                        imgInfo->height,
                    };
                }
            }
            return {nullptr, 0, 0};
        }

        std::tuple<const uint32_t *, size_t, size_t> decode(uint32_t imageIndex, bool removeMosaic)
        {
            return decode(to_u8(imageIndex >> 16), to_u16(imageIndex), removeMosaic);
        }

        const WilImagePackage *getPackage(uint8_t fileIndex) const
        {
            return m_packageList.at(fileIndex).get();
        }

    public:
        static const char *dbName(int fileIndex)
        {
            constexpr static const char *s_dbNameList[]
            {
                "Tilesc",
                "Tiles30c",
                "Tiles5c",
                "Smtilesc",
                "Housesc",
                "Cliffsc",
                "Dungeonsc",
                "Innersc",
                "Furnituresc",
                "Wallsc",
                "SmObjectsc",
                "Animationsc",
                "Object1c",
                "Object2c",
                "Custom",
                "Wood/Tilesc",
                "Wood/Tiles30c",
                "Wood/Tiles5c",
                "Wood/Smtilesc",
                "Wood/Housesc",
                "Wood/Cliffsc",
                "Wood/Dungeonsc",
                "Wood/Innersc",
                "Wood/Furnituresc",
                "Wood/Wallsc",
                "Wood/SmObjectsc",
                "Wood/Animationsc",
                "Wood/Object1c",
                "Wood/Object2c",
                "Wood/Custom",
                "Sand/Tilesc",
                "Sand/Tiles30c",
                "Sand/Tiles5c",
                "Sand/Smtilesc",
                "Sand/Housesc",
                "Sand/Cliffsc",
                "Sand/Dungeonsc",
                "Sand/Innersc",
                "Sand/Furnituresc",
                "Sand/Wallsc",
                "Sand/SmObjectsc",
                "Sand/Animationsc",
                "Sand/Object1c",
                "Sand/Object2c",
                "Sand/Custom",
                "Snow/Tilesc",
                "Snow/Tiles30c",
                "Snow/Tiles5c",
                "Snow/Smtilesc",
                "Snow/Housesc",
                "Snow/Cliffsc",
                "Snow/Dungeonsc",
                "Snow/Innersc",
                "Snow/Furnituresc",
                "Snow/Wallsc",
                "Snow/SmObjectsc",
                "Snow/Animationsc",
                "Snow/Object1c",
                "Snow/Object2c",
                "Snow/Custom",
                "Forest/Tilesc",
                "Forest/Tiles30c",
                "Forest/Tiles5c",
                "Forest/Smtilesc",
                "Forest/Housesc",
                "Forest/Cliffsc",
                "Forest/Dungeonsc",
                "Forest/Innersc",
                "Forest/Furnituresc",
                "Forest/Wallsc",
                "Forest/SmObjectsc",
                "Forest/Animationsc",
                "Forest/Object1c",
                "Forest/Object2c",
                "Forest/Custom",
            };

            if((fileIndex >= 0) && (fileIndex < to_d(std::extent_v<decltype(s_dbNameList)>))){
                return s_dbNameList[fileIndex];
            }
            return nullptr;
        }
};
