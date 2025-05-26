#include <array>
#include "fflerror.hpp"
#include "pwdf.hpp"

int pwdf::strength(const char *s)
{
    fflassert(s);

    int length = 0;
    int digits = 0;
    int lowers = 0;
    int uppers = 0;
    int specials = 0;

    std::array<uint8_t, 256> counts{};
    for(; s[length] != '\0'; ++length){
        if     (s[length] >= '0' && s[length] <= '9') ++digits;
        else if(s[length] >= 'a' && s[length] <= 'z') ++lowers;
        else if(s[length] >= 'A' && s[length] <= 'Z') ++uppers;
        else if(s[length] == '!' || s[length] == '@' || s[length] == '#' ||
                s[length] == '$' || s[length] == '%' || s[length] == '^' ||
                s[length] == '&' || s[length] == '*' || s[length] == '(' ||
                s[length] == ')' || s[length] == '-' || s[length] == '+') ++specials;

        counts[s[length]]++;
        if(length >= 255){
            break;
        }
    }

    if(length < 8 || digits == 0 || lowers == 0 || uppers == 0 || specials == 0){
        return pwdf::INVALID;
    }

    const auto unichars = std::count_if(counts.begin(), counts.end(), [](const auto &x)
    {
        return x > 0;
    });

    if(unichars * 2 < length){
        return pwdf::WEAK;
    }

    int score = 0;

    if(length   >= 12) score++;
    if(uppers   >=  2) score++;
    if(specials >=  2) score++;

    if(unichars * 4 >= length * 3) score++;

    if     (score >= 4) return pwdf::STRONG;
    else if(score >= 2) return pwdf::MODERATE;
    else                return pwdf::WEAK;
}
