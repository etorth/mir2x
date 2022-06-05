// length encoding for mode 1:
//
//  [0 - 254]          : length in 0 ~ 254
//  [    255][0 ~ 255] : length as 0 ~ 255 + 255
//
//  1. most likely we are using 0 ~ 254
//  2. if compressed length more than 254 we need two bytes
//  3. we support range in [0, 255 + 255]

#pragma once
#include <cstdint>
#include <cstddef>
#include "fflerror.hpp"
#include "msgattribute.hpp"

class MsgBase
{
    protected:
        const uint8_t m_headCode;

    protected:
        MsgBase(uint8_t headCode)
            : m_headCode(headCode)
        {}

    public:
        int type() const
        {
            return getAttribute().type;
        }

        size_t dataLen() const
        {
            return getAttribute().dataLen;
        }

        size_t maskLen() const
        {
            if(type() == 1){
                return (dataLen() + 7) / 8;
            }
            return 0;
        }

        const std::string &name() const
        {
            return getAttribute().name;
        }

    private:
        virtual const MsgAttribute &getAttribute() const = 0;

    public:
        bool checkData(const void *data, size_t dataSize) const
        {
            switch(type()){
                case 0:
                    {
                        return (data == nullptr) && (dataSize == 0);
                    }
                case 1:
                case 2:
                    {
                        return data && (dataSize == dataLen());
                    }
                case 3:
                    {
                        if(data){
                            return (dataSize > 0) && (dataSize < 0XFFFFFFFF);
                        }
                        else{
                            return dataSize == 0;
                        }
                    }
                default:
                    {
                        return false;
                    }
            }
        }

        bool checkDataSize(size_t maskSize, size_t bodySize) const
        {
            switch(type()){
                case 0:
                    {
                        return (maskSize == 0) && (bodySize == 0);
                    }
                case 1:
                    {
                        return (maskSize == maskLen()) && (bodySize <= dataLen());
                    }
                case 2:
                    {
                        return (maskSize == 0) && (bodySize == dataLen());
                    }
                case 3:
                    {
                        return (maskSize == 0) && (bodySize <= 0XFFFFFFFF);
                    }
                default:
                    {
                        return false;
                    }
            }
        }
};
