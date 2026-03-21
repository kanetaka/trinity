#pragma once

namespace tr {

// Default interface for sample programs
class ITrinityApp
{
public:
    virtual ~ITrinityApp() = default;
    virtual void OnInitialize() = 0;
    virtual void OnDrawFrame() = 0;
    virtual void OnCleanup() = 0;

#if defined(__ANDROID__)
    virtual void OnSurfaceChanged() = 0;
#endif
};

} // namespace tr

// Checker Macro
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
# error need to define `GLM_FORCE_DEPTH_ZERO_TO_ONE`
#endif
