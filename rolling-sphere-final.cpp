//
//  rolling-sphere-shading.cpp
//  based on rolling-sphere.cpp, rotate-cube-shading.cpp and rotate-cube-new.cpp
//  Created by Xuelin Yu on 10/30/18
//  Last modified on 12/20/18

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "Angel-yjc.h"

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

// flags
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'
int beginkey = 0; // 1: "begin" key has been hit. 0: "begin" key has not been hit.
int shadowFlag = 1; // 1: produce the shadow; 0: do not produce the shadow.
int blendingShadowFlag = 1; //1: enable shadow blending; 0: do not enable shadow blending.
int lightingFlag = 1; // 1: lighting is enable; 0: lighting is disabled.
int shadingFlag = 2; // 1: flat shading; 2: smooth shading;
int wireFlag = 1; // 1: draw a wire-frame sphere; 0: draw a filled sphere.
int lightSourceFlag = 0; //1: point source; 2: spot light; 0: no positional light;
int fogFlag = 0; //0: no fog; 1: linear; 2: exponential; 3: exponential square;
int groundTextureFlag = 0; //0: do not perform texture mapping on the ground; 1: perform texture mapping;
int sphereTextureFlag = 0; //0: do not perform texture mapping on the sphere; 1: perform "vertical" stripe; 2: perform "slanted" stripe;
                            // 3: perform "vertical" checkerboard; 4 perfomr "slanted" checkerboard;
int whichSpace = 0; //sphere texture coordinate in 0: object space; 1: eye space;
int latticeFlag = 0; //0: no lattice effect; 1: upright; 2: tilted
int fireworkFlag = 0; //0: disable firework; 1: enable firework

float timeStart; //a timer for firework
float time_max = 9000;
float time_firework;

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;
GLuint program;       /* shader program object id */
GLuint pfirework;  /* firework shader program object id */

GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint shadow_buffer;   /* vertex buffer object id for shadow */
GLuint floor_buffer;   /* vertex buffer object id for floor */
GLuint axes_buffer;     /* vertex buffer object id for axes */
GLuint firework_buffer; /* vertex buffer object id for firework */

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 20.0;

vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

//rolling animation parameters
float delta_angle = 1; // rotation speed;
int state = 0; //which state: A-B: 0, B-C: 1, C-A: 2
float angle = 0.0; // rotation angle in degrees
float accAngle = 0.0; // accumulated rotation angle
float d_angle=0.0; // distance travelled when rolling an angle of "angle"
vec3 OA(3.0, 1.0, 5.0); //vector from O to A
vec3 OB(-2.0, 1.0, -2.5); //vector from O to B
vec3 OC(2.0, 1.0, -4.0); //vector from O to C
vec3 Rab(-7.5, 0.0, 5.0); //rotation axis vector when rotate from A to B;
vec3 Rbc(-1.5, 0.0, -4.0); //rotation axis vector when rotate from B to C;
vec3 Rca(9.0, 0.0, -1.0); //rotation axis vector when rotate from C to A;

vec3 vtranslation(3.0, 1.0, 5.0); //translation vector
vec3 vrotation = Rab; // rotation axis vector
mat4 ACC_R = (1, 1, 1, 1); // accumulated rotation

//Shader Lighting Parameters
color4 global_ambient( 1.0, 1.0, 1.0, 1.0 );
//--- material--sphere
color4 sphere_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 sphere_diffuse( 1.0, 0.84, 0.0, 1.0 );
color4 sphere_specular( 1.0, 0.84, 0.0, 1.0 );
float  sphere_shininess = 125.0;
//--- material--floor
color4 floor_ambient(0.2, 0.2, 0.2, 1.0);
color4 floor_diffuse(0.0, 1.0, 0.0, 1.0);
color4 floor_specular(0.0, 0.0, 0.0, 1.0);

//---part c light
vec4 c_direction( 0.1, 0.0, -1.0, 0 ); //light direction vector
color4 c_ambient( 0.0, 0.0, 0.0, 1.0 );
color4 c_diffuse( 0.8, 0.8, 0.8, 1.0 );
color4 c_specular( 0.2, 0.2, 0.2, 1.0 );

//---part d light
point4 light_source( -14.0, 12.0, -3.0, 1.0 ); // also for shadow
point4 light_dest( -6.0, 0.0, -4.5, 1.0 );
color4 d_ambient( 0.0, 0.0, 0.0, 1.0 );
color4 d_diffuse( 1.0, 1.0, 1.0, 1.0 );
color4 d_specular( 1.0, 1.0, 1.0, 1.0 );

