struct ROI final
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    Widget::IntOffset2D offset() const noexcept
    {
        return {x, y};
    }

    Widget::IntSize2D size() const noexcept
    {
        return {std::max<int>(w, 0), std::max<int>(h, 0)};
    }

    bool empty() const noexcept
    {
        return w <= 0 || h <= 0;
    }

    operator bool () const noexcept
    {
        return !empty();
    }

    bool in(int argX, int argY) const noexcept
    {
        return mathf::pointInRectangle<int>(argX, argY, x, y, w, h);
    }

    bool overlap(const Widget::ROI &r) const noexcept
    {
        return mathf::rectangleOverlap<int>(x, y, w, h, r.x, r.y, r.w, r.h);
    }

    Widget::ROI clone() const noexcept
    {
        return *this;
    }

    Widget::ROI & crop(const Widget::ROI &r)
    {
        mathf::cropSegment<int>(x, w, r.x, r.w);
        mathf::cropSegment<int>(y, h, r.y, r.h);
        return *this;
    }
};

class VarROI final
{
    private:
        std::variant<Widget::VarGetter<Widget::ROI>, std::tuple<Widget::VarOffset2D, Widget::VarSize2D>> m_varROI;

    public:
        VarROI()
            : m_varROI(std::tuple<Widget::VarOffset2D, Widget::VarSize2D>{}) // prefer decoupled roi
        {}

    public:
        VarROI(Widget::VarGetter<Widget::ROI> arg)
            : m_varROI(std::in_place_type<Widget::VarGetter<Widget::ROI>>, std::move(arg))
        {}

        VarROI(Widget::VarOffset2D argOff, Widget::VarSize2D argSize)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, std::move(argOff), std::move(argSize))
        {}

        VarROI(Widget::VarSize2D argSize)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, Widget::VarOffset2D{}, std::move(argSize))
        {}

        VarROI(Widget::VarOffset2D argOff, Widget::VarSize argW, Widget::VarSize argH)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, std::move(argOff), Widget::VarSize2D(std::move(argW), std::move(argH)))
        {}

        VarROI(Widget::VarSize argW, Widget::VarSize argH)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, Widget::VarOffset2D{}, Widget::VarSize2D(std::move(argW), std::move(argH)))
        {}

        VarROI(Widget::VarInt argX, Widget::VarInt argY, Widget::VarSize2D argSize)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, Widget::VarOffset2D(std::move(argX), std::move(argY)), std::move(argSize))
        {}

        VarROI(Widget::VarInt argX, Widget::VarInt argY, Widget::VarSize argW, Widget::VarSize argH)
            : m_varROI(std::in_place_type<std::tuple<Widget::VarOffset2D, Widget::VarSize2D>>, Widget::VarOffset2D(std::move(argX), std::move(argY)), Widget::VarSize2D(std::move(argW), std::move(argH)))
        {}

    public:
        Widget::ROI roi(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg);
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    const auto [x, y] = std::get<0>(varg).offset(widget, arg);
                    const auto [w, h] = std::get<1>(varg).  size(widget, arg);

                    return Widget::ROI
                    {
                        .x = x,
                        .y = y,
                        .w = w,
                        .h = h,
                    };
                },
            },

            m_varROI);
        }

        Widget::IntOffset2D offset(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).offset();
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<0>(varg).offset(widget, arg);
                },
            },

            m_varROI);
        }

        Widget::IntSize2D size(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).size();
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<1>(varg).size(widget, arg);
                },
            },

            m_varROI);
        }

    public:
        int x(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).x;
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<0>(varg).x(widget, arg);
                },
            },

            m_varROI);
        }

        int y(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).y;
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<0>(varg).y(widget, arg);
                },
            },

            m_varROI);
        }

        int w(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).w;
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<1>(varg).w(widget, arg);
                },
            },

            m_varROI);
        }

        int h(const Widget *widget, const void *arg = nullptr) const
        {
            return std::visit(VarDispatcher
            {
                [widget, arg](const Widget::VarGetter<Widget::ROI> &varg)
                {
                    return Widget::evalGetter<Widget::ROI>(varg, widget, arg).h;
                },

                [widget, arg](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<1>(varg).h(widget, arg);
                },
            },

            m_varROI);
        }

    public:
        bool combinedOffset() const
        {
            return std::visit(VarDispatcher
            {
                [](const Widget::VarGetter<Widget::ROI> &)
                {
                    return true;
                },

                [](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<0>(varg).combined();
                },
            },

            m_varROI);
        }

        bool combinedSize() const
        {
            return std::visit(VarDispatcher
            {
                [](const Widget::VarGetter<Widget::ROI> &)
                {
                    return true;
                },

                [](const std::tuple<Widget::VarOffset2D, Widget::VarSize2D> &varg)
                {
                    return std::get<1>(varg).combined();
                },
            },

            m_varROI);
        }

        bool combinedROI() const
        {
            return std::holds_alternative<Widget::VarGetter<Widget::ROI>>(m_varROI);
        }
};

class VarROIOpt final
{
    private:
        std::optional<Widget::VarROI> m_varROIOpt;

    public:
        VarROIOpt() = default;
        VarROIOpt(std::nullopt_t): VarROIOpt() {};

    public:
        template<typename... Args> explicit VarROIOpt(Args&&... args)
            : m_varROIOpt(Widget::VarROI(std::forward<Args>(args)...))
        {}

