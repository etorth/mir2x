private:
    // don't use std::string_view, use const char *
    // both std::string_view and std::string can be constructed from string literal
    // which causes ambiguity if assigned from string literal

    // no need to add std::nullptr_t
    // because const char * is nullable

    using VarStrHelper = std::variant<const char *, // not owning, nullable,
                                      std::string>; //     owning

public:
    class VarStr: public VarStrHelper
    {
        public:
            using VarStrHelper::VarStrHelper;

        public:
            const char *c_str() const
            {
                return std::visit(VarDispatcher
                {
                    [](const        char *varg){ return varg ? varg : ""; },
                    [](const std::string &varg){ return varg.c_str()    ; },
                },

                *this);
            }

        public:
            bool empty() const
            {
                return c_str()[0] == '\0';
            }

        public:
            size_t size() const
            {
                return std::visit(VarDispatcher
                {
                    [](const        char *varg){ return varg ? std::strlen(varg) : 0; },
                    [](const std::string &varg){ return varg.size()                 ; },
                },

                *this);
            }

        public:
            std::string str() &
            {
                return std::string(c_str());
            }

            std::string str() &&
            {
                if(auto sptr = std::get_if<std::string>(this)){
                    return std::move(*sptr);
                }
                else{
                    return std::string(c_str());
                }
            }
    };

public:
    using VarStrFunc = std::variant<

            // no need of std::nullptr_t
            // because const char * is nullable here

            const char *,  // direct value, not owning, nullable
            std::string,   // direct value,     owning

            std::function<Widget::VarStr()>,
            std::function<Widget::VarStr(const Widget *)>,
            std::function<Widget::VarStr(const Widget *, const void *)>>;

public:
    static Widget::VarStr evalStrFunc(const Widget::VarStrFunc &, const Widget *, const void * = nullptr);