//---part d spot light
float const_att = 2.0;
float linear_att = 0.01;
float quad_att = 0.001;
float exponent = 15.0;
float cutoff_angle = 20 * 180 / M_PI;

//floor-----------------------------------------------------------------------
const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
color4 floor_color(0.0, 1.0, 0.0, 1.0);
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 floor_normals[floor_NumVertices]; // normals for all faces
point4 floor_vertices[4] = {
    point4(-5.0, 0.0, -4.0, 1.0),
    point4(-5.0, 0.0, 8.0, 1.0),
    point4(5.0, 0.0, 8.0, 1.0),
    point4(5.0, 0.0, -4.0, 1.0),
};
vec2 floor_texCoord[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 1.5),
    vec2(1.25, 1.5),
    
    vec2(1.25, 1.5),
    vec2(1.25, 0.0),
    vec2(0.0, 0.0),
};
void floor() // generate 2 triangles: 6 vertices and 6 colors
{
    //compute floor normal
    vec4 u = floor_vertices[2] - floor_vertices[0];
    vec4 v = floor_vertices[1] - floor_vertices[0];
    vec3 normal = normalize(cross(v, u));
    //normal = normalize(vec3(0, 1, -0.1)); //*** avoid being perpendicular to the light direction in part c
    
    floor_colors[0] = floor_color; floor_normals[0] = normal; floor_points[0] = floor_vertices[0];
    floor_colors[1] = floor_color; floor_normals[1] = normal; floor_points[1] = floor_vertices[1];
    floor_colors[2] = floor_color; floor_normals[2] = normal; floor_points[2] = floor_vertices[2];
    
    floor_colors[3] = floor_color; floor_normals[3] = normal; floor_points[3] = floor_vertices[2];
    floor_colors[4] = floor_color; floor_normals[4] = normal; floor_points[4] = floor_vertices[3];
    floor_colors[5] = floor_color; floor_normals[5] = normal; floor_points[5] = floor_vertices[0];
}
//-----------------------------------------------------------------------------

//axes-------------------------------------------------------------------------
color4 axes_colors[6];  //colors for axes
point4 axes_points[6];  //positions for axes
color4 x_axis_color(1.0, 0.0, 0.0, 1.0);
color4 y_axis_color(1.0, 0.0, 1.0, 1.0);
color4 z_axis_color(0.0, 0.0, 1.0, 1.0);
point4 origin(0.0, 0.0, 0.0, 1.0);
point4 x_axis_end(10.0, 0.0, 0.0, 1.0);
point4 y_axis_end(0.0, 10.0, 0.0, 1.0);
point4 z_axis_end(0.0, 0.0, 10.0, 1.0);
void axes() // generate 3 axes(segments): origin to x/y/z/_axis_end
{
    axes_colors[0] = x_axis_color; axes_points[0] = origin;
    axes_colors[1] = x_axis_color; axes_points[1] = x_axis_end;
    
    axes_colors[2] = y_axis_color; axes_points[2] = origin;
    axes_colors[3] = y_axis_color; axes_points[3] = y_axis_end;
    
    axes_colors[4] = z_axis_color; axes_points[4] = origin;
    axes_colors[5] = z_axis_color; axes_points[5] = z_axis_end;
}
//-----------------------------------------------------------------------------

//sphere-----------------------------------------------------------------------
color4 sphere_color = color4(1.0, 0.84, 0, 1.0);
point4 *sphere_points; // positions for all vertices
color4 *sphere_colors; // colors for all vertices
vec3 *flat_normals; // normals for all triangles
vec3 *smooth_normals; // normals for all vertices

int sphere_NumVertices;
int Index = 0;
void colorsphere(void)
{
    string file_name;
    ifstream input_sphere;
    GLfloat x, y, z;
    int num_vertices;
    
    cout << "Input name of unit-sphere file: "<< endl;
    cin >> file_name;
    //file_name = "sphere.1024";
    //cout << file_name << endl;
    
    input_sphere.open(file_name, ios::in);
    if (!input_sphere.is_open()) {
        cout << "Open" << file_name << "failure" << endl;
        exit(EXIT_FAILURE);
    }
    
    input_sphere >> sphere_NumVertices;
    input_sphere >> num_vertices;
    sphere_NumVertices *= num_vertices;
    
    sphere_colors = new color4 [sphere_NumVertices];
    sphere_points = new point4 [sphere_NumVertices];
    flat_normals = new vec3 [sphere_NumVertices];
    smooth_normals = new vec3 [sphere_NumVertices];
    
    int r=0;
    vec4 u, v;
    vec3 normal;
    while ( !input_sphere.eof()) {
        if ( !(r % 4 == 0) ){
            input_sphere >> x >> y >> z;
            sphere_points[Index] = point4(x, y, z, 1.0);
            sphere_colors[Index] = sphere_color;
            smooth_normals[Index] = normalize(vec3(x, y, z));
            Index++;
        }
        else {
            input_sphere >> num_vertices;
            if (r > 0){
                u = sphere_points[Index-1] - sphere_points[Index-3];
                v = sphere_points[Index-2] - sphere_points[Index-3];
                normal = normalize( cross(v, u) );
                flat_normals[Index-1] = normal;
                flat_normals[Index-2] = normal;
                flat_normals[Index-3] = normal;
            }
        }
        r++;
        if (Index > sphere_NumVertices) break;
    }
    input_sphere.close();
}

