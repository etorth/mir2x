/*
 * =====================================================================================
 *
 *       Filename: imagedb.hpp
 *        Created: 02/14/2016 16:33:12
 *    Description: Handle operation against wilimagepackage
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <optional>
#include "wilimagepackage.hpp"

class ImageDB
{
    private:
        const WILIMAGEINFO *m_wilImageInfo = nullptr;

    private:
        std::vector<uint32_t> m_decodeBuf;
        std::vector<std::unique_ptr<WilImagePackage>> m_packageList;

    public:
        ImageDB(const char *pathName)
        {
            m_packageList.reserve(256);
            for(int i = 0;; ++i){
                if(const auto fileName = dbName(i)){
                    try{
                        m_packageList.push_back(std::make_unique<WilImagePackage>(pathName, fileName));
                    }
                    catch(...){
                        m_packageList.push_back({});
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
            m_wilImageInfo = nullptr;
            if(fileIndex >= to_u8(m_packageList.size())){
                return nullptr;
            }
            return m_wilImageInfo = m_packageList.at(fileIndex)->setIndex(imageIndex);
        }

    public:
        const WILIMAGEINFO *currImageInfo() const
        {
            return m_wilImageInfo;
        }

        std::tuple<const uint32_t *, size_t, size_t> decode(uint8_t fileIndex, uint16_t imageIndex, uint32_t color0, uint32_t color1, uint32_t color2)
        {
            if(setIndex(fileIndex, imageIndex)){
                const size_t width  = m_packageList.at(fileIndex)->currImageInfo()->width;
                const size_t height = m_packageList.at(fileIndex)->currImageInfo()->height;

                m_decodeBuf.resize(width * height);
                m_packageList.at(fileIndex)->decode(m_decodeBuf.data(), color0, color1, color2);
                return {m_decodeBuf.data(), width, height};
            }
            return {nullptr, 0, 0};
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
