/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *  Last Modified: 04/03/2017 18:29:44
 *
 *    Description: for file name translation
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

#include <string>
#include <cstdlib>
#include <cstdint>
int main(int argc, char *argv[])
{
    if(argc == 2){
        if(std::strlen(argv[1]) == 6){
            for(int nIndex = 0; nIndex < 6; ++nIndex){
                if(argv[1][nIndex] < '0' || argv[1][nIndex] > '9'){
                    return 1;
                }
            }

            auto nRes = std::atoll(argv[1]);
            {
                uint32_t nShadow    = 0;
                uint32_t nSex       = 0;
                uint32_t nDress     = 0;
                uint32_t nAction    = 0;
                uint32_t nDirection = 0;
                uint32_t nFrame     = 0;
            }
        }



        std::string szFileName = argv[1];
        if(szFileName.size() == 6){

        }
    }
    return 1;
}
