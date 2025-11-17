#pragma once

#include <string>

class Texture
{
public:
    Texture();
    ~Texture();

    bool Load(const std::string& filename);
    void Unload();
    void SetActive();

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    unsigned int texture_id_;
    int width_;
    int height_;
};