//shadow-------------------------------------------------------------------------
color4 shadow_color = color4(0.25, 0.25, 0.25, 0.65);
point4 *shadow_points; // positions for shadow
color4 *shadow_colors; // colors for shadow
mat4 shadow_position;
void shadow(void)
{
    shadow_position = mat4(
                            vec4(light_source[1], -light_source[0], 0, 0),
                            vec4(0, 0, 0, 0),
                            vec4(0, -light_source[2], light_source[1], 0),
                            vec4(0, -1, 0, light_source[1])
                           );
    shadow_points = new point4 [sphere_NumVertices];
    shadow_colors = new color4 [sphere_NumVertices];
    for (int i=0; i<sphere_NumVertices; i++) {
        shadow_colors[i] = shadow_color;
        shadow_points[i] = sphere_points[i];
    }
}
//texture-----------------------------------------------------------------------------
/*--- Create checkerboard texture ---*/
#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];

/*--- Create stripe texture ---*/
#define    stripeImageWidth 32
GLubyte stripeImage[4*stripeImageWidth];

GLuint checkerBoard, stripe;
void image_set_up(void)
{
    int i, j, c;
    /* --- Generate checkerboard image to the image array ---*/
    for (i = 0; i < ImageHeight; i++)
    {
        for (j = 0; j < ImageWidth; j++)
        {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));
            if (c == 1) /* white */
            {
                c = 255;
                Image[i][j][0] = (GLubyte) c;
                Image[i][j][1] = (GLubyte) c;
                Image[i][j][2] = (GLubyte) c;
            }
            else  /* green */
            {
                Image[i][j][0] = (GLubyte) 0;
                Image[i][j][1] = (GLubyte) 150;
                Image[i][j][2] = (GLubyte) 0;
            }
            Image[i][j][3] = (GLubyte) 255;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /*--- Generate 1D stripe image to array stripeImage[] ---*/
    for (j = 0; j < stripeImageWidth; j++)
    {
        /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
         When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
         */
        stripeImage[4*j] = (GLubyte)    255;
        stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
        stripeImage[4*j+2] = (GLubyte) 0;
        stripeImage[4*j+3] = (GLubyte) 255;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//firework-----------------------------------------------------------------------------
const int Firework_Particles = 300;
vec3 fireworkColors[Firework_Particles];
vec3 fireworkVelocity[Firework_Particles];


void Firework(void)
{
    float Vx, Vy, Vz;
    float R, G, B;
    for ( int i = 0; i < Firework_Particles; i++ ) {
        Vx = 2.0 * ( (rand() % 256 ) / 256.0 - 0.5 );
        Vz = 2.0 * ( (rand() % 256 ) / 256.0 - 0.5 );
        Vy = 1.2 * 2.0 * ( (rand() % 256 ) / 256.0 );
        fireworkVelocity[i] = vec3( Vx, Vy, Vz );
        //cout<<fireworkVelocity[i]<<endl;
        
        
        R = ( rand() % 256 ) / 256.0;
        G = ( rand() % 256 ) / 256.0;
        B = ( rand() % 256 ) / 256.0;
        fireworkColors[i] = vec3( R, G, B );
        //cout<<fireworkColors[i]<<endl;
    }
}
//-----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    
// Create and initialize a texture object (checker board for floor)
    image_set_up();
    //checkerboard
    glGenTextures(1, &checkerBoard);      // Generate texture obj name(s)
    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_2D, checkerBoard); // Bind the texture to this texture unit
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
    
    //stripe
    glGenTextures(1, &stripe);      // Generate texture obj name(s)
    glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 0

    glBindTexture(GL_TEXTURE_1D, stripe);// Bind the texture to this texture unit
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

// Create and initialize a vertex buffer object for sphere, to be used in display()
    colorsphere();
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices + sizeof(color4)*sphere_NumVertices + 2 * sizeof(vec3) * sphere_NumVertices, NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, sphere_points); //position
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices, sphere_colors);//color
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, flat_normals);//flat shading
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices, sizeof(vec3)*sphere_NumVertices, smooth_normals);//smooth shading
    
