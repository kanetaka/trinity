#pragma once

class INovaApp {
public:
  virtual ~INovaApp() = default;
  virtual void Initialize() = 0;
  virtual void DrawFrame() = 0;
  virtual void Cleanup() = 0;
};
