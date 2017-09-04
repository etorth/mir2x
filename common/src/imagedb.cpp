/*
 * =====================================================================================
 *
 *       Filename: imagedb.cpp
 *        Created: 02/14/2016 16:35:49
 *  Last Modified: 09/03/2017 20:14:40
 *
 *    Description:
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

#include <cstring>
#include "imagedb.hpp"

const static char *g_DBFileName[]
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
    ""
};

const char *ImageDB::DBName(int nIndex) const
{
    if(true
            && nIndex > 0
            && nIndex < (int)(sizeof(g_DBFileName) / sizeof(g_DBFileName[0]))){
        return g_DBFileName[nIndex];
    }
    return nullptr;
}

bool ImageDB::LoadDB(const char *szPathName)
{
    for(int nIndex = 0; std::strlen(g_DBFileName[nIndex]) > 0; ++nIndex){
        Load((uint8_t)(nIndex), szPathName, g_DBFileName[nIndex], ".wil");
    }
    return true;
}

bool ImageDB::Load(uint8_t nFileIndex, const char *szPathName, const char *szFileName, const char *szNamePrefix)
{
    return m_ImagePackage[nFileIndex].Load(szPathName, szFileName, szNamePrefix);
}

bool ImageDB::Valid(uint8_t nFileIndex, uint16_t nImageIndex)
{
    if(nFileIndex == 255 || nImageIndex == 65535){ return false; }

    if(true
            && m_ImagePackage[nFileIndex].SetIndex(nImageIndex)
            && m_ImagePackage[nFileIndex].CurrentImageValid()){
        int nW = m_ImagePackage[nFileIndex].CurrentImageInfo().shWidth;
        int nH = m_ImagePackage[nFileIndex].CurrentImageInfo().shHeight;
        if((nW > 0) && (nH > 0)){
            return true;
        }
    }
    return false;
}

int ImageDB::FastW(uint8_t nFileIndex)
{
    return m_ImagePackage[nFileIndex].CurrentImageInfo().shWidth;
}

int ImageDB::FastH(uint8_t nFileIndex)
{
    return m_ImagePackage[nFileIndex].CurrentImageInfo().shHeight;
}

int ImageDB::W(uint8_t nFileIndex, uint16_t nImageIndex)
{
    return Valid(nFileIndex, nImageIndex) ? FastW(nFileIndex) : 0;
}

int ImageDB::H(uint8_t nFileIndex, uint16_t nImageIndex)
{
    return Valid(nFileIndex, nImageIndex) ? FastH(nFileIndex) : 0;
}

const uint32_t *ImageDB::FastDecode(uint8_t nFileIndex, uint32_t nC0, uint32_t nC1, uint32_t nC2)
{
    int nW = FastW(nFileIndex);
    int nH = FastH(nFileIndex);

    m_Buf.resize(nW * nH);
    m_ImagePackage[nFileIndex].Decode(&(m_Buf[0]), nC0, nC1, nC2);

    return &(m_Buf[0]);
}

const uint32_t *ImageDB::Decode(uint8_t nFileIndex, uint16_t nImageIndex, uint32_t nC0, uint32_t nC1, uint32_t nC2)
{
    return Valid(nFileIndex, nImageIndex) ? FastDecode(nFileIndex, nC0, nC1, nC2) : nullptr;
}

const WILIMAGEINFO &ImageDB::ImageInfo(uint8_t nFileIndex, uint16_t nImageIndex)
{
    m_ImagePackage[nFileIndex].SetIndex(nImageIndex);
    return m_ImagePackage[nFileIndex].CurrentImageInfo();
}

const WilImagePackage &ImageDB::GetPackage(uint8_t nFileIndex)
{
    return m_ImagePackage[nFileIndex];
}
