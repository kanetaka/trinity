#pragma once

#include <SDL/SDL.h>
#include <unordered_map>
#include <string>
#include <vector>

class Entity;
class CameraEntity;
class Texture;
class Shader;
class VertexArray;
class Renderer;
class Ship;
class Asteroid;
class SpriteComponent;

class Visualizer
{
public:
    Visualizer();

    bool Initialize();
    void RunLoop();
    void Shutdown();

    void AddEntity(Entity* entity);
    void RemoveEntity(Entity* entity);

    Renderer* GetRenderer() { return renderer_; }

private:
    void ProcessInput();
    void UpdateVisualizer();
    void GenerateOutput();
    void LoadData();
    void UnloadData();

private:
    std::vector<Entity*> entities_;
    std::vector<Entity*> pending_entities_;

    Renderer* renderer_;

    Uint32 ticks_count_;
    bool is_running_;
    bool updating_entity_;

    CameraEntity* camera_entity_;
};
