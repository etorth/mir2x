#include <iostream>
#include <string>
#include <memory>
#include "imgf.hpp"
#include "fileptr.hpp"

struct Bitmap
{
    size_t width = 0;
    std::vector<uint32_t> data;

    Bitmap(size_t argW, size_t argH)
        : width(argW)
        , data(argW * argH)
    {}

    Bitmap()
        : Bitmap(0, 0)
    {}

    uint32_t &getPixel(size_t w, size_t h)
    {
        return data[h * width + w];
    }

    void resize(size_t argW, size_t argH)
    {
        width = argW;
        data.resize(argW * argH);
    }

    bool empty() const
    {
        return width == 0;
    }

    void toARGB()
    {
        for(auto &pixel: data){
            pixel = ((pixel & 0xFF00FF00) | ((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16));
        }
    }
};

struct MImageHeader
{
    int16_t width;
    int16_t height;

    int16_t offsetX;
    int16_t offsetY;
    int16_t shadowX;
    int16_t shadowY;

    int32_t length;
    char    shadow;
};

struct MImage
{
    MImageHeader THeader;
    Bitmap Texture;

    MImageHeader OHeader;
    Bitmap Overlay;

    MImage(fileptr_t &fptr)
    {
        readHeader(fptr, THeader);
    }

    void readHeader(fileptr_t &fptr, MImageHeader &header)
    {
        read_fileptr(fptr, &header.width  , 2);
        read_fileptr(fptr, &header.height , 2);
        read_fileptr(fptr, &header.offsetX, 2);
        read_fileptr(fptr, &header.offsetY, 2);
        read_fileptr(fptr, &header.shadowX, 2);
        read_fileptr(fptr, &header.shadowY, 2);

        uint8_t a, b, c;
        read_fileptr(fptr, &a, 1);
        read_fileptr(fptr, &b, 1);
        read_fileptr(fptr, &c, 1);

        header.length = (int32_t)(a) | ((int32_t)(b) << 8) | ((int32_t)(c) << 16);

        read_fileptr(fptr, &header.shadow , 1);
    }

    void DecompressV1Texture(fileptr_t &fptr)
    {
        constexpr int size = 8;
        int blockOffset = 0, dataOffset = 0;
        std::vector<uint8_t> countList;

        int tWidth = 2;
        while (tWidth < THeader.width)
            tWidth *= 2;

        auto _fBytes = read_fileptr<std::vector<uint8_t>>(fptr, THeader.length);

        Texture.resize(THeader.width, THeader.height);

        auto pixels = (uint8_t*)Texture.data.data();

        int cap = THeader.width * THeader.height * 4;
        int currentx = 0;

        while (dataOffset < THeader.length)
        {
            countList.clear();
            for (int i = 0; i < 8; i++)
                countList.push_back(_fBytes[dataOffset++]);

            for (int i = 0; i < (int)countList.size(); i++)
            {
                int count = countList[i];

                if (i % 2 == 0)
                {
                    if (currentx >= tWidth)
                        currentx -= tWidth;

                    for (int off = 0; off < count; off++)
                    {
                        if (currentx < THeader.width)
                            blockOffset++;

                        currentx += 4;

                        if (currentx >= tWidth)
                            currentx -= tWidth;
                    }
                    continue;
                }

                for (int c = 0; c < count; c++)
                {
                    if (dataOffset >= (int)_fBytes.size())
                        break;

                    uint8_t newPixels[64];
                    uint8_t block[8];

                    std::memcpy(block + 0, _fBytes.data() + dataOffset, 8);
                    dataOffset += size;
                    DecompressBlock(newPixels, block);

                    int pixelOffSet = 0;
                    uint8_t sourcePixel[4];

                    for (int py = 0; py < 4; py++)
                    {
                        for (int px = 0; px < 4; px++)
                        {
                            int blockx = blockOffset % (THeader.width / 4);
                            int blocky = blockOffset / (THeader.width / 4);

                            int x = blockx * 4;
                            int y = blocky * 4;

                            int destPixel = ((y + py) * THeader.width) * 4 + (x + px) * 4;

                            std::memcpy(sourcePixel, newPixels + pixelOffSet, 4);
                            pixelOffSet += 4;

                            if (destPixel + 4 > cap)
                                break;
                            for (int pc = 0; pc < 4; pc++)
                                pixels[destPixel + pc] = sourcePixel[pc];
                        }
                    }
                    blockOffset++;
                    if (currentx >= THeader.width)
                        currentx -= THeader.width;
                    currentx += 4;
                }
            }
        }
    }

