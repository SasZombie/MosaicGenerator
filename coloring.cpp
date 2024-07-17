#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

bool acceptableColorRange(unsigned char value, int targetValue)
{
    const int range = 20;

    if(targetValue == value)
        return true;

    if(value > targetValue - range && value < targetValue + range)
        return true;

    return false;
}

bool notOnGlasses(int index, unsigned char* resizedImage)
{            
    const int ns = 225;
    if((resizedImage[index] > ns && resizedImage[index + 1] > ns && resizedImage[index + 2] > ns) || (acceptableColorRange(resizedImage[index], 79) && acceptableColorRange(resizedImage[index + 1], 125) && acceptableColorRange(resizedImage[index + 2], 161))||(acceptableColorRange(resizedImage[index], 149) && acceptableColorRange(resizedImage[index + 1], 202) && acceptableColorRange(resizedImage[index + 2], 220)))
        return false;
    return true;
}


int main()
{
    int width, height, channels;
    unsigned char *image = stbi_load("images/White.png", &width, &height, &channels, 0);

    if (image == nullptr)
    {
        std::cerr << "Failed to load image\n";
        return -1;
    }

    int newWidth = 30;
    int newHeight = 30;

    unsigned char *resizedImage = new unsigned char[newWidth * newHeight * channels];

    if (!stbir_resize_uint8(image, width, height, 0, resizedImage, newWidth, newHeight, 0, channels))
    {
        std::cerr << "Failed to resize image\n";
        delete[] resizedImage;
        stbi_image_free(image);
        return -1;
    }

    const int size = 70;

    for (int x = 0; x < newHeight; ++x)
    {
        for (int y = 0; y < newWidth; ++y)
        {
            const int index = (x * newWidth + y) * channels;
            const int ns = 25;

            if (notOnGlasses(index, resizedImage) && (resizedImage[index + 0] > ns && resizedImage[index + 1] > ns && resizedImage[index + 2] > ns))
            {
                resizedImage[index + 0] = 255;
                resizedImage[index + 1] =  0;
                resizedImage[index + 2] =  0;
            }
        }
    }

    if (!stbi_write_png("resized_image.png", newWidth, newHeight, channels, resizedImage, newWidth * channels))
    {
        std::cerr << "Failed to save resized image\n";
        delete[] resizedImage;
        stbi_image_free(image);
        return -1;
    }

    delete[] resizedImage;
    stbi_image_free(image);
}