// Create and initialize a vertex buffer object for shadow, to be used in display()
    shadow();
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, shadow_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices, shadow_colors);

// Create and initialize a vertex buffer object for floor with texture, to be used in display()
    floor();
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + 2 * sizeof(floor_normals) + sizeof(floor_texCoord), NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * floor_NumVertices, floor_points); //position
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices, sizeof(color4) * floor_NumVertices, floor_colors); // color
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices, sizeof(vec3) * floor_NumVertices, floor_normals); // shading (flat)
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices + sizeof(vec3) * floor_NumVertices, sizeof(vec3) * floor_NumVertices, floor_normals); // shading(smooth)
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices + sizeof(vec3) * floor_NumVertices * 2, sizeof(floor_texCoord), floor_texCoord); //texture
    
// Create and initialize a vertex buffer object for axes, to be used in display()
    axes();
    glGenBuffers(1, &axes_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axes_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axes_points) + sizeof(axes_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axes_points), axes_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axes_points), sizeof(axes_colors), axes_colors);

// Create and initialize a vertex buffer object for firework, to be used in display()
    Firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fireworkVelocity) + sizeof(fireworkColors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireworkVelocity), fireworkVelocity);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworkVelocity), sizeof(fireworkColors), fireworkColors);
    // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader53.glsl", "fshader53.glsl");
    pfirework = InitShader("vfirework.glsl", "ffirework.glsl");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
    glPointSize(3.0);
}


//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
//----------------------------------------------------------------------
void SetUp_Lighting_Uniform_Vars(mat4 mv)
{
    //glUniform4fv( glGetUniformLocation(program, "LookAt"), 1, mv );
    glUniform1i(glGetUniformLocation(program, "LightSourceType"), lightSourceFlag);
    
    glUniform4fv( glGetUniformLocation(program, "GlobalAmbient"), 1, global_ambient );
    glUniform4fv( glGetUniformLocation(program, "cAmbient"), 1, c_ambient );
    glUniform4fv( glGetUniformLocation(program, "cDiffuse"), 1, c_diffuse );
    glUniform4fv( glGetUniformLocation(program, "cSpecular"), 1, c_specular );
    
    glUniform4fv( glGetUniformLocation(program, "cLightDirection"), 1, c_direction);
    glUniform4fv( glGetUniformLocation(program, "dAmbient"), 1, d_ambient );
    glUniform4fv( glGetUniformLocation(program, "dDiffuse"), 1, d_diffuse );
    glUniform4fv( glGetUniformLocation(program, "dSpecular"), 1, d_specular );
    vec4 light_source_eye = mv * light_source; // to eye frame
    vec4 light_dest_eye = mv * light_dest;
    glUniform4fv( glGetUniformLocation(program, "dLightSource"), 1, light_source_eye );
    glUniform4fv( glGetUniformLocation(program, "dLightDest"), 1, light_dest_eye);
    
    glUniform1f(glGetUniformLocation(program, "Exponent"), exponent);
    glUniform1f(glGetUniformLocation(program, "CutoffAngle"), cutoff_angle);
    glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
    glUniform1f(glGetUniformLocation(program, "Shininess"), sphere_shininess );
}
void lighting_sphere() // transfer reflection parameters of sphere to shader
{
    glUniform4fv( glGetUniformLocation(program, "MaterialAmbient"), 1, sphere_ambient );
    glUniform4fv( glGetUniformLocation(program, "MaterialDiffuse"), 1, sphere_diffuse );
    glUniform4fv( glGetUniformLocation(program, "MaterialSpecular"), 1, sphere_specular );
}
void lighting_floor() // transfer reflection parameters of floor to shader
{
    glUniform4fv( glGetUniformLocation(program, "MaterialAmbient"), 1, floor_ambient );
    glUniform4fv( glGetUniformLocation(program, "MaterialDiffuse"), 1, floor_diffuse );
    glUniform4fv( glGetUniformLocation(program, "MaterialSpecular"), 1, floor_specular );
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices, num_ends):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices
//   num_ends = 2 for segment, num_ends = 3 for triangle
//   lighting = 1 for flat shading, lighting = 2 for smooth shading, lighting = 0 for disable lighting
void drawObj(GLuint buffer, int num_vertices, int num_ends, int lighting)
{
//--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    

/*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

/*----- Set up texture attribute arrays for ground-----*/

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices + sizeof(vec3) * floor_NumVertices * 2));
    
