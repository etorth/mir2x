#include <regex>
#include <cstring>
#include "idstrf.hpp"

// regex for 0~255
// same with colorf.cpp _REGEX_COL_0_255 but don't support hex format

#define _REGEX_BYTE_0_255 R"###(((?:\d)|(?:[^0]\d)|(?:1\d\d)|(?:2[0-4]\d)|(?:25[0-5])))###"
//                              -   --     ------     -----     --------     -------
//                              ^   ^         ^         ^          ^            ^
//                              |   |         |         |          |            |
//                              |   |         |         |          |            +----------  250  ~ 255
//                              |   |         |         |          +-----------------------  200  ~ 249
//                              |   |         |         +----------------------------------  100  ~ 199
//                              |   |         +--------------------------------------------  19   ~ 99
//                              |   +------------------------------------------------------  0    ~ 9
//                              +----------------------------------------------------------  create a group

#define _REGEX_EMAIL_DOMAIN_DIGITS R"###(()###" _REGEX_BYTE_0_255 R"###(\.)###" _REGEX_BYTE_0_255 R"###(\.)###" _REGEX_BYTE_0_255 R"###(\.)###" _REGEX_BYTE_0_255 R"###())###"

bool idstrf::isEmail(const char *s)
{
    if(str_haschar(s)){
        // see here
        //
        //      https://stackoverflow.com/questions/46155/how-to-validate-an-email-address-in-javascript
        //
        // this regex is not perfect, I changed the hex domain part
        // and also it allows double-quate in email address like: "abcd"@gamil.com, this is strange
        const static std::regex emailPtn
        {
            R"###(^)###"                                                            // start achor
            R"###(()###"                                                            //      start quote for body match
            R"###(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*))###"          //          body to support like abcd.ef@gmail.com
            R"###(|)###"                                                            //      body-or
            R"###((".+"))###"                                                       //          body to support like "你好"@gmail.com
            R"###())###"                                                            //      end quote for body match
            R"###(@)###"                                                            // @
            R"###(()###"                                                            //      start quote for domain match
            _REGEX_EMAIL_DOMAIN_DIGITS                                              //          domain to support hello@123.45.67.99
            R"###(|)###"                                                            //      domain-or
            R"###((([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))###"                            //          domain to support hellow@gmail.com or hello@abcd.com.cn
            R"###())###"                                                            //      end quote for domain match
            R"###($)###"                                                            // end achor
        };
        return std::regex_match(s, emailPtn);
    }
    return false;
}

bool idstrf::isCharName(const char *s)
{
    return str_haschar(s) && std::strlen(s) > 6;
}

bool idstrf::isPassword(const char *s)
{
    if(str_haschar(s)){
        size_t digits   = 0;    // 0-9
        size_t lowers   = 0;    // a-z
        size_t uppers   = 0;    // A-Z
        size_t specials = 0;    // ~!@#$%^&*() <= first line on keyboard
        size_t length   = 0;

        for(size_t i = 0; s[i]; ++i){
            if(s[i] >= '0' && s[i] <= '9'){
                digits++;
            }
            else if(s[i] >= 'a' && s[i] <= 'z'){
                lowers++;
            }
            else if(s[i] >= 'A' && s[i] <= 'Z'){
                uppers++;
            }
            else if(std::string_view("~!@#$%^&*()").find(s[i]) != std::string_view::npos){
                specials++;
            }
            length++;
        }

        return length   >= 8
            && digits   >= 1
            && lowers   >= 1
            && uppers   >= 1
            && specials >= 1;
    }
    return false;
}
