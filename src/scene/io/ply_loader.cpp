#include "scene/io/ply_loader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

using namespace tri;

namespace
{
    struct Property
    {
        std::string name;
        std::string type;
        size_t byteSize;
        size_t offset;
    };
} // namespace

bool PlyLoader::LoadPly(const std::string& filepath, std::vector<FullSplat>& out_splats)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open PLY file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);
    if (line != "ply" && line != "ply\r")
    {
        std::cerr << "Invalid PLY format: Missing 'ply' magic word." << std::endl;
        return false;
    }

    std::getline(file, line);
    if (line.find("format binary_little_endian") == std::string::npos)
    {
        std::cerr << "Only binary little endian PLY files are supported."
            << std::endl;
        return false;
    }

    size_t vertexCount = 0;
    std::vector<Property> properties;
    size_t currentOffset = 0;
    bool inVertexElement = false;

    // Parse Header
    while (std::getline(file, line))
    {
        // Strip carriage return if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line == "end_header")
        {
            break;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "element")
        {
            std::string elementType;
            iss >> elementType;
            if (elementType == "vertex")
            {
                iss >> vertexCount;
                inVertexElement = true;
            }
            else
            {
                inVertexElement = false;
            }
        }
        else if (token == "property" && inVertexElement)
        {
            std::string propType, propName;
            iss >> propType >> propName;

            size_t size = 0;
            if (propType == "float" || propType == "float32")
                size = 4;
            else if (propType == "double" || propType == "float64")
                size = 8;
            else if (propType == "uchar" || propType == "uint8")
                size = 1;
            else if (propType == "int" || propType == "int32")
                size = 4;

            if (size > 0)
            {
                properties.push_back({ propName, propType, size, currentOffset });
                currentOffset += size;
            }
        }
    }

    if (vertexCount == 0 || properties.empty())
    {
        std::cerr << "Invalid PLY file or no vertices found." << std::endl;
        return false;
    }

    size_t vertexStride = currentOffset;
    out_splats.resize(vertexCount);

    // Create a mapping from property name to offset for quick access
    std::map<std::string, size_t> propMap;
    for (const auto& prop : properties)
    {
        propMap[prop.name] = prop.offset;
    }

    // Helper lambda to get offset, or max size_t if not found
    auto getOffset = [&](const std::string& name) -> size_t
        {
            auto it = propMap.find(name);
            return it != propMap.end() ? it->second : static_cast<size_t>(-1);
        };

    // Pre-look up offsets
    size_t off_x = getOffset("x");
    size_t off_y = getOffset("y");
    size_t off_z = getOffset("z");

    size_t off_nx = getOffset("nx");
    size_t off_ny = getOffset("ny");
    size_t off_nz = getOffset("nz");

    size_t off_f_dc_0 = getOffset("f_dc_0");
    size_t off_f_dc_1 = getOffset("f_dc_1");
    size_t off_f_dc_2 = getOffset("f_dc_2");

    // sh_rest offsets
    std::vector<size_t> off_f_rest(45, static_cast<size_t>(-1));
    for (int i = 0; i < 45; ++i)
    {
        off_f_rest[i] = getOffset("f_rest_" + std::to_string(i));
    }

    size_t off_opacity = getOffset("opacity");
    size_t off_scale_0 = getOffset("scale_0");
    size_t off_scale_1 = getOffset("scale_1");
    size_t off_scale_2 = getOffset("scale_2");

    size_t off_rot_0 = getOffset("rot_0");
    size_t off_rot_1 = getOffset("rot_1");
    size_t off_rot_2 = getOffset("rot_2");
    size_t off_rot_3 = getOffset("rot_3");

    // Read payload
    std::vector<char> buffer(vertexStride);
    for (size_t i = 0; i < vertexCount; ++i)
    {
        file.read(buffer.data(), vertexStride);
        if (!file)
        {
            std::cerr << "Error reading PLY payload at vertex " << i << std::endl;
            return false;
        }

        FullSplat& splat = out_splats[i];

        // Helper to read a float safely
        auto readFloat = [&](size_t offset, float fallback = 0.0f) -> float
            {
                if (offset != static_cast<size_t>(-1))
                {
                    return *reinterpret_cast<float*>(buffer.data() + offset);
                }
                return fallback;
            };

        splat.position.x = readFloat(off_x);
        splat.position.y = readFloat(off_y);
        splat.position.z = readFloat(off_z);

        splat.normal.x = readFloat(off_nx);
        splat.normal.y = readFloat(off_ny);
        splat.normal.z = readFloat(off_nz);

        splat.sh_dc[0] = readFloat(off_f_dc_0);
        splat.sh_dc[1] = readFloat(off_f_dc_1);
        splat.sh_dc[2] = readFloat(off_f_dc_2);

        for (int j = 0; j < 45; ++j)
        {
            splat.sh_rest[j] = readFloat(off_f_rest[j]);
        }

        splat.opacity = readFloat(off_opacity, 1.0f);

        splat.scale.x = readFloat(off_scale_0, 1.0f);
        splat.scale.y = readFloat(off_scale_1, 1.0f);
        splat.scale.z = readFloat(off_scale_2, 1.0f);

        splat.rot.x = readFloat(off_rot_0, 1.0f); // Sometimes rot_0 is w, but we'll read it straight
        splat.rot.y = readFloat(off_rot_1, 0.0f);
        splat.rot.z = readFloat(off_rot_2, 0.0f);
        splat.rot.w = readFloat(off_rot_3, 0.0f);
    }

    std::cout << "Successfully loaded " << vertexCount << " splats from "
        << filepath << std::endl;
    return true;
}

