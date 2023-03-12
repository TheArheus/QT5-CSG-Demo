#ifndef OPENGLRENDERWIDGET_H
#define OPENGLRENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_core>
#include <QTimer>
#include <QDebug>

#include <fstream>
#include <sstream>
#include <iostream>

#include <queue>

#include "mat_h.hpp"
#include "mesh.h"

struct bsp_node
{
    vec4 Plane;
    std::vector<polygon> Polygons;
    std::unique_ptr<bsp_node> Front;
    std::unique_ptr<bsp_node> Back;
};

class OpenGLRenderWidget : public QOpenGLWidget, public QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

    float DeltaTime = 1.0f/30.0f;
    float A = 0;

    GLuint Program;
    GLuint CubeVertexBuffer;
    GLuint CubeIndexBuffer;
    GLuint CylinderVertexBuffer;
    GLuint CylinderIndexBuffer;
    GLuint CubeVertexObject;
    GLuint CylinderVertexObject;

    vec3 CameraPos = {0, 0, -1};
    mat4 ProjMat = Identity();
    mat4 ViewMat = Identity();

    static void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *msg, const void *data);

public:
    OpenGLRenderWidget(QWidget* parent = nullptr);

    void MoveTo(vec3 NewPos, float V);

    mesh Cube;
    mesh Cylinder;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};

#endif // OPENGLRENDERWIDGET_H
