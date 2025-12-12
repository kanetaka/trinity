#pragma once
#include <memory>

template<typename T>
class GpuResourceBase {
public:
	GpuResourceBase(const GpuResourceBase&) = delete; // Prevent copy
    virtual ~GpuResourceBase() = default;
protected:
    GpuResourceBase() = default;

public:
    static std::shared_ptr<T> Create() { return std::shared_ptr<T>(new T()); }

public:
	GpuResourceBase& operator=(const GpuResourceBase&) = delete; // Prevent copy
};
