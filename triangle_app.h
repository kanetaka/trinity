#pragma once
#include "trinity_app.h"

class TriangleApp : public ITrinityApp {
public:
    virtual void Initialize() override { } 
    virtual void DrawFrame() override;
    virtual void Cleanup() override { }
};