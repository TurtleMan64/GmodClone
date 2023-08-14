#ifndef FONTSHADER_H
#define FONTSHADER_H

class Matrix4f;
class Camera;
class Light;
class Vector3f;

#include <glad/glad.h>


class FontShader
{
private:
    GLuint programId;
    GLuint vertexShaderId;
    GLuint fragmentShaderId;

    int location_color;
    int location_translation;
    int location_fontHeight;
    int location_screenRatio;
    int location_screenHeight;
    int location_alpha;

public:
    FontShader(const char*, const char*);

    void start();

    void stop();

    void cleanUp();

    void loadColor(Vector3f* color);

    void loadAlpha(float alpha);

    void loadTranslation(Vector2f* translation);

    void loadFontHeight(float fontHeight);

    void loadScreenRatio(float ratio);

    void loadScreenHeight(int height);

protected:
    void bindAttributes();

    void bindAttribute(int, const char*);

    void getAllUniformLocations();

    int getUniformLocation(const char*);

    void loadFloat(int, float);

    void loadInt(int, int);

    void loadVector(int, Vector3f*);

    void load2DVector(int location, Vector2f* vect);

    void loadBoolean(int, float);

    void loadMatrix(int, Matrix4f*);

    GLuint loadShader(const char*, int);
};

#endif
