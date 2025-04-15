#pragma once
#include <cctype>
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

    struct defVal
    {
        constexpr static int     clientPort = 6000; // open to players
        constexpr static int masterPeerPort = 7000; // open to slave servers
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

    namespace _details
    {
        inline std::string strAppendSpace(const char *s)
        {
            if(str_haschar(s)){
                if(std::isspace(static_cast<unsigned char>(*(to_sv(s).rbegin())))){
                    return s;
                }
                else{
                    return str_printf("%s ", s);
                }
            }
            else{
                return {};
            }
        }

        inline std::string strAppendSpace(const std::string &s)
        {
            return _details::strAppendSpace(s.c_str());
        }
    }

    template<typename T> std::pair<T, bool> parseInteger(const std::optional<std::string> optStr, const char *name, auto fnCheck, T defVal = T{}, std::optional<T> defValOnEmpty = std::nullopt)
    {
        const auto val = [&]() -> T
        {
            if(optStr.has_value()){
                try{
                    if(str_haschar(optStr.value())){
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
                    else if(defValOnEmpty.has_value()){
                        return defValOnEmpty.value();
                    }
                }
                catch(...){}
                throw fflerror("invalid %svalue: %s", _details::strAppendSpace(name).c_str(), to_cstr(optStr.value()));
            }
            return defVal;
        }();

        return {fnCheck(name, val), !optStr.has_value()}; // {value, value_is_default}
    }

    template<typename T> T checkPass(const char *, T val)
    {
        return std::move(val);
    }

    constexpr auto checkPort = [](const char *name, int port)
    {
        if(port < 0    ) throw fflerror("invalid %snetagive port: %d", _details::strAppendSpace(name).c_str(), port);
        if(port > 65535) throw fflerror("invalid %sport: %d",          _details::strAppendSpace(name).c_str(), port);
        return port;
    };

    constexpr auto checkUserListenPort(bool allowZeroPort = true)
    {
        return [allowZeroPort](const char *name, int port)
        {
            argf::checkPort(name, port);
            if(port == 0){
                if(allowZeroPort){
                    return 0;
                }
                else{
                    throw fflerror("invalid %szero port: %d", _details::strAppendSpace(name).c_str(), port);
                }
            }
            else if(port < 1024){
                throw fflerror("invalid %sreserved port: %d", _details::strAppendSpace(name).c_str(), port);
            }
            else{
                return port;
            }
        };
    }

    const auto checkPositive = [](const char *name, int val)
    {
        if(val <= 0) throw fflerror("invalid %snon-positive value: %d", _details::strAppendSpace(name).c_str(), val);
        return val;
    };

    const auto checkNonNegative = [](const char *name, int val)
    {
        if(val < 0) throw fflerror("invalid %snegative value: %d", _details::strAppendSpace(name).c_str(), val);
        return val;
    };

    constexpr auto checkMapID = [](const char *name, uint32_t mapID)
    {
        if(!DBCOM_MAPRECORD(mapID)) throw fflerror("invalid %smap id: %llu", _details::strAppendSpace(name).c_str(), to_llu(mapID));
        return mapID;
    };

    inline std::pair<int, bool> parseMapIDString(std::optional<std::string> mapIDOptStr, const char *name, bool allowZeroMapID, int defVal = 0, std::optional<int> defValOnEmpty = std::nullopt)
    {
        const auto fnCheckMapID = [&](int mapID)
        {
            if(mapID < 0){
                throw fflerror("invalid %snegative map id: %d", _details::strAppendSpace(name).c_str(), mapID);
            }

            if(mapID == 0){
                if(allowZeroMapID){
                    return 0;
                }
                else{
                    throw fflerror("invalid %szero map id: %d", _details::strAppendSpace(name).c_str(), mapID);
                }
            }

            if(DBCOM_MAPRECORD(mapID)){
                return mapID;
            }

            throw fflerror("invalid %smap id: %d", _details::strAppendSpace(name).c_str(), mapID);
        };

        const auto parsedMapID = [&]() -> int
        {
            if(!mapIDOptStr.has_value()){
                return defVal;
            }

            if(!str_haschar(mapIDOptStr.value())){
                if(defValOnEmpty.has_value()){
                    return defValOnEmpty.value();
                }
                throw fflerror("invalid %svalue: %s", _details::strAppendSpace(name).c_str(), to_cstr(mapIDOptStr.value()));
            }

            if(const auto mapID = DBCOM_MAPID(to_u8cstr(mapIDOptStr.value())); mapID > 0){
                return mapID;
            }

            try{
                return std::stol(mapIDOptStr.value());
            }

            catch(...){}
            throw fflerror("failed to parse %svalue: %s", _details::strAppendSpace(name).c_str(), to_cstr(mapIDOptStr.value()));
        }();

        return {fnCheckMapID(parsedMapID), !mapIDOptStr.has_value()}; // {value, value_is_default}
    }
}
