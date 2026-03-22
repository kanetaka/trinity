#include "stream/ply_loader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

namespace tri
{
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
} // namespace tri
