#include "vertex_array.h"
#include <GL/glew.h>

VertexArray::VertexArray(const float* verts,
                         unsigned int num_verts,
                         const unsigned int* indices,
                         unsigned int num_indices) :
    num_verts_(num_verts),
    num_indices_(num_indices)
{
    // Vertex Array
    glGenVertexArrays(1, &vertex_array_);
    glBindVertexArray(vertex_array_);

    // Vertex Buffer
    glGenBuffers(1, &vertex_buffer_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    // glBufferData(GL_ARRAY_BUFFER, num_verts * 5 * sizeof(float), verts, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, num_verts * 8 * sizeof(float), verts, GL_STATIC_DRAW);

    // Index Buffer
    glGenBuffers(1, &index_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
            reinterpret_cast<void*>(sizeof(float) * 3));
    // Texture
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8,
            reinterpret_cast<void*>(sizeof(float) * 6));
}

VertexArray::~VertexArray()
{
    glDeleteBuffers(1, &vertex_buffer_);
    glDeleteBuffers(1, &index_buffer_);
    glDeleteVertexArrays(1, &vertex_array_);
}

void VertexArray::SetActive()
{
    glBindVertexArray(vertex_array_);
}
