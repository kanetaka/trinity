#pragma once
#include "math.h"
#include "matrix.h"
#include <GL/glew.h>
#include <string>

class Shader
{
public:
    Shader();
    ~Shader();
    bool Load(const std::string& vert_name, const std::string& frag_name);
    void Unload();
    void SetActive();
    void SetMatrixUniform(const char* name, const Mat4f& matrix);
    void SetVectorUniform(const char* name, const Vec3f& vector);
    void SetFloatUniform(const char* name, float value);
private:
    bool CompileShader(const std::string& fileName,
            GLenum shader_type,
            GLuint& out_shader);
    bool IsCompiled(GLuint shader);
    bool IsValidProgram();
private:
    GLuint vertex_shader_;
    GLuint frag_shader_;
    GLuint shader_program_;
};
