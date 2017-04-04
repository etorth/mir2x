/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *  Last Modified: 04/04/2017 11:26:38
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

#include <cstdio>
#include <cstring>
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

                    {
                        auto nActionIndex = (nWilIndex % 3000) / 80;
                        if((nActionIndex >= 0) && (nActionIndex < 33)){
                            static const uint32_t nActionTable[] = {
                                0,      // 00:  stand
                                1,      // 01:  arrow shoot
                                2,      // 02:  magic cast 0
                                3,      // 03:  magic cast 1
                                4,      // 04:  withstand
                                5,      // 05:  under kick 0
                                6,      // 06:  under kick 1
                                7,      // 07:  prepare to attack
                                8,      // 08:  cut
                                9,      // 09:  attack by one hand
                                10,     // 10:  attack by two hands 0
                                11,     // 11:  attack by one hand, used for half-moon
                                12,     // 12:  attack by two hands 1
                                13,     // 13:  attack by two hands 2
                                14,     // 14:  attack by two hands 3
                                15,     // 15:  under attack
                                16,     // 16:  attack by one hand, used for swoop-attack
                                17,     // 17:  attack by one hand, used for double-attack
                                18,     // 18:  kick
                                19,     // 19:  die
                                20,     // 20:  die on horse
                                21,     // 21:  walk
                                22,     // 22:  run
                                23,     // 23:  push
                                24,     // 24:  back roll
                                25,     // 25:  fishing
                                26,     // 26:  stop fishing
                                27,     // 27:  cast the fish rod
                                28,     // 28:  get fish
                                29,     // 29:  stand on horse
                                30,     // 30:  walk on horse
                                31,     // 31:  run on horse
                                32,     // 32:  under attack on horse
                            };
                            nAction = nActionTable[nActionIndex];
                        }else{
                            return 1;
                        }
                    }

                    nDirection = (((nWilIndex % 3000) % 80) / 10);

                    if((((nWilIndex % 3000) % 80) % 10) >= 1){
                        nFrame = (((nWilIndex % 3000) % 80) % 10) - 1;
                    }else{
                        return 1;
                    }

                    // output
                    {
                        std::printf("%08X\n", 0
                                + (nShadow    << 23)
                                + (nSex       << 22)
                                + (nDress     << 14)
                                + (nAction    <<  8)
                                + (nDirection <<  5)
                                + (nFrame     <<  0));

                        std::printf("Shadow     : %02d\n", nShadow   );
                        std::printf("Sex        : %02d\n", nSex      );
                        std::printf("Dress      : %02d\n", nDress    );
                        std::printf("Action     : %02d\n", nAction   );
                        std::printf("Direction  : %02d\n", nDirection);
                        std::printf("Frame      : %02d\n", nFrame    );
                    }

                    return 0;
                }
            }
        }
    }
    return 1;
}