    void CreateTexture(fileptr_t &fptr)
    {
        std::vector<uint8_t> countList;
        int tWidth = 2;

        while (tWidth < THeader.width)
            tWidth *= 2;

        auto fBytes = read_fileptr<std::vector<uint8_t>>(fptr, THeader.length);

        Texture.resize(THeader.width, THeader.height);

        auto pixels = (uint8_t*)Texture.data.data();

        int blockOffset = 0, dataOffset = 0;

        while (dataOffset < THeader.length)
        {
            countList.clear();
            for (int i = 0; i < 8; i++)
                countList.push_back(fBytes[dataOffset++]);

            for (int i = 0; i < (int)countList.size(); i++)
            {
                int count = countList[i];

                if (i % 2 == 0)
                {
                    blockOffset += count;
                    continue;
                }

                for (int c = 0; c < count; c++)
                {
                    if (dataOffset >= (int)fBytes.size())
                        break;

                    uint8_t block[8];
                    std::memcpy(block + 0, fBytes.data() + dataOffset, 8);
                    dataOffset += 8;

                    uint8_t newPixels[64];
                    DecompressBlock(newPixels, block);

                    int blockx = blockOffset % (tWidth / 4);
                    int blocky = blockOffset / (tWidth / 4);

                    for (int py = 0; py < 4; py++)
                    {
                        int y = blocky * 4 + py;
                        if(y >= THeader.height)
                            break;

                        for (int px = 0; px < 4; px++)
                        {
                            int x = blockx * 4 + px;
                            if(x >= THeader.width)
                                break;

                            std::memcpy(pixels + (y * THeader.width + x) * 4, newPixels + (py * 4 + px) * 4, 4);
                        }
                    }
                    blockOffset++;
                }
            }
        }
    }

