private:
    static int sizeOff(auto && func, int index)
    {
        /**/ if(index <  0) return          0;
        else if(index == 0) return func() / 2;
        else                return func() - 1;
    }

public:
    static int xSizeOff(dir8_t argDir, auto && argFunc)
    {
        switch(argDir){
            case DIR_UPLEFT   : return sizeOff(argFunc, -1);
            case DIR_UP       : return sizeOff(argFunc,  0);
            case DIR_UPRIGHT  : return sizeOff(argFunc,  1);
            case DIR_RIGHT    : return sizeOff(argFunc,  1);
            case DIR_DOWNRIGHT: return sizeOff(argFunc,  1);
            case DIR_DOWN     : return sizeOff(argFunc,  0);
            case DIR_DOWNLEFT : return sizeOff(argFunc, -1);
            case DIR_LEFT     : return sizeOff(argFunc, -1);
            default           : return sizeOff(argFunc,  0);
        }
    }

    static int ySizeOff(dir8_t argDir, auto && argFunc)
    {
        switch(argDir){
            case DIR_UPLEFT   : return sizeOff(argFunc, -1);
            case DIR_UP       : return sizeOff(argFunc, -1);
            case DIR_UPRIGHT  : return sizeOff(argFunc, -1);
            case DIR_RIGHT    : return sizeOff(argFunc,  0);
            case DIR_DOWNRIGHT: return sizeOff(argFunc,  1);
            case DIR_DOWN     : return sizeOff(argFunc,  1);
            case DIR_DOWNLEFT : return sizeOff(argFunc,  1);
            case DIR_LEFT     : return sizeOff(argFunc,  0);
            default           : return sizeOff(argFunc,  0);
        }
    }
