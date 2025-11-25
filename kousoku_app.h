#pragma once

class IKousokuApp {
public:
    virtual ~IKousokuApp() = default;
    virtual void OnInitialize() = 0;
    virtual void OnDrawFrame() = 0;
    virtual void OnCleanup() = 0;
};
