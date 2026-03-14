#pragma once

class ITrinityApp {
public:
    virtual ~ITrinityApp() = default;
    virtual void Initialize() = 0;
    virtual void DrawFrame() = 0;
    virtual void Cleanup() = 0;
};
