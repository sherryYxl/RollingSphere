/*--------------
Fragment Shader for firework
---------------*/
#version 150

in vec4 objPosition;
in vec4 color;

out vec4 outColor;

void main() 
{
    if ( objPosition[1] < 0.1 ) {
        discard;
    }
    outColor = color;
}

