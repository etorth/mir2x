/*
 * =====================================================================================
 *
 *       Filename: animationdb.cpp
 *        Created: 06/22/2016 18:21:16
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
#include <filesystem>
#include "hexstring.hpp"
#include "animationdb.hpp"

bool AnimationDB::Load(const char *szDBPath)
{
    // 1. check argument
    if(!(szDBPath && std::strlen(szDBPath))){
        return false;
    }

    // 2. record this path
    m_DBPath = szDBPath;
    while(m_DBPath.back() == '/'){
        m_DBPath.pop_back();
    }

    if(m_DBPath.empty()){
        return false;
    }

    for(auto &p: std::filesystem::directory_iterator(m_DBPath.c_str())){
        if(!p.is_regular_file()){
            continue;
        }

        auto szFileName = p.path().filename().u8string();
        if(szFileName.size() != (18 + 4)){ continue; }
        if(szFileName[0] != '0'){ continue; }
        if((szFileName[1] != '0') && (szFileName[1] != '1')){ continue; }
        if((szFileName.substr(18) != ".PNG") && (szFileName.substr(18) != ".png")){ continue; }

        // 4. ok it's time to get the info
        uint32_t nMonsterID = 0;
        uint32_t nAction    = 0;
        uint32_t nDirection = 0;
        uint32_t nFrame     = 0;

        auto nDesc = HexString::ToHex<uint32_t, 4>(szFileName.c_str());

        nMonsterID = ((nDesc & 0X00FFF000) >> 12);
        nAction    = ((nDesc & 0X00000F00) >>  8);
        nDirection = ((nDesc & 0X000000E0) >>  5);
        nFrame     = ((nDesc & 0X0000001F) >>  0);

        // we refuse to accept frame with MonsterID == 0
        if(nMonsterID == 0){ continue; }

        auto nOffset = HexString::ToHex<uint32_t, 4>(szFileName.c_str() + 10);

        int nDX = ((szFileName[8] == '0') ? -1 : 1) * (int)((nOffset & 0XFFFF0000) >> 16);
        int nDY = ((szFileName[9] == '0') ? -1 : 1) * (int)((nOffset & 0X0000FFFF) >>  0);

        Add(nMonsterID, nAction, nDirection, nFrame, (szFileName[1] == '1'), nDX, nDY, (m_DBPath + "/" + szFileName));
        // since we didn't update this directory, so we don't need rewinddir()
    }
    return true;
}
