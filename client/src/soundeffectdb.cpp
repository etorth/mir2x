#include "soundeffecthandle.hpp"
#include "clientargparser.hpp"
#include "soundeffectdb.hpp"
#include "hexstr.hpp"

// sound effect is directly copy of mir2ei/Sound/*.wav, with name remapping
// for index and it's meaning, check: https://github.com/lzxsz/MIR2/blob/master/GameOfMir/Client/SoundUtil.pas
//
// naming-------------------+-index off------+-exists in lzxsz/MIR2-+-existing in etorth/CBWCQ3-+-base offset in etorth/mir2x-+-comments--------
// s_walk_ground_l          =  1.wav           yes                    yes                         0X01000000                    copy from etorth/CBWCQ3
// s_walk_ground_r          =  2.wav           ...
// s_run_ground_l           =  3.wav
// s_run_ground_r           =  4.wav
// s_walk_stone_l           =  5.wav
// s_walk_stone_r           =  6.wav
// s_run_stone_l            =  7.wav
// s_run_stone_r            =  8.wav
// s_walk_lawn_l            =  9.wav
// s_walk_lawn_r            = 10.wav
// s_run_lawn_l             = 11.wav
// s_run_lawn_r             = 12.wav
// s_walk_rough_l           = 13.wav
// s_walk_rough_r           = 14.wav
// s_run_rough_l            = 15.wav
// s_run_rough_r            = 16.wav
// s_walk_wood_l            = 17.wav
// s_walk_wood_r            = 18.wav
// s_run_wood_l             = 19.wav
// s_run_wood_r             = 20.wav
// s_walk_cave_l            = 21.wav
// s_walk_cave_r            = 22.wav
// s_run_cave_l             = 23.wav
// s_run_cave_r             = 24.wav
// s_walk_room_l            = 25.wav
// s_walk_room_r            = 26.wav
// s_run_room_l             = 27.wav
// s_run_room_r             = 28.wav
// s_walk_water_l           = 29.wav
// s_walk_water_r           = 30.wav
// s_run_water_l            = 31.wav
// s_run_water_r            = 32.wav
//
// s_horse_walk_l           = 33.wav          no                      yes                         0X01000000                    copy from etorth/CBWCQ3
// s_horse_walk_r           = 34.wav          ...
// s_horse_run_l            = 35.wav
// s_horse_run_r            = 36.wav          no                      no                          0X01000000                    identical copy of s_horse_run_l
//
// s_hit_short              = 50.wav          yes                     yes                         0X01010000                    短剑
// s_hit_wooden             = 51.wav          ...                                                                               木剑
// s_hit_sword              = 52.wav                                                                                            长剑
// s_hit_do                 = 53.wav                                                                                            刀
// s_hit_axe                = 54.wav                                                                                            斧
// s_hit_club               = 55.wav                                                                                            梃: 裁决
// s_hit_long               = 56.wav                                                                                            扇/棍/法杖: 无极棍 魔杖, etorth/CBWCQ3 doesn't have this weapon attribute
// s_hit_fist               = 57.wav                                                                                            赤手空拳
//
// s_struck_short           = 60.wav
// s_struck_wooden          = 61.wav
// s_struck_sword           = 62.wav
// s_struck_do              = 63.wav
// s_struck_axe             = 64.wav
// s_struck_club            = 65.wav
//
// s_struck_body_sword      = 70.wav
// s_struck_body_axe        = 71.wav
// s_struck_body_longstick  = 72.wav
// s_struck_body_fist       = 73.wav
//
// s_struck_armor_sword     = 80.wav
// s_struck_armor_axe       = 81.wav
// s_struck_armor_longstick = 82.wav
// s_struck_armor_fist      = 83.wav
//
// s_strike_stone           = 91.wav           yes                    no                          NA                            not used
// s_drop_stonepiece        = 92.wav           ...
//
// s_rock_door_open         = 100.wav          yes                    yes                         0X01020000
// s_meltstone              = 101.wav          ...
// s_intro_theme            = 102.wav          yes                    no                          NA                            etorth/CBWCQ3 doesn't have this file
// s_main_theme             = 102.wav          yes                    no                          NA                            same as above
// s_norm_button_click      = 103.wav          yes                    yes                         0X01020000
// s_rock_button_click      = 104.wav          ...
// s_glass_button_click     = 105.wav
// s_money                  = 106.wav
// s_eat_drug               = 107.wav
// s_click_drug             = 108.wav
// s_spacemove_out          = 109.wav
// s_spacemove_in           = 110.wav
//
// s_click_weapon           = 111.wav
// s_click_armor            = 112.wav
// s_click_ring             = 113.wav
// s_click_armring          = 114.wav
// s_click_necklace         = 115.wav
// s_click_helmet           = 116.wav
// s_click_grobes           = 117.wav
// s_itmclick               = 118.wav
// s_unknown_sound_1        = 125.wav          no                     yes                         0X01020000                    etorth/CBWCQ3 has this but SoundList.wwl doesn't refers to this file, nor any code refers to it, guess abandoned item click sound
// s_unknown_sound_2        = 126.wav          no                     yes                         0X01020000                    same as above
//
// s_yedo_man               = 130.wav          yes                    no                          NA                            etorth/CBWCQ3 doesn't have this file, SoundList.wwl refers to m-xxx.wav for magic sound effects like this
// s_yedo_woman             = 131.wav          ...
// s_longhit                = 132.wav
// s_widehit                = 133.wav
// s_rush_l                 = 134.wav
// s_rush_r                 = 135.wav
// s_firehit_ready          = 136.wav
// s_firehit                = 137.wav
//
// s_man_struck             = 138.wav          yes                    yes                         0X01030000
// s_wom_struck             = 139.wav          ...
// s_man_die                = 144.wav
// s_wom_die                = 145.wav

