#pragma once
#include "nova_app.h"

class TriangleApp : public INovaApp {
public:
  virtual void Initialize() override {}
  virtual void DrawFrame() override;
  virtual void Cleanup() override {}
};