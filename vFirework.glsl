/*--------------/Users/xlinyu/Desktop/ComputerGraphics/Assignment4/Rolling-Sphere-Final/build/Debug/vFirework.glsl
Vertex Shader for firework
---------------*/
#version 150

in  vec3 fireworkVelocity;
in  vec3 fireworkColors;

uniform mat4 P_Firework;
uniform mat4 MV_Firework;
uniform float time;

out vec4 objPosition;
out vec4 color;

void main() 
{
    vec3 pos;
    float a = -0.00000049;
    
    pos[0] = 0.001 * fireworkVelocity[0] * time;
    pos[1] = 0.1 + 0.001 * fireworkVelocity[1] * time + 0.5 * a * time * time;
    pos[2] = 0.001 * fireworkVelocity[2] * time;

    objPosition = vec4(pos.xyz, 1);

    
    gl_Position = P_Firework * MV_Firework * objPosition;

    color = vec4(fireworkColors.xyz, 1.0);
} 
