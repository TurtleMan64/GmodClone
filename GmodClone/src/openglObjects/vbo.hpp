#ifndef VBO_H
#define VBO_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "../toolbox/maths.hpp"
#include "../toolbox/matrix.hpp"

class Vbo
{
private:
    Vbo(GLuint vboId, GLuint type);

    GLuint vboId;
    GLuint type;

public:
    static Vbo* create(GLuint type);

    void bind();
	
	void unbind();
	
	void storeData(std::vector<float>* data);

	void storeData(std::vector<int>* data);

	void deleteMe();
};
#endif
