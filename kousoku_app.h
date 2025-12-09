#pragma once

class IKousokuApp {
public:
    virtual ~IKousokuApp() = default;
    virtual void Initialize() = 0;
    virtual void DrawFrame() = 0;
    virtual void Cleanup() = 0;
};
