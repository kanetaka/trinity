#include "shader.h"
#include "texture.h"
#include <SDL/SDL.h>
#include <fstream>
#include <sstream>

Shader::Shader() :
    shader_program_(0),
    vertex_shader_(0),
    frag_shader_(0)
{
}

Shader::~Shader()
{
}

bool Shader::Load(const std::string& vert_name, const std::string& frag_name)
{
    if (!CompileShader(vert_name, GL_VERTEX_SHADER, vertex_shader_) ||
                       !CompileShader(frag_name, GL_FRAGMENT_SHADER,
                       frag_shader_)) {
        return false;
    }

    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex_shader_);
    glAttachShader(shader_program_, frag_shader_);
    glLinkProgram(shader_program_);

    if (!IsValidProgram()) {
        return false;
    }

    return true;
}

void Shader::Unload()
{
    glDeleteProgram(shader_program_);
    glDeleteShader(vertex_shader_);
    glDeleteShader(frag_shader_);
}

void Shader::SetActive()
{
    glUseProgram(shader_program_);
}

void Shader::SetMatrixUniform(const char* name, const Mat4f& matrix)
{
    GLuint loc = glGetUniformLocation(shader_program_, name);
    glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.data_);
}


void Shader::SetVectorUniform(const char* name, const Vec3f& vector)
{
    GLuint loc = glGetUniformLocation(shader_program_, name);
    glUniform3fv(loc, 1, vector.data_);
}

void Shader::SetFloatUniform(const char* name, float value)
{
    GLuint loc = glGetUniformLocation(shader_program_, name);
    glUniform1f(loc, value);
}

bool Shader::CompileShader(
        const std::string& file_name,
        GLenum shader_type,
        GLuint& out_shader)
{
    std::ifstream shader_file(file_name);

    if (shader_file.is_open()) {
        std::stringstream sstream;
        sstream << shader_file.rdbuf();
        std::string contents = sstream.str();
        const char* contents_char = contents.c_str();

        out_shader = glCreateShader(shader_type);
        glShaderSource(out_shader, 1, &(contents_char), nullptr);
        glCompileShader(out_shader);

        if (!IsCompiled(out_shader)) {
            SDL_Log("シェーダーのコンパイルに失敗しました %s", file_name.c_str());
            return false;
        }
    }
    else {
        SDL_Log("シェーダーファイルが見つかりませんでした: %s", file_name.c_str());
        return false;
    }

    return true;
}

bool Shader::IsCompiled(GLuint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetShaderInfoLog(shader, 511, nullptr, buffer);
        SDL_Log("GLSLのコンパイルに失敗しました:\n%s", buffer);
        return false;
    }

    return true;
}

bool Shader::IsValidProgram()
{
    GLint status;
    glGetProgramiv(shader_program_, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        char buffer[512];
        memset(buffer, 0, 512);
        glGetProgramInfoLog(shader_program_, 511, nullptr, buffer);
        SDL_Log("GLSLリンクステータス:\n%s", buffer);
        return false;
    }

    return true;
}