// monster sound effect list, direct copy of mir2ei/Sound/*.wav, with renaming: xyz-p -> 0X02000000 + (xyz - 200) * 16 + p
// before renaming, changes:
// 1. remove '224-2 .wav', it's identical to '224-2.wav'
// 2. renaming 238,9-x.wav -> 800-x.wav, since 238 and 239 are both taken
//    this sound effect set hears like for some angry 钉耙猫 type, since 238 and a29 are 多钩猫 and 钉耙猫

// magic and monster sound effects, to be continued

extern ClientArgParser *g_clientArgParser;
std::optional<std::tuple<SoundEffectElement, size_t>> SoundEffectDB::loadResource(uint32_t key)
{
    if(g_clientArgParser->disableAudio){
        return {};
    }

    char soundEffectIndexString[16];
    std::vector<uint8_t> soundEffectDataBuf;

    if(!m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, soundEffectIndexString, true), 8, &soundEffectDataBuf)){
        return {};
    }

    if(soundEffectDataBuf.empty()){
        return {};
    }

    Mix_Chunk *chunkPtr = nullptr;
    if(auto rwOpsPtr = SDL_RWFromConstMem(soundEffectDataBuf.data(), soundEffectDataBuf.size())){
        chunkPtr = Mix_LoadWAV_RW(rwOpsPtr, SDL_TRUE);
    }

    if(!chunkPtr){
        return {};
    }

    return std::make_tuple(SoundEffectElement
    {
        .handle = std::shared_ptr<SoundEffectHandle>(new SoundEffectHandle
        {
            .chunk = chunkPtr,
            .chunkFileData = std::move(soundEffectDataBuf),
        }),
    }, 1);
}

void SoundEffectDB::freeResource(SoundEffectElement &element)
{
    // check SDL_mixer docmument
    // Mix_FreeMusic() stops music if it's playing, this is blocking when music doing fading out

    if(g_clientArgParser->disableAudio){
        return;
    }

    element.handle.reset();
}
