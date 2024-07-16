#include <iostream>
#include <tuple>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static std::tuple<int, int, int> getAverageColor(int factor, const unsigned char *imageSlice, int width, int channels) noexcept;

std::tuple<int, int, int> getAverageColor(int factor, const unsigned char *imageSlice, int width, int channels) noexcept
{
    int averageR = 0, averageG = 0, averageB = 0;
    int pixelCount = factor * factor;

    for (int i = 0; i < factor; ++i)
    {
        for (int j = 0; j < factor; ++j)
        {
            int index = (i * width + j) * channels;
            averageR = averageR + static_cast<int>(imageSlice[index + 0]);
            averageG = averageG + static_cast<int>(imageSlice[index + 1]);
            averageB = averageB + static_cast<int>(imageSlice[index + 2]);
        }
    }
    return std::make_tuple(averageR / pixelCount, averageG / pixelCount, averageB / pixelCount);
}

int main()
{
    constexpr int factor = 10;

    int width, height, channels;

    unsigned char *image = stbi_load("pfp.jpg", &width, &height, &channels, 0);

    if (image == nullptr)
    {
        std::cerr << "Failed to load image\n";
        return -1;
    }

    unsigned char *newImage = new unsigned char[width * height * channels];


    for (int y = 0; y < height; y = y + factor)
    {
        for (int x = 0; x < width; x = x + factor)
        {
            int offset = (y * width + x) * channels;
            auto [R, G, B] = getAverageColor(factor, image + offset, width, channels);

            for (int i = 0; i < factor; ++i)
            {
                for (int j = 0; j < factor; ++j)
                {
                    if (y + i < height && x + j < width)
                    {
                        int newIndex = ((y + i) * width + (x + j)) * channels;
                        newImage[newIndex + 0] = R;
                        newImage[newIndex + 1] = G;
                        newImage[newIndex + 2] = B;
                    }
                }
            }
        }
    }

    if (!stbi_write_png("mosaic.png", width, height, channels, newImage, width * channels))
    {
        std::cerr << "Failed to save new image\n";
        delete[] newImage;
        stbi_image_free(image);
        return -1;
    }

    delete[] newImage;
    stbi_image_free(image);

    return 0;
}