bool PlyLoader::LoadPointCloud(const std::string& filepath, std::vector<PointCloudVertex>& out_vertices)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open PLY file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);
    if (line != "ply" && line != "ply\r")
    {
        std::cerr << "Invalid PLY format: Missing 'ply' magic word." << std::endl;
        return false;
    }

    bool is_ascii = false;
    bool is_binary_little_endian = false;

    std::getline(file, line);
    if (line.find("format ascii") != std::string::npos)
    {
        is_ascii = true;
    }
    else if (line.find("format binary_little_endian") != std::string::npos)
    {
        is_binary_little_endian = true;
    }
    else
    {
        std::cerr << "Unsupported PLY format: Only ASCII and binary little-endian are supported." << std::endl;
        return false;
    }

    size_t vertex_count = 0;
    std::vector<Property> properties;
    size_t current_offset = 0;
    bool in_vertex_element = false;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line == "end_header")
        {
            break;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "element")
        {
            std::string element_type;
            iss >> element_type;
            if (element_type == "vertex")
            {
                iss >> vertex_count;
                in_vertex_element = true;
            }
            else
            {
                in_vertex_element = false;
            }
        }
        else if (token == "property" && in_vertex_element)
        {
            std::string prop_type;
            std::string prop_name;
            iss >> prop_type >> prop_name;

            size_t size = 0;
            if (prop_type == "float" || prop_type == "float32")
            {
                size = 4;
            }
            else if (prop_type == "double" || prop_type == "float64")
            {
                size = 8;
            }
            else if (prop_type == "uchar" || prop_type == "uint8" || prop_type == "char" || prop_type == "int8")
            {
                size = 1;
            }
            else if (prop_type == "ushort" || prop_type == "uint16" || prop_type == "short" || prop_type == "int16")
            {
                size = 2;
            }
            else if (prop_type == "uint" || prop_type == "uint32" || prop_type == "int" || prop_type == "int32")
            {
                size = 4;
            }

            properties.push_back({ prop_name, prop_type, size, current_offset });
            current_offset += size;
        }
    }

    if (vertex_count == 0 || properties.empty())
    {
        std::cerr << "Invalid PLY file or no vertices found." << std::endl;
        return false;
    }

    out_vertices.resize(vertex_count);

    int idx_x = -1;
    int idx_y = -1;
    int idx_z = -1;
    int idx_r = -1;
    int idx_g = -1;
    int idx_b = -1;

    for (size_t i = 0; i < properties.size(); ++i)
    {
        const auto& name = properties[i].name;
        if (name == "x") idx_x = static_cast<int>(i);
        else if (name == "y") idx_y = static_cast<int>(i);
        else if (name == "z") idx_z = static_cast<int>(i);
        else if (name == "red" || name == "r" || name == "diffuse_red") idx_r = static_cast<int>(i);
        else if (name == "green" || name == "g" || name == "diffuse_green") idx_g = static_cast<int>(i);
        else if (name == "blue" || name == "b" || name == "diffuse_blue") idx_b = static_cast<int>(i);
    }

    if (idx_x == -1 || idx_y == -1 || idx_z == -1)
    {
        std::cerr << "Point cloud PLY missing x, y, or z properties." << std::endl;
        return false;
    }

    if (is_ascii)
    {
        for (size_t i = 0; i < vertex_count; ++i)
        {
            if (!std::getline(file, line))
            {
                std::cerr << "Unexpected EOF reading ASCII point cloud at vertex " << i << std::endl;
                return false;
            }

            std::istringstream iss(line);
            PointCloudVertex& v = out_vertices[i];
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);

            for (size_t p = 0; p < properties.size(); ++p)
            {
                if (static_cast<int>(p) == idx_x)
                {
                    iss >> v.position.x;
                }
                else if (static_cast<int>(p) == idx_y)
                {
                    iss >> v.position.y;
                }
                else if (static_cast<int>(p) == idx_z)
                {
                    iss >> v.position.z;
                }
                else if (static_cast<int>(p) == idx_r)
                {
                    float val = 0.0f;
                    iss >> val;
                    if (properties[p].type == "uchar" || properties[p].type == "uint8" || val > 1.0f)
                    {
                        val /= 255.0f;
                    }
                    v.color.r = val;
                }
                else if (static_cast<int>(p) == idx_g)
                {
                    float val = 0.0f;
                    iss >> val;
                    if (properties[p].type == "uchar" || properties[p].type == "uint8" || val > 1.0f)
                    {
                        val /= 255.0f;
                    }
                    v.color.g = val;
                }
                else if (static_cast<int>(p) == idx_b)
                {
                    float val = 0.0f;
                    iss >> val;
                    if (properties[p].type == "uchar" || properties[p].type == "uint8" || val > 1.0f)
                    {
                        val /= 255.0f;
                    }
                    v.color.b = val;
                }
                else
                {
                    std::string dummy;
                    iss >> dummy;
                }
            }
        }
    }
    else if (is_binary_little_endian)
    {
        size_t vertex_stride = current_offset;
        std::vector<char> buffer(vertex_stride);

        auto read_float_val = [&](const Property& prop, const char* data) -> float
        {
            if (prop.type == "float" || prop.type == "float32")
            {
                return *reinterpret_cast<const float*>(data + prop.offset);
            }
            if (prop.type == "double" || prop.type == "float64")
            {
                return static_cast<float>(*reinterpret_cast<const double*>(data + prop.offset));
            }
            if (prop.type == "uchar" || prop.type == "uint8")
            {
                return static_cast<float>(*reinterpret_cast<const uint8_t*>(data + prop.offset));
            }
            if (prop.type == "ushort" || prop.type == "uint16")
            {
                return static_cast<float>(*reinterpret_cast<const uint16_t*>(data + prop.offset));
            }
            if (prop.type == "int" || prop.type == "int32")
            {
                return static_cast<float>(*reinterpret_cast<const int32_t*>(data + prop.offset));
            }
            return 0.0f;
        };

        for (size_t i = 0; i < vertex_count; ++i)
        {
            file.read(buffer.data(), vertex_stride);
            if (!file)
            {
                std::cerr << "Error reading binary PLY payload at vertex " << i << std::endl;
                return false;
            }

            PointCloudVertex& v = out_vertices[i];
            v.position.x = read_float_val(properties[idx_x], buffer.data());
            v.position.y = read_float_val(properties[idx_y], buffer.data());
            v.position.z = read_float_val(properties[idx_z], buffer.data());

            v.color = glm::vec3(1.0f, 1.0f, 1.0f);
            if (idx_r != -1)
            {
                float val = read_float_val(properties[idx_r], buffer.data());
                if (properties[idx_r].type == "uchar" || properties[idx_r].type == "uint8" || val > 1.0f)
                {
                    val /= 255.0f;
                }
                v.color.r = val;
            }
            if (idx_g != -1)
            {
                float val = read_float_val(properties[idx_g], buffer.data());
                if (properties[idx_g].type == "uchar" || properties[idx_g].type == "uint8" || val > 1.0f)
                {
                    val /= 255.0f;
                }
                v.color.g = val;
            }
            if (idx_b != -1)
            {
                float val = read_float_val(properties[idx_b], buffer.data());
                if (properties[idx_b].type == "uchar" || properties[idx_b].type == "uint8" || val > 1.0f)
                {
                    val /= 255.0f;
                }
                v.color.b = val;
            }
        }
    }

    std::cout << "Successfully loaded " << vertex_count << " point cloud vertices from "
        << filepath << std::endl;
    return true;
}

