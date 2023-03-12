#version 450

layout(location = 0) in vec4 InPos;
layout(location = 1) in vec3 InNorm;
layout(location = 2) in vec3 InCol;

layout(location = 0) out vec3 OutCol;
layout(location = 1) out vec3 OutNorm;
layout(location = 2) out vec3 OutCamPos;

uniform vec3 CamPos;
uniform mat4 Model;
uniform mat4 Proj;
uniform mat4 View;

void main(void)
{
    OutCol = InCol;
    OutNorm = mat3(Model) * InNorm;
    OutCamPos = CamPos;

    gl_Position = Proj*Model*InPos;
}
