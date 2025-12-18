#pragma once
#include <unordered_map>
#include <optional>
#include <vector>
#include <tuple>
#include <algorithm>

class SkillBoardConfig final
{
    private:
        struct MagicConfig
        {
            int level = -1;
            std::optional<char> key;
        };

    private:
        std::unordered_map<uint32_t, MagicConfig> m_learnedMagicList;

    public:
        std::optional<char> getMagicKey  (uint32_t) const;
        std::optional<int > getMagicLevel(uint32_t) const;

    public:
        bool hasMagicID(uint32_t magicID) const
        {
            return m_learnedMagicList.count(magicID) > 0;
        }

    public:
        void setMagicLevel(uint32_t, int);
        void setMagicKey  (uint32_t, std::optional<char>);

    public:
        auto getMagicKeyList() const
        {
            std::vector<std::tuple<uint32_t, char>> result;
            for(const auto &p: m_learnedMagicList){
                if(p.second.key.has_value()){
                    result.emplace_back(p.first, p.second.key.value());
                }
            }

            std::sort(result.begin(), result.end());
            return result;
        }

    public:
        uint32_t key2MagicID(char key) const
        {
            for(const auto &p: m_learnedMagicList){
                if(p.second.key.has_value() && p.second.key.value() == key){
                    return p.first;
                }
            }
            return 0;
        }
};
