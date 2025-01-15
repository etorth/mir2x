#pragma once
namespace pwdf
{
    // A valid password should satisfy the following conditions:
    //
    //    It contains at least one digit.
    //    It contains at least one lowercase character.
    //    It contains at least one uppercase character.
    //    It contains at least one special character, the special characters are: !@#$%^&*()-+
    //    Its length is at least 8.

    enum
    {
        INVALID  = 0,
        WEAK     = 1,
        MODERATE = 2,
        STRONG   = 3,
    };

    int strength(const char *);
}
