#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 M_normal;
out vec3 M_pos;

void main()
{
    M_normal = normal;
    vec4 mpos = proj * view * model * vec4(pos, 1.0);
    M_pos = mpos.xyz / mpos.w;
    gl_Position = mpos;
}