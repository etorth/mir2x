#pragma once
#include <cstdint>
#include <optional>
#include "argh.h"
#include "strf.hpp"
#include "fflerror.hpp"

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
}
