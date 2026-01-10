public:
    struct IntOffset2D final
    {
        int x = 0;
        int y = 0;
    };

    class VarOffset2D final
    {
        private:
            std::variant<Widget::VarGetter<Widget::IntOffset2D>, std::tuple<Widget::VarInt, Widget::VarInt>> m_varOffset;

        public:
            VarOffset2D()
                : m_varOffset(std::make_tuple(0, 0)) // prefer decoupled offset
            {}

            VarOffset2D(Widget::VarGetter<Widget::IntOffset2D> arg)
                : m_varOffset(std::in_place_type<Widget::VarGetter<Widget::IntOffset2D>>, std::move(arg))
            {}

            VarOffset2D(Widget::VarInt arg1, Widget::VarInt arg2)
                : m_varOffset(std::in_place_type<std::tuple<Widget::VarInt, Widget::VarInt>>, std::move(arg1), std::move(arg2))
            {}

        public:
            int x(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntOffset2D> &varg)
                    {
                        return Widget::evalGetter<Widget::IntOffset2D>(varg, widget, arg).x;
                    },

                    [widget, arg](const std::tuple<Widget::VarInt, Widget::VarInt> &varg)
                    {
                        return Widget::evalInt(std::get<0>(varg), widget, arg);
                    },
                },

                m_varOffset);
            }

            int y(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntOffset2D> &varg)
                    {
                        return Widget::evalGetter<Widget::IntOffset2D>(varg, widget, arg).y;
                    },

                    [widget, arg](const std::tuple<Widget::VarInt, Widget::VarInt> &varg)
                    {
                        return Widget::evalInt(std::get<1>(varg), widget, arg);
                    },
                },

                m_varOffset);
            }

        public:
            Widget::IntOffset2D offset(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntOffset2D> &varg)
                    {
                        return Widget::evalGetter<Widget::IntOffset2D>(varg, widget, arg);
                    },

                    [widget, arg](const std::tuple<Widget::VarInt, Widget::VarInt> &varg)
                    {
                        return Widget::IntOffset2D
                        {
                            .x = Widget::evalInt(std::get<0>(varg), widget, arg),
                            .y = Widget::evalInt(std::get<1>(varg), widget, arg),
                        };
                    },
                },

                m_varOffset);
            }

        public:
            bool combined() const
            {
                return std::holds_alternative<Widget::VarGetter<Widget::IntOffset2D>>(m_varOffset);
            }
    };
