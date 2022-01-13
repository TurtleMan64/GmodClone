#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class Vbo;

class Vao
{
private:
    Vao(GLuint id);

    void bindPrivate();

	void unbindPrivate();

public:
    static constexpr int BYTES_PER_FLOAT = 4;
    static constexpr int BYTES_PER_INT = 4;

    GLuint id = GL_NONE;
    std::vector<Vbo*> dataVbos;
    Vbo* indexVbo = nullptr;
    int indexCount = -1;

    //Creates a vao on the heap, so needs to be deleted later
    static Vao* create();

    int getIndexCount();

	void bind(std::vector<GLuint>* attributes);

	void unbind(std::vector<GLuint>* attributes);
	
	void createIndexBuffer(std::vector<int>* indices);

	void createAttribute(GLuint attribute, std::vector<float>* data, GLint attrSize);
	
	void createIntAttribute(GLuint attribute, std::vector<int>* data, GLint attrSize);
	
	void deleteMe();
};
#endif
