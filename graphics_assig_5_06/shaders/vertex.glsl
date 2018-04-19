// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 TexturePosition;
layout(location = 2) in vec3 NormalPosition;

uniform mat4 modelViewProjection;
uniform mat4 transform;

out vec2 TextureCoords;
out vec3 Normals;
out vec3 FragmentPosition;

void main()
{
    // assign vertex position without modification
    gl_Position = modelViewProjection * transform * vec4(VertexPosition, 1.0);
    
    TextureCoords = TexturePosition;
    Normals = NormalPosition;
    FragmentPosition = VertexPosition;
}
