#pragma once

class IApplication {
public:
    virtual ~IApplication() = default;
    virtual void Initialize() = 0;
    virtual void DrawFrame() = 0;
    virtual void Cleanup() = 0;
};