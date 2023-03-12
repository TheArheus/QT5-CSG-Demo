#version 450

layout(location = 0) in vec3 InCol;
layout(location = 1) in vec3 InNorm;
layout(location = 2) in vec3 InCamPos;


void main(void)
{
    float Brightness = clamp(dot(InNorm, InCamPos), 0, 1);
    gl_FragColor = vec4(InCol*Brightness, 1);
}
