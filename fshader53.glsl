/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version
//in vec4
in vec4 color;
in vec2 groundTexCoord;
in vec2 sphereTexCoord;
in vec2 latticeCoord;
in float z;

out vec4 fColor;

uniform sampler2D checkerBoard;
uniform sampler1D stripe;
uniform int fogOptions;

uniform int groundTexOption;
uniform int sphereTexOption;
uniform int spaceOption;
uniform int groundSphereOrShadow; //ground or sphere or shadow? 0: others; 1: ground; 2: sphere; 3: shadow;
uniform int latticeOption;

void main() 
{
    vec4 texColor = color;

    if ( groundTexOption == 1 && groundSphereOrShadow == 1 ) {
        texColor = color * texture( checkerBoard, groundTexCoord );
    }
    else if ( groundSphereOrShadow == 2 ){
        if ( sphereTexOption == 1 || sphereTexOption == 2 ){
            texColor = color * texture( stripe, sphereTexCoord[0] );
        }
        else if ( sphereTexOption >=3 ){
            texColor = color * texture ( checkerBoard, sphereTexCoord );
            if ( texColor[0] == 0 ) {
                texColor = color * vec4(0.9, 0.1, 0.1, 1.0);
            }
        }
    }
    
    if ( latticeOption > 0 ) {
        float s_fract = fract( 4 * latticeCoord[0] );
        float t_fract = fract( 4 * latticeCoord[1] );
        if (groundSphereOrShadow >=2 && s_fract < 0.35 && t_fract < 0.35) {
            discard;
        }
    }

    if (fogOptions == 0) {
        fColor = texColor;
    }
    else{
        vec4 Cf = vec4(0.7, 0.7, 0.7, 0.5);
        float f, f_linear, f_exponential, f_exponential_square;
        float start = 0.0;
        float end = 18.0;
        float density = 0.09;
        
        if (fogOptions == 1) {
            f_linear = (end - z)/(end - start);
            f = f_linear;
        }
        else if (fogOptions == 2){
            f_exponential = exp( -density * z);
            f = f_exponential;
        }
        else if (fogOptions == 3){
            f_exponential_square = exp( -pow((density * z),2) );
            f = f_exponential_square;
        }
        f = clamp(f, 0, 1);
        fColor = clamp( mix(Cf, texColor, f), 0, 1);
    }
} 