/*----- Set up normal attribute arrays when lighting is enable -----*/
    if (lighting == 0) {
        GLuint vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices) );

        /* Draw a sequence of geometric objs (triangles or segments) from the vertex buffer
         (using the attributes specified in each enabled vertex attribute array) */
        if (num_ends == 3) glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        else if (num_ends == 2) glDrawArrays(GL_LINES, 0, num_vertices);
        
        /*--- Disable each vertex attribute array being enabled ---*/
        glDisableVertexAttribArray(vPosition);
        glDisableVertexAttribArray(vColor);
    }
    else if (lighting == 1){
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET( sizeof(point4) * num_vertices + sizeof(color4) * num_vertices) );

        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        
        /*--- Disable each vertex attribute array being enabled ---*/
        glDisableVertexAttribArray(vPosition);
        glDisableVertexAttribArray(vNormal);
    }
    else if (lighting == 2){
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET( sizeof(point4) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices) );
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        
        glDisableVertexAttribArray(vPosition);
        glDisableVertexAttribArray(vNormal);
    }
}
//----------------------------------------------------------------------------
void drawFirework(GLuint buffer, int num_vertices){
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint fireworkVelocity = glGetAttribLocation(pfirework, "fireworkVelocity");
    glEnableVertexAttribArray(fireworkVelocity);
    glVertexAttribPointer(fireworkVelocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    GLuint fireworkColors = glGetAttribLocation(pfirework, "fireworkColors");
    glEnableVertexAttribArray(fireworkColors);
    glVertexAttribPointer(fireworkColors, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( sizeof(vec3) * num_vertices ));
    // the offset is the (total) size of the previous attribute array(s)
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_POINTS, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(fireworkVelocity);
    glDisableVertexAttribArray(fireworkColors);
}
//----------------------------------------------------------------------------
void display( void )
{
    GLuint  ModelView;  // model-view matrix uniform shader variable location
    GLuint  Projection;  // projection matrix uniform shader variable location
    GLuint  LightingType; // lighting flag uniform shader variable location
    GLuint  light_shade_type; //lighting and shading flag

    GLuint textureGroundSphereOrShadow; //1: ground; 2: sphere; 3: shadow; 0: others;
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram(program); // Use the shader program
    
    ModelView = glGetUniformLocation(program, "ModelView" );
    Projection = glGetUniformLocation(program, "Projection" );
    LightingType = glGetUniformLocation(program, "LightingType");

/*--- Pass on lattice options ---*/
    glUniform1i( glGetUniformLocation(program, "latticeOption"), latticeFlag );
/*--- Pass on fog options ---*/
    glUniform1i(glGetUniformLocation(program, "fogOptions"), fogFlag);
   
/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    //glUniformMatrix4fv(fProjection, 1, GL_TRUE, p);
    
/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4  at(0.0, 0.0, 0.0, 1.0);
    vec4  up(0.0, 1.0, 0.0, 0.0);
    mat4  mv = LookAt(eye, at, up);
    mat4  model_view = mv;
    
/*----- Set up the Mode-View matrix for the axes -----*/
    //glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
    //drawObj(axes_buffer, 6, 2);  // draw the axes
    model_view = mv * Translate(0, 0.02, 0.0);
    textureGroundSphereOrShadow = 0;
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
    
    glUniform1i( glGetUniformLocation(program, "groundSphereOrShadow"), textureGroundSphereOrShadow );
    glUniform1i(LightingType, 0); //lightingType = 0 : lighting is disable
    drawObj(axes_buffer, 6, 2, 0);  // emphasize the axes
    
/*----- Set Up the Model-View matrix for the sphere -----*/
    model_view = LookAt(eye, at, up) * Translate(vtranslation[0], vtranslation[1], vtranslation[2]) * Rotate(angle, vrotation[0], vrotation[1], vrotation[2]) * ACC_R;
    
    /**--- Set up texture ---**/
    textureGroundSphereOrShadow = 2;
    glUniform1i( glGetUniformLocation(program, "groundSphereOrShadow"), textureGroundSphereOrShadow );
    glUniform1i( glGetUniformLocation(program, "stripe"), 1 );
    glUniform1i( glGetUniformLocation(program, "sphereTexOption"), sphereTextureFlag );
    glUniform1i( glGetUniformLocation(program, "spaceOption"), whichSpace );
    
    /**--- Set up shade type ---**/
    if ( lightingFlag == 0 ) light_shade_type = lightingFlag;
    else light_shade_type = shadingFlag;
    if ( wireFlag == 0 ) light_shade_type = 0;
    SetUp_Lighting_Uniform_Vars(mv);
    lighting_sphere();
    
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
    
    glUniform1i(LightingType, light_shade_type); // lightingType = 0: lighting is disable; 1: flat shading; 2: smooth shading.
    if (wireFlag == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else if (wireFlag == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(sphere_buffer, sphere_NumVertices, 3, light_shade_type);  // draw the sphere
    
/*----- Set up the Mode-View matrix for the floor -----*/
    model_view = LookAt(eye, at, up);
    if (shadowFlag == 1 && eye[1] >= 0) glDepthMask(GL_FALSE); // disable writing floor to the z-buffer
    
    /*--- texture options ---*/
    textureGroundSphereOrShadow = 1;
    glUniform1i( glGetUniformLocation(program, "groundSphereOrShadow"), textureGroundSphereOrShadow );
    glUniform1i( glGetUniformLocation(program, "checkerBoard"), 0 );// Set the value of the fragment shader texture sampler variable
    glUniform1i( glGetUniformLocation( program, "groundTexOption"), groundTextureFlag );
    
    /**--- Set up shade type ---**/
    if (lightingFlag == 0) light_shade_type = lightingFlag;
    else light_shade_type = shadingFlag;
    SetUp_Lighting_Uniform_Vars(mv);
    lighting_floor();
    glUniform1i(LightingType, light_shade_type); // lightingType = 0: lighting is disable; 1: flat shading; 2: smooth shading.
    
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObj(floor_buffer, floor_NumVertices, 3, light_shade_type);  // draw the floor to frame buffer
    
    if (shadowFlag == 1 && eye[1] >= 0) {
/*----- Write shadow to frame buffer -----*/
        if (blendingShadowFlag == 1) {
            glEnable(GL_BLEND); // enable shadow blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    /*----- Set Up the shadow matrix for the shadow -----*/
        model_view = LookAt(eye, at, up) * shadow_position * Translate(vtranslation[0], vtranslation[1], vtranslation[2]) * Rotate(angle, vrotation[0], vrotation[1], vrotation[2]) * ACC_R;
        
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
        
        textureGroundSphereOrShadow = 3;
        glUniform1i( glGetUniformLocation(program, "groundSphereOrShadow"), textureGroundSphereOrShadow );
        glUniform1i(LightingType, 0); // lightingType = 0: lighting is disable
        if (wireFlag == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (wireFlag == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(shadow_buffer, sphere_NumVertices, 3, 0);  // draw the shadow to frame buffer
        
/*----- Write shadow to z-buffer -----*/
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to color buffer
        glDepthMask(GL_TRUE); // enable writing to the z-buffer
        drawObj(shadow_buffer, sphere_NumVertices, 3, 0);  // draw the shadow to z-buffer
        
        if (blendingShadowFlag == 1) glDisable(GL_BLEND); //disable blending
        
/*----- Write floor to z-buffer -----*/
        model_view = LookAt(eye, at, up);
        SetUp_Lighting_Uniform_Vars(mv);
        lighting_floor();
        glUniform1i(LightingType, light_shade_type); // lightingType = 0: lighting is disable; 1: flat shading; 2: smooth shading.
        
        /**--- texture options ---**/
        textureGroundSphereOrShadow = 1;
        glUniform1i( glGetUniformLocation(program, "groundSphereOrShadow"), textureGroundSphereOrShadow );
        glUniform1i( glGetUniformLocation(program, "checkerBoard"), 0 );// Set the value of the fragment shader texture sampler variable
        glUniform1i( glGetUniformLocation( program, "groundTexOption"), groundTextureFlag );
        
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view); // GL_TRUE: matrix is row-major
        
        glUniform1i(LightingType, light_shade_type);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj(floor_buffer, floor_NumVertices, 3, light_shade_type);  // draw the floor to z-buffer
        
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // enable writing to color buffer
    }
    
    if (fireworkFlag == 1) {
        glUseProgram(pfirework); // Use the shader program
        
        GLuint  MV_Firework;
        GLuint  P_Firework;

        MV_Firework = glGetUniformLocation(pfirework, "MV_Firework" );
        P_Firework = glGetUniformLocation(pfirework, "P_Firework" );
        
        glUniformMatrix4fv( P_Firework, 1, GL_TRUE, p);
        glUniformMatrix4fv( MV_Firework, 1, GL_TRUE, mv);

        float timeNow = glutGet(GLUT_ELAPSED_TIME);
        time_firework = timeNow - timeStart; // timeStart = the time of program beginning
        if (time_firework >= time_max) {
            timeStart = timeNow;
        }
        //cout<<time<<endl;
        glUniform1f( glGetUniformLocation(pfirework, "time"), time_firework );
        drawFirework(firework_buffer, Firework_Particles);
    }

    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    if (animationFlag == 0) glutPostRedisplay();
    else {
        angle += delta_angle;
        if (state % 3 == 0) {
            if (vtranslation[0] <= -2) {
                state++;
                accAngle = angle;
                ACC_R = Rotate(angle, vrotation[0], vrotation[1], vrotation[2]) * ACC_R;
                angle = 0;
                vrotation = Rbc;
            }
        }
        else if (state % 3 == 1) {
            if (vtranslation[0] >= 2) {
                state++;
                accAngle = angle;
                ACC_R = Rotate(angle, vrotation[0], vrotation[1], vrotation[2]) * ACC_R;
                angle = 0;
                vrotation = Rca;
            }
        }
        else if (state % 3 == 2) {
            if (vtranslation[0] >= 3) {
                state++;
                accAngle = angle;
                ACC_R = Rotate(angle, vrotation[0], vrotation[1], vrotation[2]) * ACC_R;
                angle = 0;
                vrotation = Rab;
            }
        }
        d_angle = angle / 360 * 2 * M_PI;
        if (state % 3 == 0) {
            vtranslation = OA + d_angle * normalize( OB - OA );
        }
        else if (state % 3 == 1) {
            vtranslation = OB + d_angle * normalize( OC - OB );
        }
        else if (state % 3 == 2) {
            vtranslation = OC + d_angle * normalize( OA - OC );
        }
        glutPostRedisplay();
    }
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 'b': case 'B': // begin rolling
            if (beginkey == 0 && animationFlag == 0) {
                //glutIdleFunc(idle);
                beginkey = 1;
                animationFlag = 1;
            }
            break;
            
        case 'X': eye[0] += 1.0; break;
        case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
        case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
        case 'z': eye[2] -= 1.0; break;
            
        case 'v': case 'V':
            if (sphereTextureFlag > 0) {
                if ( sphereTextureFlag < 3) sphereTextureFlag = 1;
                else sphereTextureFlag = 3;
                if (latticeFlag > 0) latticeFlag = 1;
            }
            break;
        case 's': case 'S':
            if (sphereTextureFlag > 0) {
                if (sphereTextureFlag < 3) sphereTextureFlag = 2;
                else sphereTextureFlag = 4;
                if (latticeFlag > 0) latticeFlag = 2;
            }
            break;
        case 'o': case 'O':
            whichSpace = 0; break;
        case 'e': case 'E':
            whichSpace = 1; break;
            
        case 'l': case 'L':
            if (latticeFlag == 0 ) {
                latticeFlag = 1;
                if (sphereTextureFlag > 0  && sphereTextureFlag < 3) {
                    sphereTextureFlag = 1;
                }
                else if (sphereTextureFlag >= 3) {
                    sphereTextureFlag = 3;
                }
            }
            else latticeFlag = 0;
            break;
        case 'u': case 'U':
            if ( latticeFlag > 0 ) {
                latticeFlag = 1;
                if (sphereTextureFlag > 0  && sphereTextureFlag < 3) {
                    sphereTextureFlag = 1;
                }
                else if (sphereTextureFlag >= 3) {
                    sphereTextureFlag = 3;
                }
            }
            break;
        case 't': case 'T':
                if ( latticeFlag > 0 ) {
                    latticeFlag = 2;
                    if (sphereTextureFlag > 0  && sphereTextureFlag < 3) {
                        sphereTextureFlag = 2;
                    }
                    else if (sphereTextureFlag >= 3) {
                        sphereTextureFlag = 4;
                    }
                }
            break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void mouse(int button, int mstate, int x, int y)
{
    if(button == GLUT_RIGHT_BUTTON && mstate == GLUT_DOWN){
        if (beginkey == 1){
            animationFlag = 1 - animationFlag;
//            if (animationFlag == 1) glutIdleFunc(idle);
//            else glutIdleFunc(NULL);
        }
    }
}
//----------------------------------------------------------------------------
void menu (int id)
{
    switch (id) {
        case 1:
            eye = init_eye;
            break;
        case 2:
            wireFlag = 1 - wireFlag;
            if (wireFlag == 0) sphereTextureFlag = 0;
            break;
        case 3:
            exit(0);
            break;
    }
    glutPostRedisplay();
}
void shadow_menu (int id)
{
    switch (id) {
        case 1:
            shadowFlag = 0;
            break;
        case 2:
            shadowFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void blend_shadow_menu (int id)
{
    switch (id) {
        case 1:
            blendingShadowFlag = 0;
            break;
        case 2:
            blendingShadowFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void lighting_menu (int id)
{
    switch (id) {
        case 1:
            lightingFlag = 0;
            break;
        case 2:
            lightingFlag = 1;
            wireFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void shading_menu (int id)
{
    wireFlag = 1;
    switch (id) {
        case 1:
            shadingFlag = 1;
            break;
        case 2:
            shadingFlag = 2;
            break;
    }
    glutPostRedisplay();
}
void light_source_menu (int id)
{
    switch (id) {
        case 1:
            lightSourceFlag = 1;
            break;
        case 2:
            lightSourceFlag = 2;
            break;
    }
    glutPostRedisplay();
}

void fog_options_menu (int id)
{
    switch (id) {
        case 0:
            fogFlag = 0;
            break;
        case 1:
            fogFlag = 1;
            break;
        case 2:
            fogFlag = 2;
            break;
        case 3:
            fogFlag = 3;
            break;
    }
    glutPostRedisplay();
}
void texture_ground_menu (int id)
{
    switch (id){
        case 1:
            groundTextureFlag = 0;
            break;
        case 2:
            groundTextureFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void texture_sphere_menu (int id)
{
    switch (id){
        case 1:
            sphereTextureFlag = 0;
            break;
        case 2:
            sphereTextureFlag = 1;
            if (latticeFlag > 0 ) latticeFlag = 1;
            break;
        case 3:
            sphereTextureFlag = 3;
            if (latticeFlag > 0) latticeFlag = 1;
            break;
    }
    glutPostRedisplay();
}
void firework_menu (int id)
{
    switch (id){
        case 1:
            fireworkFlag = 0;
            break;
        case 2:
            if (fireworkFlag == 0) {
                fireworkFlag = 1;
                time_firework = 0;
                timeStart = glutGet(GLUT_ELAPSED_TIME);
            }
            fireworkFlag = 1;
            break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Rolling Sphere");
    //uint id = glutCreateWindow("secend window");
    
#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    {
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err));
        exit(1);
    }
#endif
    
    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    
    //set up menu
    int myMenu, shadowMenu, bShadowMenu, lightingMenu, shadingMenu, lightSourceMenu, fogMenu, texGroundMenu, texSphereMenu, fireworkMenu;
    
    shadowMenu = glutCreateMenu(shadow_menu); // set up submenu: Shadow
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    bShadowMenu = glutCreateMenu(blend_shadow_menu);
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    lightingMenu = glutCreateMenu(lighting_menu); // set up submenu: Enable Lighting
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    shadingMenu = glutCreateMenu(shading_menu); // set up submenu: Shading
    glutAddMenuEntry("flat shading", 1);
    glutAddMenuEntry("smooth shading", 2);
    
    lightSourceMenu = glutCreateMenu(light_source_menu); // set up submenu: Light Source
    glutAddMenuEntry("spot light", 2);
    glutAddMenuEntry("point light", 1);
    
    fogMenu = glutCreateMenu(fog_options_menu); //set up submenu: Fog Options
    glutAddMenuEntry("no fog", 0);
    glutAddMenuEntry("linear", 1);
    glutAddMenuEntry("exponential", 2);
    glutAddMenuEntry("exponential square", 3);
    
    texGroundMenu = glutCreateMenu(texture_ground_menu); //set up submenu: Textrue Mapped Ground
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    texSphereMenu = glutCreateMenu(texture_sphere_menu); //set up submenu: Textrue Mapped Sphere
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes - Contour Lines", 2);
    glutAddMenuEntry("Yes - Checkerboard", 3);
    
    fireworkMenu = glutCreateMenu(firework_menu); //set up submenu: Firework
    glutAddMenuEntry("No", 1);
    glutAddMenuEntry("Yes", 2);
    
    myMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddSubMenu("Shadow", shadowMenu);
    glutAddSubMenu("Blending Shadow", bShadowMenu);
    glutAddSubMenu("Enable Lighting", lightingMenu);
    glutAddMenuEntry("Wire Frame Sphere", 2);
    glutAddSubMenu("Shading", shadingMenu);
    glutAddSubMenu("Light Source", lightSourceMenu);
    glutAddSubMenu("Fog Options", fogMenu);
    glutAddSubMenu("Texture Mapped Ground", texGroundMenu);
    glutAddSubMenu("Texture Mapped Sphere", texSphereMenu);
    glutAddSubMenu("Firework", fireworkMenu);
    glutAddMenuEntry("Quit", 3);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    
    init();
    timeStart = glutGet(GLUT_ELAPSED_TIME);
    glutMainLoop();
    return 0;
}
