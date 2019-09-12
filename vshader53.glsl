/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
in  vec2 vTexCoord;

out vec4 color;
out float z;
out vec2 groundTexCoord;
out vec2 sphereTexCoord;
out vec2 latticeCoord;

uniform mat4 ModelView;
uniform mat4 Projection;

uniform int groundTexOption;
uniform int sphereTexOption;
uniform int spaceOption;
uniform int latticeOption;

uniform int LightingType;
uniform int LightSourceType;

uniform vec4 GlobalAmbient;
uniform vec4 MaterialAmbient, MaterialDiffuse, MaterialSpecular;
uniform vec4 cAmbient, cDiffuse, cSpecular;
uniform vec4 dAmbient, dDiffuse, dSpecular;
uniform vec4 cLightDirection;
uniform vec4 dLightSource, dLightDest;

uniform float Exponent;
uniform float CutoffAngle;
uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

uniform float Shininess;

void main()
{
    sphereTexCoord = vec2(0,0);
    gl_Position = Projection * ModelView * vPosition;
    
    vec3 pos = (ModelView * vPosition).xyz;
    vec3 posTex = vPosition.xyz ;
    
    z=length(pos);
    groundTexCoord = vTexCoord;
    
    if ( sphereTexOption > 0) {
        if ( spaceOption == 0 ) posTex = vPosition.xyz;
        else if ( spaceOption == 1 ) posTex = pos;
    
        if ( sphereTexOption == 1 ) {
            sphereTexCoord[0] = 2.5 * posTex[0];
        }
        else if ( sphereTexOption == 2 ) {
            sphereTexCoord[0] = 1.5 * (posTex[0] + posTex[1] + posTex[2]);
        }
        else if ( sphereTexOption == 3 ) {
            sphereTexCoord[0] = 0.5 * ( posTex[0] + 1 );
            sphereTexCoord[1] = 0.5 * ( posTex[1] + 1 );
        }
        else if ( sphereTexOption == 4 ) {
            sphereTexCoord[0] = 0.3 * ( posTex[0] + posTex[1] + posTex[2] );
            sphereTexCoord[1] = 0.3 * ( posTex[0] - posTex[1] + posTex[2] );
        }
    }
    
    if ( latticeOption == 1 ){
//    if ( spaceOption == 0 && latticeOption == 1 ){
        latticeCoord[0] = 0.5 * ( posTex[0] + 1 );
        latticeCoord[1] = 0.5 * ( posTex[1] + 1 );
    }
    else if ( latticeOption == 2 ){
//    else if ( spaceOption == 0 && latticeOption == 2 ){
        latticeCoord[0] = 0.3 * ( posTex[0] + posTex[1] + posTex[2] );
        latticeCoord[1] = 0.3 * ( posTex[0] - posTex[1] + posTex[2] );
    }
    
    
    if (LightingType == 0) {
        color = vColor;
    }
    else{
        vec3 V = normalize( -pos ); //vector from vertex to viewer
        vec3 N = normalize( ModelView * vec4(vNormal, 0.0) ).xyz; // normal vector of vertex
        // YJC Note: N must use the one pointing *toward* the viewer
        //     ==> If (N dot E) < 0 then N must be changed to -N
        //
        if ( dot(N, V) < 0 ) N = -N;
        
        vec4 globalAmbientProduct = GlobalAmbient * MaterialAmbient;
        
        vec3 cL = normalize( -cLightDirection.xyz );
        vec3 cH = normalize( cL + V );
        float c_attenuation = 1.0;

        vec4 cAmbientProduct = cAmbient * MaterialAmbient;
        
        float c_diffuse = max( dot( cL, N ), 0.0 );
        vec4 cDiffuseProduct = c_diffuse * cDiffuse * MaterialDiffuse;
        
        float c_specular = pow( max(dot(N, cH), 0.0), Shininess );
        vec4  cSpecularProduct = c_specular * cSpecular * MaterialSpecular;
        if( dot( cL, N ) < 0.0 ) {
            cSpecularProduct = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        if ( LightSourceType == 0 ) { // no positional light
            color = globalAmbientProduct + c_attenuation * (cAmbientProduct + cDiffuseProduct + cSpecularProduct);
        }
        else {
            vec3 dLightS = dLightSource.xyz;
            vec3 dLightD = dLightDest.xyz;
            vec3 dLightDirection = normalize( dLightD - dLightS );
            vec3 dL = normalize( dLightS - pos );
            vec3 dH = normalize( dL + V );
            float D = sqrt( dot( (dLightS - pos), (dLightS - pos) ) ); // distance from vertex to light source
            
            float d_attenuation;
            d_attenuation = 1 / ( ConstAtt + LinearAtt * D + QuadAtt * D * D ); // point source
            
            vec4 dAmbientProduct = dAmbient * MaterialAmbient;
            
            float d_diffuse = max( dot( dL, N ), 0.0 );
            vec4  dDiffuseProduct = d_diffuse * dDiffuse * MaterialDiffuse;
            
            float d_specular = pow( max(dot(N, dH), 0.0), Shininess );
            vec4  dSpecularProduct = d_specular * dSpecular * MaterialSpecular;
            if( dot( dL, N ) < 0.0 ) {
                dSpecularProduct = vec4(0.0, 0.0, 0.0, 1.0);
            }
            
            
            if ( LightSourceType == 2 ) { // spot light
                float cos_phi = dot( ( -dL ), dLightDirection );
                float cos_theta = cos( CutoffAngle );
                float dot_l_n = dot(dL, N);
                if ( dot_l_n < 0) d_attenuation = 0;
                else if (cos_phi < cos_theta) d_attenuation = 0;
                else d_attenuation *= pow( dot( ( -dL ), dLightDirection ), Exponent );
            }
            color = globalAmbientProduct + c_attenuation * (cAmbientProduct + cDiffuseProduct + cSpecularProduct) + d_attenuation * (dAmbientProduct + dDiffuseProduct + dSpecularProduct);
        }
    }
}
