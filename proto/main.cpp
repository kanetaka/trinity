#define SDL_MAIN_HANDLED
#include "visualizer.h"

int main()
{
    Visualizer vis;
    bool success = vis.Initialize();

    if (success) {
        vis.RunLoop();
    }

    vis.Shutdown();

    return 0;
}
