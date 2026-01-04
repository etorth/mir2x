private:
    struct InitArgs final
    {
        Widget::VarDir dir = DIR_UPLEFT;

        Widget::VarInt x = 0;
        Widget::VarInt y = 0;

        Widget::VarSizeOpt fixed = std::nullopt; // the other side to flexible edge

        bool v = true;
        ItemAlign align = ItemAlign::UPLEFT;

        Widget::VarSize itemSpace = 0;

        std::initializer_list<std::pair<Widget *, bool>> childList {};
        Widget::WADPair parent {};
    };

public:
    void    addItem(Widget *, bool);
    void removeItem(uint64_t, bool);

public:
    bool hasShowItem() const;
    void flipItemShow(uint64_t);

public:
    void buildLayout(); // empty function

public:
    inline void clearItem(std::invocable<const Widget *, bool> auto);
    inline void clearItem();

public:
    inline auto foreachItem(this auto &&, bool, auto);
    inline auto foreachItem(this auto &&,       auto);

protected:
    void afterResizeDefault() override
    {
        buildLayout();
    }
