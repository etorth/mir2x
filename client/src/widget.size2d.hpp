public:
    struct IntSize2D final
    {
        int w = 0;
        int h = 0;
    };

    class VarSize2D final
    {
        private:
            std::variant<Widget::VarGetter<Widget::IntSize2D>, std::tuple<Widget::VarSize, Widget::VarSize>> m_varSize;

        public:
            VarSize2D()
                : m_varSize(std::make_tuple(0, 0)) // prefer decoupled size
            {}

            VarSize2D(Widget::VarGetter<Widget::IntSize2D> arg)
                : m_varSize(std::in_place_type<Widget::VarGetter<Widget::IntSize2D>>, std::move(arg))
            {}

            VarSize2D(Widget::VarSize arg1, Widget::VarSize arg2)
                : m_varSize(std::in_place_type<std::tuple<Widget::VarSize, Widget::VarSize>>, std::move(arg1), std::move(arg2))
            {}

        public:
            int w(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntSize2D> &varg)
                    {
                        return std::max<int>(Widget::evalGetter<Widget::IntSize2D>(varg, widget, arg).w, 0);
                    },

                    [widget, arg](const std::tuple<Widget::VarSize, Widget::VarSize> &varg)
                    {
                        return Widget::evalSize(std::get<0>(varg), widget, arg);
                    },
                },

                m_varSize);
            }

            int h(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntSize2D> &varg)
                    {
                        return std::max<int>(Widget::evalGetter<Widget::IntSize2D>(varg, widget, arg).h, 0);
                    },

                    [widget, arg](const std::tuple<Widget::VarSize, Widget::VarSize> &varg)
                    {
                        return Widget::evalSize(std::get<1>(varg), widget, arg);
                    },
                },

                m_varSize);
            }

        public:
            Widget::IntSize2D size(const Widget *widget, const void * arg = nullptr) const
            {
                return std::visit(VarDispatcher
                {
                    [widget, arg](const Widget::VarGetter<Widget::IntSize2D> &varg)
                    {
                        const auto [w, h] = Widget::evalGetter<Widget::IntSize2D>(varg, widget, arg);
                        return Widget::IntSize2D
                        {
                            .w = std::max<int>(w, 0),
                            .h = std::max<int>(h, 0),
                        };
                    },

                    [widget, arg](const std::tuple<Widget::VarSize, Widget::VarSize> &varg)
                    {
                        return Widget::IntSize2D
                        {
                            .w = Widget::evalSize(std::get<0>(varg), widget, arg),
                            .h = Widget::evalSize(std::get<1>(varg), widget, arg),
                        };
                    },
                },

                m_varSize);
            }

        public:
            bool combined() const
            {
                return std::holds_alternative<Widget::VarGetter<Widget::IntSize2D>>(m_varSize);
            }
    };
