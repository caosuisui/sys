#version 330 core

uniform vec4 color;

void main()
{
    //gl_FragColor = vec4(1,0,0,1);
    gl_FragColor = color;
}