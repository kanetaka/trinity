#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include <GL/glew.h>
#include <SDL/SDL.h>
#include <stb_image.h>

#include <iostream>

Texture::Texture() :
    texture_id_(0),
    width_(0),
    height_(0)
{
}

Texture::~Texture()
{
}

bool Texture::Load(const std::string& filename)
{
    int channels = 0;
    unsigned char* image = stbi_load(filename.c_str(), &width_, &height_, &channels, 0);

    if (image == nullptr) {
        SDL_Log("画像の読み込みに失敗しました %s", filename.c_str());
        return false;
    }

    int format = GL_RGB;
    if (channels == 4) {
        format = GL_RGBA;
    }

    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);

    return true;
}

void Texture::Unload()
{
    glDeleteTextures(1, &texture_id_);
}

void Texture::SetActive()
{
    glBindTexture(GL_TEXTURE_2D, texture_id_);
}
