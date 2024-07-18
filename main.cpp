#include <iostream>
#include <tuple>
#include <cstring>
#include <unordered_map>
#include "raylib/raylib-cpp/include/raylib-cpp.hpp"


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

void makeImageMossaic(int height, int width, int factor, int channels, const unsigned char *image, unsigned char *newImage, const std::string_view pathOfComponent) noexcept
{

    int componentWidth, componentHeight, componentChanneles;
    unsigned char *componentImage = stbi_load(pathOfComponent.data(), &componentWidth, &componentHeight, &componentChanneles, 0);

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

        }
    }

    stbi_image_free(componentImage);
    delete[] resizedImage;
}

enum class typeOfMossaic{
    classic, picture
};

int main()
{
    constexpr size_t WIDTH = 600, HEIGHT = 800;
    constexpr size_t areaWidth = 100, areaHight = 25;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    raylib::Window window(WIDTH, HEIGHT, "Test");
    raylib::Font font = LoadFont("fonts/Rubik/Rubik-VariableFont_wght.ttf");

    window.SetTargetFPS(60);

    raylib::Texture2D texture;
    float scale = 3.f;
    bool textureLoaded = false;
    bool exporting = false;

    constexpr size_t xPad = 350, yPad = 60;
    const raylib::Rectangle button{WIDTH / 2 - 150 / 2, HEIGHT - yPad, 150, 50};
    std::string writting = "Next";
    std::string text = "Drop an image to transform";
    FilePathList paths;
    std::string imagePaths[2];
    size_t position = 0;

    bool classic = false;

    while (!window.ShouldClose())
    {
        window.BeginDrawing();
        window.ClearBackground(BLACK);

        if (IsFileDropped())
        {
            int fileCount;
            paths = LoadDroppedFiles();

            if (paths.count > 0)
            {
                if (textureLoaded)
                {
                    texture.Unload();
                }

                texture.Load(paths.paths[0]);
                float scaleX = (float)WIDTH / texture.width;
                float scaleY = (float)HEIGHT / texture.height;
                scale = (scaleX < scaleY) ? scaleX : scaleY;
                textureLoaded = true;
                imagePaths[position] = paths.paths[0];

                UnloadDroppedFiles(paths);
            }
        }

        if (IsKeyPressed(KEY_E))
        {
            classic = !classic;
        }

        if (CheckCollisionPointRec(GetMousePosition(), button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && textureLoaded)
        {
            writting = "Export";
            textureLoaded = false;
            texture.Unload();
            text = "Drop an image to transform with";
            ++position;

            if(position == 2)
            {
                position = 0;
                exporting = true;
                writting = "Next";
            }
        }

        if (exporting)
        {
            constexpr int factor = 30;
            int width, height, channels;
            unsigned char *image = stbi_load(imagePaths[0].c_str(), &width, &height, &channels, 0);

            if (image == nullptr)
            {
                std::cerr << "Failed to load image\n";
                return -1;
            }

            unsigned char *newImage = new unsigned char[width * height * channels];

            if(classic)
                makeClassicMossaic(height, width, factor, channels, image, newImage);
            else
                makeImageMossaic(height, width, factor, channels, image, newImage, imagePaths[1]);

            if (!stbi_write_png("mosaic.png", width, height, channels, newImage, width * channels))
            {
                std::cerr << "Failed to save new image\n";
                delete[] newImage;
                stbi_image_free(image);
                return -1;
            }

            delete[] newImage;
            stbi_image_free(image);
            exporting = false;
        }
        else if (textureLoaded)
            texture.Draw(raylib::Vector2{(WIDTH - texture.width * scale) / 2, (HEIGHT - texture.height * scale) / 2}, 0.0f, scale);
        else
            raylib::DrawTextEx(font, text + static_cast<char>(classic), raylib::Vector2{WIDTH / 2 - 250, HEIGHT / 2}, 40, 0.5, WHITE);

        button.Draw(RED);
        raylib::DrawTextEx(font, writting, button.GetPosition(), 30, 0.f, WHITE);

        window.EndDrawing();
    }
}