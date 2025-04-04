#pragma once
#include <cstdint>
#include <optional>
#include "argh.h"
#include "strf.hpp"
#include "fflerror.hpp"

namespace argf
{
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

    inline std::optional<uint32_t> getPort(const argf::parser &parser, const std::string &portName)
    {
        if(const auto val = parser(portName).str(); !val.empty()){
            try{
                if(const auto port = std::stoi(val); port >= 0){
                    return static_cast<uint32_t>(port);
                }
            }
            catch(...){}
            throw fflerror("invalid port: %s", val.c_str());
        }
        return std::nullopt;
    }
}
