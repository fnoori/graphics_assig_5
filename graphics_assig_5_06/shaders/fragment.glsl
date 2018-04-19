// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

in vec2 TextureCoords;

// interpolated colour received from vertex stage
uniform vec3 Colour;
uniform sampler2D textureImage_one;

// for shading
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

in vec3 Normals;
in vec3 FragmentPosition;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

void main(void)
{
    vec3 colour = texture(textureImage_one, TextureCoords).rgb;
    
    vec3 ambient = 1 * colour;
    
    vec3 lightDirection = normalize(lightPosition - FragmentPosition);
    vec3 normal = normalize(Normals);
    float difference = max(dot(lightDirection, normal), 0.5);
    vec3 diffuse = difference * colour;
    
    vec3 viewDirection = normalize(cameraPosition - FragmentPosition);
    vec3 reflectionDirection = reflect(lightDirection, normal);
    
    float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), 8.0);
    
    vec3 specular = vec3(1.) * spec;
    
    FragmentColour = vec4((ambient + diffuse + specular) * colour, 1.0);
    
    
    /*
    vec4 newColour = texture(textureImage_one, TextureCoords);
    
    // write colour output without modification
    FragmentColour = newColour;
     */
}
