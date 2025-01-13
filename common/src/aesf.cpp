#include "aesf.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "base64f.hpp"

std::string aesf::encrypt(const char *orig, const char *password, uint64_t r)
{
    fflassert(orig, to_cstr(orig));
    fflassert(password, to_cstr(password));

    std::string result;
    result.append(orig);
    result.append(password);
    result.append(std::to_string(r));
    result.append(str_printf("%04zu", to_sv(orig).size()));
    return base64f::encode(result);
}

std::string aesf::decrypt(const char *encoded, const char *, uint64_t)
{
    const auto s = base64f::decode(encoded, to_sv(encoded).size());
    const size_t size = (s.rbegin()[3] - '0') * 1000
                      + (s.rbegin()[2] - '0') * 100
                      + (s.rbegin()[1] - '0') * 10
                      + (s.rbegin()[0] - '0') * 1;
    return s.substr(0, size);
}
