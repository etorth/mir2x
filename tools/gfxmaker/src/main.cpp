/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *  Last Modified: 04/03/2017 23:50:07
 *
 *    Description: for file name translation, for example
 *                      id_translate 009641 1
 *                 means
 *                      1. input 009641
 *                      2. sex as male, or 0 as female
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
    if(argc == 3){
        if((std::strlen(argv[2]) == 1) && ((argv[2][0] == '0') || (argv[2][0] == '1'))){
            if(std::strlen(argv[1]) == 6){
                for(int nIndex = 0; nIndex < 6; ++nIndex){
                    if(argv[1][nIndex] < '0' || argv[1][nIndex] > '9'){
                        return 1;
                    }
                }

                auto nWilIndex = std::atoll(argv[1]);
                {
                    uint32_t nShadow    = 0;
                    uint32_t nSex       = 0;
                    uint32_t nDress     = 0;
                    uint32_t nAction    = 0;
                    uint32_t nDirection = 0;
                    uint32_t nFrame     = 0;

                    nShadow = 0;
                    nSex    = std::atol(argv[2]);
                    nDress  = nWilIndex / 3000;
                }
            }



            std::string szFileName = argv[1];
            if(szFileName.size() == 6){

            }
        }
        return 1;
    }
