#include "mesh.h"
#include "renderer.h"
#include "texture.h"
#include "vertex_array.h"
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include <SDL/SDL_log.h>
#include "math.h"
#include "vector.h"

Mesh::Mesh() :
    vertex_array_(nullptr),
    radius_(0.0f),
    specular_power_(100.0f) {
}

Mesh::~Mesh() {
}

bool Mesh::Load(const std::string& filename, class Renderer* renderer) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SDL_Log("File not found: Mesh %s", filename.c_str());
        return false;
    }

    std::stringstream file_stream;
    file_stream << file.rdbuf();
    std::string contents = file_stream.str();
    rapidjson::StringStream json_str(contents.c_str());
    rapidjson::Document doc;
    doc.ParseStream(json_str);

    if (!doc.IsObject()) {
        SDL_Log("Mesh %s is not valid json", filename.c_str());
        return false;
    }

    int ver = doc["version"].GetInt();

    if (ver != 1) {
        SDL_Log("Mesh %s not version 1", filename.c_str());
        return false;
    }

    shader_name_ = doc["shader"].GetString();
    size_t vert_size = 8;

    const rapidjson::Value& textures = doc["textures"];
    if (!textures.IsArray() || textures.Size() < 1) {
        SDL_Log("Mesh %s has no textures, there should be at least one", filename.c_str());
        return false;
    }

    specular_power_ = static_cast<float>(doc["specularPower"].GetDouble());

    for (rapidjson::SizeType i = 0; i < textures.Size(); ++i) {
        std::string tex_name = textures[i].GetString();
        Texture* tex = renderer->GetTexture(tex_name);
        if (tex == nullptr) {
            tex = renderer->GetTexture(tex_name);
            if (tex == nullptr) {
                tex = renderer->GetTexture("assets/default.png");
            }
        }
        textures_.emplace_back(tex);
    }

    const rapidjson::Value& verts_json = doc["vertices"];
    if (!verts_json.IsArray() || verts_json.Size() < 1) {
        SDL_Log("Mesh %s has no vertices", filename.c_str());
        return false;
    }

    std::vector<float> vertices;
    vertices.reserve(verts_json.Size() * vert_size);
    radius_ = 0.0f;
    for (rapidjson::SizeType i = 0; i < verts_json.Size(); ++i) {
        const rapidjson::Value& vert = verts_json[i];
        if (!vert.IsArray() || vert.Size() != 8) {
            SDL_Log("Unexpected vertex format for %s", filename.c_str());
            return false;
        }

        Vec3f pos(vert[0].GetFloat(), vert[1].GetFloat(), vert[2].GetFloat());
        radius_ = Math::Max(radius_, pos.LengthSq());

        for (rapidjson::SizeType i = 0; i < vert.Size(); ++i) {
            vertices.emplace_back(static_cast<float>(vert[i].GetFloat()));
        }
    }

    radius_ = Math::Sqrt(radius_);

    const rapidjson::Value& indices_json = doc["indices"];
    if (!indices_json.IsArray() || indices_json.Size() < 1) {
        SDL_Log("Mesh %s has no indices", filename.c_str());
        return false;
    }

    std::vector<unsigned int>indices;
    indices.reserve(indices_json.Size() * 3);
    for (rapidjson::SizeType i = 0; i < indices_json.Size(); ++i) {
        const rapidjson::Value& index = indices_json[i];
        if (!index.IsArray() || index.Size() != 3) {
            SDL_Log("Invalud indices for %s", filename.c_str());
            return false;
        }
        indices.emplace_back(index[0].GetUint());
        indices.emplace_back(index[1].GetUint());
        indices.emplace_back(index[2].GetUint());
    }
    vertex_array_ = new VertexArray(
            vertices.data(), static_cast<unsigned int>(vertices.size()) / vert_size,
            indices.data(), static_cast<unsigned int>(indices.size()));

    return true;
}

void Mesh::Unload() {
    delete vertex_array_;
    vertex_array_ = nullptr;
}

Texture* Mesh::GetTexture(size_t index) {
    if (index < textures_.size()) {
        return textures_[index];
    }
    else {
        return nullptr;
    }
}
