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
#include <unordered_set>
#include <algorithm>

#include <cmath>

#include "mat_h.hpp"
#include "mesh.h"

enum bsp_bool
{
    bsp_union,
    bsp_not,
};

enum bsp_position
{
    POLYGON_COPLANAR_WITH_PLANE,
    POLYGON_IN_FRONT_OF_PLANE,
    POLYGON_BEHIND_PLANE,
    POLYGON_STRADDLING_PLANE,

    POINT_ON_PLANE,
    POINT_IN_FRONT_OF_PLANE,
    POINT_BEHIND_PLANE,
};

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

    vec3 CameraPos = {0.1, 1, -1};
    mat4 ProjMat = Identity();
    mat4 ViewMat = Identity();

    static void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                GLenum severity, GLsizei length,
                                const GLchar *msg, const void *data);

public:
    OpenGLRenderWidget(QWidget* parent = nullptr);

    void MoveTo(vec3 NewPos, float V);
    void SetNewCamera(vec3 Transform);

    mesh Cube;
    mesh Cylinder;

    vec3 TargetPoint = vec3(0.5, 0,  2);

    bool CubeWasModified = false;
    bool FirstStep = true;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};

#endif // OPENGLRENDERWIDGET_H
