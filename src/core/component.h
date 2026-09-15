#pragma once

namespace tri
{
    // Interface implemented by every component that can be attached to an Object.
    class IComponent
    {
    public:
        virtual ~IComponent() = default;
    };
} // namespace tri
