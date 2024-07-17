#include <iostream>
#include <tuple>
#include <cstring>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

static std::tuple<int, int, int> getAverageColor(int factor, const unsigned char *imageSlice, int width, int channels) noexcept;
static void makeClassicMossaic(int height, int width, int factor, int channels, const unsigned char *image, unsigned char *newImage) noexcept;

bool acceptableColorRange(unsigned char value, int targetValue)
{
    const int range = 20;

    if (targetValue == value)
        return true;

    if (value > targetValue - range && value < targetValue + range)
        return true;

    return false;
}

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

bool notOnGlasses(int index, unsigned char *resizedImage)
{
    // const int ns = 225;
    // if ((resizedImage[index] > ns && resizedImage[index + 1] > ns && resizedImage[index + 2] > ns) || (acceptableColorRange(resizedImage[index], 79) && acceptableColorRange(resizedImage[index + 1], 125) && acceptableColorRange(resizedImage[index + 2], 161)) || (acceptableColorRange(resizedImage[index], 149) && acceptableColorRange(resizedImage[index + 1], 202) && acceptableColorRange(resizedImage[index + 2], 220)))
    //     return false;
    return true;
}

unsigned char *loadAndResize(const char *path, int factor) noexcept
{
    int imgWidth, imgHeight, imgChannels;
    unsigned char *image = stbi_load("images/AmongUsPng/Black.png", &imgWidth, &imgHeight, &imgChannels, 0);

    if (image == nullptr)
    {
        std::cerr << "Cannot load image: " << path << '\n';
        std::exit(-1);
    }

    int newWidth = factor / imgWidth, newHeight = factor / imgHeight;

    unsigned char *resizedImage = new unsigned char[newWidth * newHeight * imgChannels];

    if (!stbir_resize_uint8(image, imgWidth, imgHeight, 0, resizedImage, newWidth, newHeight, 0, imgChannels))
    {
        std::cerr << "Failed to resize image\n";
        delete[] resizedImage;
        stbi_image_free(image);
        exit(-1);
    }

    stbi_image_free(image);

    return resizedImage;
}

void colorImage(int R, int G, int B, int A, unsigned char *image, int width, int height, int channels) noexcept
{
    for (int x = 0; x < height; ++x)
    {
        for (int y = 0; y < width; ++y)
        {
            const int index = (x * width + y) * channels;
            const int ns = 25;

            if (notOnGlasses(index, image) && (image[index + 0] > ns && image[index + 1] > ns && image[index + 2] > ns))
            {
                image[index + 0] = R;
                image[index + 1] = G;
                image[index + 2] = B;
                if (channels > 3)
                    image[index + 3] = A;
            }
        }
    }
}

void makeClassicMossaic(int height, int width, int factor, int channels, const unsigned char *image, unsigned char *newImage) noexcept
{
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
}

void makeImageMossaic(int height, int width, int factor, int channels, const unsigned char *image, unsigned char *newImage) noexcept
{

    int componentWidth, componentHeight, componentChanneles;
    unsigned char *componentImage = stbi_load("images/White.png", &componentWidth, &componentHeight, &componentChanneles, 0);

    constexpr int newWidth = 30;
    constexpr int newHeight = 30;
    const int size = newWidth * newHeight * componentChanneles;
    unsigned char *resizedImage = new unsigned char[size];

    if (!stbir_resize_uint8(componentImage, componentWidth, componentHeight, 0, resizedImage, newWidth, newHeight, 0, componentChanneles))
    {
        std::cerr << "Failed to resize image\n";
        delete[] resizedImage;
        stbi_image_free(newImage);
        exit(-1);
    }

    unsigned char* copyResized = new unsigned char[size];

    std::memcpy(copyResized, resizedImage, size);

    for (int y = 0; y < height; y = y + factor)
    {
        for (int x = 0; x < width; x = x + factor)
        {
            int offset = (y * width + x) * channels;
            auto [R, G, B] = getAverageColor(factor, image + offset, width, channels);
            colorImage(R, G, B, 255, resizedImage, newWidth, newHeight, componentChanneles);

            for (int i = 0; i < factor; ++i)
            {
                for (int j = 0; j < factor; ++j)
                {
                    if (y + i < height && x + j < width)
                    {
                        int newIndex = ((y + i) * width + (x + j)) * channels;
                        int srcIndex = (i * factor + j) * componentChanneles;
                        newImage[newIndex + 0] = resizedImage[srcIndex + 0];
                        newImage[newIndex + 1] = resizedImage[srcIndex + 1];
                        newImage[newIndex + 2] = resizedImage[srcIndex + 2];
                    }
                }
            }

            std::memcpy(copyResized, resizedImage, size);
        }
    }

    stbi_image_free(componentImage);
    delete[] resizedImage;
    delete[] copyResized;
}

int main(int argc, char const **argv)
{

    std::string path;

    if (argc == 1)
    {
        path = "images/anime.jpg";
    }
    else
    {
        path = argv[1];
    }

    constexpr int factor = 30;
    int width, height, channels;
    unsigned char *image = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (image == nullptr)
    {
        std::cerr << "Failed to load image\n";
        return -1;
    }

    unsigned char *newImage = new unsigned char[width * height * channels];

    // makeClassicMossaic(height, width, factor, channels, image, newImage);
    makeImageMossaic(height, width, factor, channels, image, newImage);

    if (!stbi_write_png("mosaic.png", width, height, channels, newImage, width * channels))
    {
        std::cerr << "Failed to save new image\n";
        delete[] newImage;
        stbi_image_free(image);
        return -1;
    }

    delete[] newImage;
    stbi_image_free(image);
}
