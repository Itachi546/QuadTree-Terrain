#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 7) out;

// Input 
layout(location = 0) in vec3 gnormal[1];
layout(location = 1) in vec3 viewDir[1];
layout(location = 2) in mat4 gVP[1];


// Output
layout(location = 0) out vec3 fnormal;
layout(location = 1) out vec2 fuv;
layout(location = 2) out float noise;
layout(location = 3) out vec3 fviewDir;
layout(push_constant) uniform block
{
   layout(offset = 64) float time;
};


layout(binding = 4) uniform sampler2D u_distortionTexture;
layout(binding = 5) uniform sampler2D u_noiseTexture;

#define PI  3.1415926535897932384626433832795

// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotate(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

float rand(vec3 co){
    return fract(sin(dot(co, vec3(12.9898, 78.233, 153.295))) * 43758.5453);
}

const float GRASS_HEIGHT = 0.4 * 8.0;
const float GRASS_WIDTH  = 0.04 * 8.0;

// Segment length at top, middle and bottom
const float slB = 0.1;
const float slM = 0.4;
const float slT = 0.5;

mat4 generate_transform(mat4 rotation, mat4 bendRotation, mat4 windRotation)
{
 return windRotation * rotation * bendRotation;
}

void main()
{
   vec4 origin = gl_in[0].gl_Position;
   vec3 normal = normalize(gnormal[0]);

    float slope	= 1.0f - normal.y;
	if(slope > 0.2f || origin.y > 0.0f)
      return;

   vec3 helper = vec3(0.0, 1.0, 0.0);
   if(abs(normal.y) > 0.99)
      helper = vec3(0.0, 0.0, 1.0);
   vec3 tangent = normalize(cross(helper, normal));

   vec3 bitangent = normalize(cross(normal, tangent));

   float ang = rand(origin.xyz);
   mat4 rotation = rotate(vec3(0.0, 1.0, 0.0), ang * PI * 2.0);


   float height = 0.0;
   const float maxOffset = 0.1;
   origin.xyz += vec3(cos(ang) * maxOffset, 0.0, sin(ang) * maxOffset);

   vec2 windFrequency = vec2(5.0, 5.);
   float windSpeed = 0.02;
   float windIntensity = 0.5;
   vec2 uv = (origin.xz * 0.1 + windFrequency * time) * windSpeed;
   vec2 windSample = (texture(u_distortionTexture, uv).rg * 2.0 - 1.0) * PI * windIntensity;
   vec3 wind = normalize(vec3(windSample, 0.0));
   mat4	distortionRotation = rotate(wind, windSample.x) * rotate(wind,	windSample.y);

   vec3 faceNormal = mat3(inverse(transpose(rotation))) * bitangent;

   float grassHeight = GRASS_HEIGHT * max(rand(normal.xyz), 0.5f);
   // Ground 
   gl_Position = gVP[0]	*  (origin + rotation * vec4(GRASS_WIDTH * 0.5 * tangent, 0.0));
   fuv = vec2(1.0, 0.0);
   fnormal  = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   gl_Position = gVP[0]	*  (origin - rotation * vec4(GRASS_WIDTH * 0.5 * tangent, 0.0));
   fuv = vec2(0.0, 0.0);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   // First
   float bendAng = rand(origin.zzx) * 2.0 - 1.0;
   mat4 bendRotation = rotate(vec3(-1.0, 0.0, 0.0), bendAng * PI * 0.1);
   mat4 transform = generate_transform(rotation, bendRotation, mat4(1.0));
   height += grassHeight * slB;
   faceNormal = mat3(inverse(transpose(transform))) * bitangent;

   gl_Position = gVP[0]	*  (origin + transform * vec4(GRASS_WIDTH * 0.5 * tangent + height * normal, 0.0));
   fuv = vec2(1.0, slB);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   gl_Position = gVP[0]	*  (origin + transform * vec4(-GRASS_WIDTH * 0.5 * tangent + height * normal, 0.0));
   fuv = vec2(0.0, slB);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   //Second
   bendRotation =  rotate(vec3(-1.0, 0.0, 0.0), bendAng * PI * 0.12);
   transform = generate_transform(rotation, bendRotation, distortionRotation);
   height += grassHeight * slM;
   faceNormal = mat3(inverse(transpose(transform))) * bitangent;

   gl_Position = gVP[0]	*  (origin + transform * vec4(GRASS_WIDTH * 0.5 * tangent + height * normal, 0.0));
   fuv = vec2(1.0, slM + slB);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   gl_Position = gVP[0]	*  (origin + transform * vec4(-GRASS_WIDTH * 0.5 * tangent + height * normal, 0.0));
   fuv = vec2(0.0, slM + slB);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   // top
   bendRotation =  rotate(vec3(-1.0, 0.0, 0.0), bendAng * PI * 0.14);
   transform = generate_transform(rotation, bendRotation, distortionRotation);
   height += slT * grassHeight;
   faceNormal = mat3(inverse(transpose(transform))) * bitangent;

   gl_Position = gVP[0]	*  (origin + transform * vec4(height * normal, 0.0));
   fuv = vec2(0.5, 1.0);
   fnormal = faceNormal;
   fviewDir = viewDir[0];
   EmitVertex();

   EndPrimitive();
}

/*
   noise = texture(u_noiseTexture, position.xz * 0.01).r;

   vec3 normal = normalize(gnormal[0]);
   vec3 helper = vec3(0.0, 1.0, 0.0);
   if(abs(normal.y) > 0.99)
      helper = vec3(0.0, 0.0, 1.0);

   vec3 t = normalize(cross(helper, normal));
   vec3 bt = normalize(cross(normal, t));

   mat3 tnb = mat3(t, normal, bt);


   float ang = rand(position.xyz);
   mat3 rotation = mat3(rotate(vec3(0.0, 1.0, 0.0), ang * PI * 2.0));

   float bendAng = rand(position.zzx);
   mat3 bendRotation =  mat3(rotate(vec3(-1.0, 0.0, 0.0), bendAng * PI * 0.2));

   vec2 windFrequency = vec2(0.0, 0.06);
   vec2 uv = position.xz * 0.01 + windFrequency * time;

   float windSpeed = 0.3;
   vec2 windSample = (texture(u_distortionTexture, uv).rg * 2.0 - 1.0) * PI * windSpeed;
   //vec3 wind = normalize(vec3(windSample, 0.0));
   //mat3	distortionRotation = mat3(rotate(wind, windSample.x) * rotate(wind,	windSample.y));

   tnb = tnb * rotation;

   const float maxOffset = 0.1;
   position.xyz += tnb * vec3(cos(ang) * maxOffset, 0.0, sin(ang) * maxOffset);

   float size =  rand(vec3(ang, 0.0, bendAng));
   float width = 0.04 + 0.02 * size;
   float height = 0.3 + 0.2 * size;

   //float2 uv = pos.xz *	_WindDistortionMap_ST.xy + _WindDistortionMap_ST.zw	+ _WindFrequency * _Time.y;

   gl_Position = gVP[0] *  (position + vec4(tnb * vec3(-width, 0.0, 0.0),	0.0));
   fuv = vec2(0.0, 0.0);
   EmitVertex();
   gl_Position = gVP[0] * (position + vec4(tnb * vec3(windSample.x, height, windSample.y), 0.0));
   fuv = vec2(0.5, 1.0);
   EmitVertex();
   gl_Position = gVP[0] * (position + vec4(tnb * vec3(width, 0.0, 0.0), 0.0));
   fuv = vec2(1.0, 0.0);
   EmitVertex();
   EndPrimitive();

*/