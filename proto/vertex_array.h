#pragma once

class VertexArray
{

public:
    VertexArray(
            const float* verts,
            unsigned int num_verts,
            const unsigned int* indices,
            unsigned int num_indices);
    ~VertexArray();

public:
    unsigned int GetNumIndices() const { return num_indices_; }
    unsigned int GetNumVerts() const { return num_verts_; }
    void SetActive();

private:
    unsigned int num_verts_;
    unsigned int num_indices_;
    unsigned int vertex_buffer_;
    unsigned int index_buffer_;
    unsigned int vertex_array_;
};