    void CreateOverlayTexture(fileptr_t &fptr)
    {
        std::vector<uint8_t> countList;
        int tWidth = 2;

        readHeader(fptr, OHeader);

        while (tWidth < OHeader.width)
            tWidth *= 2;

        auto fBytes = read_fileptr<std::vector<uint8_t>>(fptr, OHeader.length);

        Overlay.resize(OHeader.width, OHeader.height);

        auto pixels = (uint8_t*)Overlay.data.data();
        int cap = OHeader.width * OHeader.height * 4;

        int offset = 0, dataOffset = 0;

        while (dataOffset < (int)fBytes.size())
        {
            countList.clear();
            for (int i = 0; i < 8; i++)
                countList.push_back(fBytes[dataOffset++]);

            for (int i = 0; i < (int)countList.size(); i++)
            {
                int count = countList[i];

                if (i % 2 == 0)
                {
                    offset += count;
                    continue;
                }

                for (int c = 0; c < count; c++)
                {
                    if (dataOffset >= (int)fBytes.size())
                        break;

                    uint8_t newPixels[64];
                    uint8_t block[8];

                    std::memcpy(block + 0, fBytes.data() + dataOffset, 8);
                    dataOffset += 8;
                    DecompressBlock(newPixels, block);

                    int pixelOffSet = 0;
                    uint8_t sourcePixel[4];

                    for (int py = 0; py < 4; py++)
                    {
                        for (int px = 0; px < 4; px++)
                        {
                            int blockx = offset % (tWidth / 4);
                            int blocky = offset / (tWidth / 4);

                            int x = blockx * 4;
                            int y = blocky * 4;

                            int destPixel = ((y + py) * OHeader.width) * 4 + (x + px) * 4;

                            std::memcpy(sourcePixel + 0, newPixels + pixelOffSet, 4);
                            pixelOffSet += 4;

                            if (destPixel + 4 > cap)
                                break;

                            std::memcpy(pixels + destPixel, sourcePixel, 4);
                        }
                    }

                    offset++;
                }
            }
        }
        /*while (dataOffset < fBytes.Length)
         {
             countList.Clear();
             for (int i = 0; i < 8; i++)
                 countList.Add(fBytes[dataOffset++]);

             for (int i = 0; i < countList.Count; i++)
             {
                 int count = countList[i];

                 if (i % 2 == 0)
                 {
                     if (currentx >= tWidth)
                         currentx -= tWidth;

                     for (int off = 0; off < count; off++)
                     {
                         if (currentx < OHeader.width)
                             offset++;

                         currentx += 4;

                         if (currentx >= tWidth)
                             currentx -= tWidth;
                     }
                     continue;
                 }

                 for (int c = 0; c < count; c++)
                 {
                     if (dataOffset >= fBytes.Length)
                         break;

                     auto newPixels = new uint8_t[64];
                     auto block = new uint8_t[size];

                     Array.Copy(fBytes, dataOffset, block, 0, size);
                     dataOffset += size;
                     DecompressBlock(newPixels, block);

                     int pixelOffSet = 0;
                     auto sourcePixel = new uint8_t[4];

                     for (int py = 0; py < 4; py++)
                     {
                         for (int px = 0; px < 4; px++)
                         {
                             int blockx = offset % (OHeader.width / 4);
                             int blocky = offset / (OHeader.width / 4);

                             int x = blockx * 4;
                             int y = blocky * 4;

                             int destPixel = ((y + py) * OHeader.width) * 4 + (x + px) * 4;

                             Array.Copy(newPixels, pixelOffSet, sourcePixel, 0, 4);
                             pixelOffSet += 4;

                             if (destPixel + 4 > cap)
                                 break;
                             for (int pc = 0; pc < 4; pc++)
                                 pixels[destPixel + pc] = sourcePixel[pc];
                         }
                     }
                     offset++;
                     if (currentx >= OHeader.width)
                         currentx -= OHeader.width;
                     currentx += 4;
                 }
             }
         }
         */
    }

    void DecompressBlock(uint8_t *newPixels, uint8_t *block)
    {
        uint8_t colours[8];
        std::memcpy(colours + 0, block + 0, 8);

        uint8_t codes[16];

        int a = Unpack(block, 0, codes, 0);
        int b = Unpack(block, 2, codes, 4);

        for (int i = 0; i < 3; i++)
        {
            int c = codes[i];
            int d = codes[4 + i];

            if (a <= b)
            {
                codes[8 + i] = (uint8_t)((c + d) / 2);
                codes[12 + i] = 0;
            }
            else
            {
                codes[8 + i] = (uint8_t)((2 * c + d) / 3);
                codes[12 + i] = (uint8_t)((c + 2 * d) / 3);
            }
        }

        codes[8 + 3] = 255;
        codes[12 + 3] = (a <= b) ? (uint8_t)0 : (uint8_t)255;
        for (int i = 0; i < 4; i++)
        {
            if ((codes[i * 4] == 0) && (codes[(i * 4) + 1] == 0) && (codes[(i * 4) + 2] == 0) && (codes[(i * 4) + 3] == 255))
            { //dont ever use pure black cause that gives transparency issues
                codes[i * 4] = 1;
                codes[(i * 4) + 1] = 1;
                codes[(i * 4) + 2] = 1;
            }
        }

        uint8_t indices[16];
        for (int i = 0; i < 4; i++)
        {
            uint8_t packed = block[4 + i];

            indices[0 + i * 4] = (uint8_t)(packed & 0x3);
            indices[1 + i * 4] = (uint8_t)((packed >> 2) & 0x3);
            indices[2 + i * 4] = (uint8_t)((packed >> 4) & 0x3);
            indices[3 + i * 4] = (uint8_t)((packed >> 6) & 0x3);
        }

        for (int i = 0; i < 16; i++)
        {
            auto offset = (uint8_t)(4 * indices[i]);
            for (int j = 0; j < 4; j++)
                newPixels[4 * i + j] = codes[offset + j];
        }
    }

