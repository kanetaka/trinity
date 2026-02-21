#pragma once

class INo9App {
public:
  virtual ~INo9App() = default;
  virtual void Initialize() = 0;
  virtual void DrawFrame() = 0;
  virtual void Cleanup() = 0;
};
