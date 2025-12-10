#pragma once
#include "common/application.h"

class TriangleApp : public IApplication {
public:
    virtual void Initialize() override { } 
    virtual void DrawFrame() override;
    virtual void Cleanup() override { }
};