    int Unpack(uint8_t *packed, int srcOffset, uint8_t *colour, int dstOffSet)
    {
        int value = packed[0 + srcOffset] | (packed[1 + srcOffset] << 8);

        // get components in the stored range
        auto blue = (uint8_t)((value >> 11) & 0x1F);
        auto green = (uint8_t)((value >> 5) & 0x3F);
        auto red = (uint8_t)(value & 0x1F);

        // Scale up to 8 Bit
        colour[0 + dstOffSet] = (uint8_t)((red << 3) | (red >> 2));
        colour[1 + dstOffSet] = (uint8_t)((green << 2) | (green >> 4));
        colour[2 + dstOffSet] = (uint8_t)((blue << 3) | (blue >> 2));
        colour[3 + dstOffSet] = 255;

        return value;
    }
};

struct WTLLibrary
{
    std::string _fileName;
    fileptr_t _fStream;

    std::vector<std::unique_ptr<MImage>> Images;
    int32_t _count;

    std::vector<int32_t> _indexList;

    WTLLibrary(std::string filename, int index)
        : _fileName(filename)
        , _fStream(make_fileptr(_fileName.c_str(), "rb"))
    {
        seek_fileptr(_fStream, 2, SEEK_SET);
        const auto version = read_fileptr<std::string>(_fStream, 20);
        std::cout << version.c_str() << std::endl;

        seek_fileptr(_fStream, 28, SEEK_SET);
        read_fileptr(_fStream, &_count, 4);

        Images.resize(_count);
        _indexList.resize(_count);

        for (int i = 0; i < _count; i++)
            read_fileptr(_fStream, &_indexList[i], 4);

        if (index >= 0) {
            CheckImage(index);
        }
        else {
            for (int i = 0; i < (int)Images.size(); i++)
                CheckImage(i);
        }
    }

    void CheckImage(int index)
    {
        if (Images.empty() || index < 0 || index >= (int)Images.size() || _indexList[index] <= 0) return;

        if (Images[index] == nullptr)
        {
            seek_fileptr(_fStream, _indexList[index], SEEK_SET);
            Images[index].reset(new MImage(_fStream));
        }

        if (Images[index]->Texture.empty())
        {
            seek_fileptr(_fStream, _indexList[index] + 16, SEEK_SET);
            Images[index]->CreateTexture(_fStream);

            Images[index]->Texture.toARGB();
            imgf::saveImageBuffer(Images[index]->Texture.data.data(), Images[index]->THeader.width, Images[index]->THeader.height, str_printf("%d.png", index).c_str());
        }

        long max = size_fileptr(_fStream);
        for (int i = index + 1; i < (int)Images.size(); i++)
        {
            if (_indexList[i] == 0) continue;

            max = _indexList[i];
            break;
        }

        if (_indexList[index] + 16 + Images[index]->THeader.length < max)
        {
            seek_fileptr(_fStream, _indexList[index] + 16 + Images[index]->THeader.length, SEEK_SET);
            Images[index]->CreateOverlayTexture(_fStream);

            Images[index]->Overlay.toARGB();
            imgf::saveImageBuffer(Images[index]->Overlay.data.data(), Images[index]->OHeader.width, Images[index]->OHeader.height, str_printf("%d_overlay.png", index).c_str());
        }
    }
};

int main(int argc, char **argv)
{
    int index = -1;
    if(argc >= 3){
        index = std::atoi(argv[2]);
    }

    std::cout << "WARNING: still can not decode all WTL images..." << std::endl;

    WTLLibrary lib(argv[1], index);
    std::cout << lib.Images.size() << std::endl;
    return 0;
}
