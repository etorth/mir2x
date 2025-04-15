#pragma once
#include <cstdint>
#include <optional>
#include <exception>
#include "argh.h"
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"

namespace argf
{
    enum
    {
        REQ, OPT, BAN,
    };

    class parser: public argh::parser
    {
        public:
            parser(char *argv[])
                : argh::parser(argv)
            {}

            parser(int argc, char *argv[])
                : argh::parser(argc, argv)
            {}

        public:
            int has_option(const std::string &opt) const
            {
                const auto flag = (bool)((*this)[opt]);
                const auto parm = (bool)((*this)(opt));

                if(flag && parm){
                    return 2;
                }

                if(flag || parm){
                    return 1;
                }
                return 0;
            }

            std::optional<std::string> get_option(const std::string &opt) const
            {
                if(const auto &s = (*this)(opt); s){
                    return s.str();
                }

                if((*this)[opt]){
                    return std::string{};
                }

                return std::nullopt;
            }

            // support
            // --opt      : return true
            // --opt=[xx] : return true with warning message

            bool has_flag(const std::string &opt) const
            {
                if((*this)(opt)){
                    print_message(str_printf("--%s ignores its value: %s", opt.c_str(), (*this)(opt).str().c_str()));
                    return true;
                }
                return (*this)[opt];
            }

            // accept
            // --param     : return default
            // --param=    : return default
            // --param=abc : return abc

            std::string has_param(const std::string &opt) const
            {
                return (*this)(opt).str();
            }

        protected:
            virtual void print_message(const std::string &) const
            {
            }
    };

    template<typename T> std::pair<T, bool> parseInteger(const std::optional<std::string> optStr, const std::string &name, auto fnCheck, T defVal = T{})
    {
        const auto val = [&optStr, &name, &defVal]() -> T
        {
            if(optStr.has_value()){
                try{
                    if constexpr (std::is_unsigned_v<T>){
                        // std::stoull can accept "-1" and returns 18446744073709551615
                        // check_cast can detect this corncase
                        return check_cast<T>(std::stoll(optStr.value()));
                    }
                    else if constexpr (std::is_signed_v<T>){
                        return check_cast<T>(std::stoll(optStr.value()));
                    }
                    else if constexpr (std::is_same_v<T, bool>){
                        return to_parsedbool(optStr.value().c_str());
                    }
                    else if constexpr (std::is_same_v<T, float>){
                        return std::stof(optStr.value());
                    }
                    else if constexpr (std::is_same_v<T, double>){
                        return std::stod(optStr.value());
                    }
                    else if constexpr (std::is_same_v<T, long double>){
                        return std::stold(optStr.value());
                    }
                    else{
                        static_assert(false, "unsupported type");
                    }
                }
                catch(...){}
                throw fflerror("invalid %s value: %s", to_cstr(name), to_cstr(optStr.value()));
            }
            return defVal;
        }();
        return {fnCheck(val), !optStr.has_value()}; // {value, value_is_default}
    }

    constexpr auto checkUserPort = [](int port)
    {
        if(port < 0    ) throw fflerror("invalid netagive port: %d", port);
        if(port < 1024 ) throw fflerror("invalid reserved port: %d", port);
        if(port > 65535) throw fflerror("invalid port: %d", port);
        return port;
    };

    const auto checkPositive = [](int val)
    {
        if(val <= 0) throw fflerror("invalid non-positive value: %d", val);
        return val;
    };

    const auto checkNonNegative = [](int val)
    {
        if(val < 0) throw fflerror("invalid negative value: %d", val);
        return val;
    };

    constexpr auto checkMapID = [](uint32_t mapID)
    {
        if(!DBCOM_MAPRECORD(mapID)) throw fflerror("invalid map id: %llu", to_llu(mapID));
        return mapID;
    };

    constexpr auto checkMapIDString = [](const std::string &mapIDStr) -> uint32_t
    {
        if(const auto mapID = DBCOM_MAPID(to_u8cstr(mapIDStr)); mapID > 0){
            return mapID;
        }
        return argf::parseInteger<uint32_t>(mapIDStr, "mapID", checkMapID).first;
    };
}