    public:
        VarROIOpt(const Widget::ROI &r)
            : VarROIOpt(r.x, r.y, r.w, r.h)
        {}

        VarROIOpt(const Widget::VarROI &vr)
            : m_varROIOpt(vr)
        {}

    public:
        auto operator -> (this auto && self)
        {
            return std::addressof(self.m_varROIOpt.value());
        }

    public:
        bool has_value() const
        {
            return m_varROIOpt.has_value();
        }

        decltype(auto) value(this auto && self)
        {
            return self.m_varROIOpt.value();
        }

        Widget::VarROI value_or(Widget::VarROI r) const
        {
            return m_varROIOpt.value_or(r);
        }
};

class ROIOpt final
{
    private:
        std::optional<Widget::ROI> m_roiOpt;

    public:
        ROIOpt() = default;
        ROIOpt(std::nullopt_t): ROIOpt() {};

    public:
        ROIOpt(const Widget::ROI &roi)
            : m_roiOpt(roi)
        {}

    public:
        ROIOpt(int argX, int argY, int argW, int argH)
            : m_roiOpt(Widget::ROI{argX, argY, argW, argH})
        {}

    public:
        ROIOpt(int argW, int argH)
            : ROIOpt(0, 0, argW, argH)
        {}

        ROIOpt(Widget::IntSize2D size)
            : ROIOpt(0, 0, size.w, size.h)
        {}

        ROIOpt(Widget::IntOffset2D offset, Widget::IntSize2D size)
            : ROIOpt(offset.x, offset.y, size.w, size.h)
        {}

    public:
        auto operator -> (this auto && self)
        {
            return std::addressof(self.m_roiOpt.value());
        }

    public:
        bool has_value() const
        {
            return m_roiOpt.has_value();
        }

        decltype(auto) value(this auto && self)
        {
            return self.m_roiOpt.value();
        }

        Widget::ROI value_or(Widget::ROI r) const
        {
            return m_roiOpt.value_or(r);
        }
};

struct ROIMap final
{
    dir8_t dir = DIR_UPLEFT;

    int x = 0;
    int y = 0;

    Widget::ROIOpt ro = std::nullopt;

    bool empty() const
    {
        if(ro.has_value()){
            return ro->empty();
        }
        else{
            throw fflerror("ro empty");
        }
    }

    operator bool () const
    {
        return !empty();
    }

    bool in(int pixelX, int pixelY) const
    {
        if(ro.has_value()){
            return Widget::ROI{x, y, ro->w, ro->h}.in(pixelX, pixelY);
        }
        else{
            throw fflerror("ro empty");
        }
    }

    template<typename T> bool in(const T &t) const
    {
        const auto [tx, ty] = t; return in(tx, ty);
    }

    Widget::ROIMap clone() const
    {
        return *this;
    }

    Widget::ROIMap & calibrate(const Widget *widget)
    {
        if(!ro.has_value()){
            if(widget){
                ro = widget->roi();
            }
            else{
                throw fflerror("invalid widget");
            }
        }

        if(dir != DIR_UPLEFT){
            x  -= xSizeOff(dir, ro->w);
            y  -= ySizeOff(dir, ro->h);
            dir = DIR_UPLEFT;
        }

        if(widget){
            crop(widget->roi());
        }

        if(x < 0){
            ro->x -= x;
            ro->w  = std::max<int>(ro->w + x, 0);
            x = 0;
        }

        if(y < 0){
            ro->y -= y;
            ro->h  = std::max<int>(ro->h + y, 0);
            y = 0;
        }

        return *this;
    }

    Widget::ROIMap & crop(const Widget::ROI &r)
    {
        if(!ro.has_value()){
            throw fflerror("ro empty");
        }

        if(dir != DIR_UPLEFT){
            x  -= xSizeOff(dir, ro->w);
            y  -= ySizeOff(dir, ro->h);
            dir = DIR_UPLEFT;
        }

        const auto oldX = ro->x;
        const auto oldY = ro->y;

        ro->crop(r);

        x += (ro->x - oldX);
        y += (ro->y - oldY);

        return *this;
    }

    Widget::ROIMap map(int dx, int dy, const Widget::ROI &cr) const
    {
        // maps from parent's m to child's cm
        // cr is child's cropped ROI in itself, child's (0, 0) is at (dx, dy) in parent

        auto cm = clone().crop(Widget::ROI
        {
            .x = cr.x + dx,
            .y = cr.y + dy,
            .w = cr.w,
            .h = cr.h,
        });

        cm.ro->x -= dx;
        cm.ro->y -= dy;

        return cm;
    }

    Widget::ROIMap create(const Widget::ROI &cr) const
    {
        // maps from parent's m to child's cm
        // cr is child's full ROI in parent, child's (0, 0) is at (cr.x, cr.y) in parent

        return map(cr.x, cr.y, Widget::ROI
        {
            .x = 0,
            .y = 0,
            .w = cr.w,
            .h = cr.h,
        });
    }
};

template<typename T> Widget::ROI makeROI(const T &t)
{
    const auto [x, y, w, h] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename U, typename V> Widget::ROI makeROI(const U &u, const V &v)
{
    const auto [x, y] = u;
    const auto [w, h] = v; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename T> Widget::ROI makeROI(int x, int y, const T &t)
{
    const auto [w, h] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename T> Widget::ROI makeROI(const T &t, int w, int h)
{
    const auto [x, y] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}
