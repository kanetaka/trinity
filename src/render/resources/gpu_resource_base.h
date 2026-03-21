#pragma once
#include <memory>

namespace trinity::render {




template<typename T>
class GpuResourceBase
{
public:
    // Copying is prohibited
    GpuResourceBase(const GpuResourceBase&) = delete;
    GpuResourceBase& operator=(const GpuResourceBase&) = delete;

    virtual ~GpuResourceBase() = default;

    static std::shared_ptr<T> Create() { return std::shared_ptr<T>(new T()); }
protected:
    GpuResourceBase() = default;    // Protected to restrict creation via factory method
};


} // namespace trinity::render
