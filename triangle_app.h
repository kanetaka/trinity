#pragma once
#include "kousoku_app.h"

class TriangleApp : public IKousokuApp {
public:
    virtual void Initialize() override { } 
    virtual void DrawFrame() override;
    virtual void Cleanup() override { }
};
