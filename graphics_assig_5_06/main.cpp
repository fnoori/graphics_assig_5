// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//            Jeremy Hart, University of Calgary
//            John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"
#include "Camera.h"
#include "objectReader.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

#define PI_F 3.14159265359f

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

bool lbPushed = false;

ObjectReader sunObject;
ObjectReader earthObject;
ObjectReader moonObject;
ObjectReader starsBackdropObject;

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint  textureBuffer;
    GLuint  colourBuffer;
    GLuint  normalBuffer;
    
    GLuint  vertexArray;
    GLsizei elementCount;
    
    // initialize object names to zero (OpenGL reserved value)
    Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
    {}
};

struct CelestialBodies
{
    Geometry geometry;
    MyTexture myTexture;
    vector<vec2> textureCoords;
    vector<vec3> vertices;
    vector<vec3> normals;
    float scaleBy;
    
    CelestialBodies *orbitAround;
    mat4 transformBy = mat4(1.f);
    mat4 translateBy = mat4(1.f);
    mat4 rotateBy = mat4(1.f);
};

CelestialBodies sun;
CelestialBodies earth;
CelestialBodies moon;
CelestialBodies backdrop;

Camera cam;
vec3 movement(0.f);
vec3 lightSource = vec3(0.f, 0.f, 0.f);
float scrollDir;
bool pauseAnim = false;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
    // load shader source from files
    string vertexSource = LoadSource("shaders/vertex.glsl");
    string fragmentSource = LoadSource("shaders/fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return false;
    
    // compile shader source into shader objects
    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    // link shader program
    GLuint program = LinkProgram(vertex, fragment);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // check for OpenGL errors and return false if error occurred
    return program;
}

bool InitializeVAO(Geometry *geometry){
    
    const GLuint VERTEX_INDEX = 0;
    const GLuint TEXTURE_INDEX = 1;
    const GLuint NORMAL_INDEX = 2;
    
    //Generate Vertex Buffer Objects
    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glGenBuffers(1, &geometry->textureBuffer);
    glGenBuffers(1, &geometry->normalBuffer);
    
    //Set up Vertex Array Object
    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);
    
    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(
        VERTEX_INDEX,        //Attribute index
        3,                     //# of components
        GL_FLOAT,             //Type of component
        GL_FALSE,             //Should be normalized?
        sizeof(vec3),        //Stride - can use 0 if tightly packed
        0);                    //Offset to first element
    glEnableVertexAttribArray(VERTEX_INDEX);
    
    // texture buffer
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
    glVertexAttribPointer(
        TEXTURE_INDEX,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vec2),
        0);
    glEnableVertexAttribArray(TEXTURE_INDEX);
    
    // normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
    glVertexAttribPointer(
                          NORMAL_INDEX,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(vec3),
                          0);
    glEnableVertexAttribArray(NORMAL_INDEX);
    
    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, vector<vec3> vertices, vector<vec2> textureCoords, vector<vec3> normals, int elementCount)
{
    geometry->elementCount = elementCount;
    
    // create an array buffer object for storing our vertices
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*geometry->elementCount, &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*textureCoords.size(), &textureCoords[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*normals.size(), &normals[0], GL_STATIC_DRAW);
    
    //Unbind buffer to reset to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, MyTexture *texture, GLuint program, Camera* camera, mat4 perspectiveMatrix, GLenum rendermode, mat4 transformVertice)
{
    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(program);
    
    mat4 modelViewProjection = perspectiveMatrix*camera->viewMatrix();
    GLint uniformLocation = glGetUniformLocation(program, "modelViewProjection");
    glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(modelViewProjection));
    
    unsigned int transformLoc = glGetUniformLocation(program, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transformVertice));
    
    unsigned int lightPos = glGetUniformLocation(program, "lightPosition");
    glUniform3f(lightPos, lightSource.x, lightSource.y, lightSource.z);
    
    unsigned int camPos = glGetUniformLocation(program, "cameraPosition");
    glUniform3f(camPos, cam.getPosition().x, cam.getPosition().y, cam.getPosition().z);
    
    glBindVertexArray(geometry->vertexArray);
    glBindTexture(texture->target, texture->textureID);
    glDrawArrays(rendermode, 0, geometry->elementCount);
    
    // reset state to default (no shader or geometry bound)
    glBindTexture(texture->target, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    
    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset < 0) {
        scrollDir = -1.f;
    } else {
        scrollDir = 1.f;
    }
}


// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);
    
    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 920, height = 680;
    window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }
    
    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    
    //Intialize GLAD
    if (!gladLoadGL())
    {
        cout << "GLAD init failed" << endl;
        return -1;
    }
    
    // query and print out information about our OpenGL environment
    QueryGLVersion();
    
    // call function to load and compile shader programs
    GLuint program = InitializeShaders();
    if (program == 0) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    starsBackdropObject.findSphere("sphere.obj");
    starsBackdropObject.processData();
    const char* starsTexturePath = "celestialBodyTextures/stars.jpg";
    
    sunObject.findSphere("sphere.obj");
    sunObject.processData();
    const char* sunTexturePath = "celestialBodyTextures/sun.jpg";
    
    earthObject.findSphere("sphere.obj");
    earthObject.processData();
    const char* earthTexturePath = "celestialBodyTextures/earth.jpg";
    
    moonObject.findSphere("sphere.obj");
    moonObject.processData();
    const char* moonTexturePath = "celestialBodyTextures/moon.jpg";
    
    vector<vec3> tmp;
    tmp.push_back(vec3(0,0,0));
    backdrop.vertices = starsBackdropObject.getVertices();
    backdrop.textureCoords = starsBackdropObject.getUvs();
    backdrop.normals = tmp;
    
    sun.vertices = sunObject.getVertices();
    sun.textureCoords = sunObject.getUvs();
    sun.normals = sunObject.getNormals();
    
    earth.vertices = earthObject.getVertices();
    earth.textureCoords = earthObject.getUvs();
    earth.normals = earthObject.getNormals();
    
    moon.vertices = moonObject.getVertices();
    moon.textureCoords = moonObject.getUvs();
    moon.normals = moonObject.getNormals();

    mat4 perspectiveMatrix = perspective(PI_F*0.4f, float(width)/float(height), 0.1f, 20.f);    //Fill in with Perspective Matrix
    
    Geometry geometry;
    
    if (!InitializeVAO(&backdrop.geometry))
    cout << "Program failed to intialize geometry!" << endl;
    if (!LoadGeometry(&backdrop.geometry, backdrop.vertices, backdrop.textureCoords, backdrop.normals, backdrop.vertices.size()))
    cout << "Failed to load geometry" << endl;
    if (!InitializeTexture(&backdrop.myTexture, starsTexturePath)) {
        cout << "Program failed to initialize texture!" << endl;
    }
    backdrop.transformBy = scale(backdrop.transformBy, vec3(10.f, 10.f, 10.f));
    
    // call function to create and fill buffers with geometry data
    if (!InitializeVAO(&sun.geometry))
        cout << "Program failed to intialize geometry!" << endl;
    if (!LoadGeometry(&sun.geometry, sun.vertices, sun.textureCoords, sun.normals, sun.vertices.size()))
        cout << "Failed to load geometry" << endl;
    if (!InitializeTexture(&sun.myTexture, sunTexturePath)) {
        cout << "Program failed to initialize texture!" << endl;
    }
    sun.transformBy = rotate(sun.transformBy, radians(180.f), vec3(1, 0, 0));
    
    if (!InitializeVAO(&earth.geometry))
        cout << "Program failed to intialize geometry!" << endl;
    if (!LoadGeometry(&earth.geometry, earth.vertices, earth.textureCoords, earth.normals, earth.vertices.size()))
        cout << "Failed to load geometry" << endl;
    if (!InitializeTexture(&earth.myTexture, earthTexturePath)) {
        cout << "Program failed to initialize texture!" << endl;
    }
    earth.transformBy = rotate(earth.transformBy, radians(23.5f), vec3(1, 0, 0));
    earth.transformBy = translate(earth.transformBy, vec3(3, 0, 0));
    earth.transformBy = scale(earth.transformBy, vec3(0.5f, 0.5f, 0.5f));
    
    if (!InitializeVAO(&moon.geometry))
        cout << "Program failed to intialize geometry!" << endl;
    if (!LoadGeometry(&moon.geometry, moon.vertices, moon.textureCoords, moon.normals, moon.vertices.size()))
        cout << "Failed to load geometry" << endl;
    if (!InitializeTexture(&moon.myTexture, moonTexturePath)) {
        cout << "Program failed to initialize texture!" << endl;
    }
    moon.transformBy = rotate(moon.transformBy, radians(23.5f), vec3(1, 0, 0));
    moon.transformBy = translate(moon.transformBy, vec3(3.2, 0, 0));
    moon.transformBy = scale(moon.transformBy, vec3(0.5f, 0.5f, 0.5f));
    
    vec2 lastCursorPos;
    float scrollSpeed = 2.f;
    
    float cursorSensitivity = PI_F/200.f;    //PI/hundred pixels
    float movementSpeed = 0.01f;
    
    mat4 modelMatrix = mat4(1.f);
    vec3 translateVal = vec3(1, 0, 0);
    
    float angle = 0.f;
    float speed = 1.f;
    
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        movement = vec3(0.f);
        
        // zoom level
        if (scrollDir < 0) {
            movement.z -= 10.f;
        } else if (scrollDir > 0) {
            movement.z += 10.f;
        }
        scrollDir = 0;
        
        // speed up and slow down
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            if (speed > 3.f) {
                speed = 3.f;
            } else {
                speed += 0.1f;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (speed < 1.f) {
                speed = 1.f;
            } else {
                speed -= 0.1f;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            pauseAnim = true;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            pauseAnim = false;
        }
        
        // moving camera
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        vec2 cursorPos (xpos, ypos);
        vec2 cursorChange = cursorPos - lastCursorPos;
        
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
            cam.rotateHorizontal(cursorChange.y*cursorSensitivity);
            cam.rotateVertical(cursorChange.x*cursorSensitivity);
        }
        
        lastCursorPos = cursorPos;
        cam.zoom(movement*movementSpeed);

        // draw scene
        
        // clear screen to a dark grey colour
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        RenderScene(&backdrop.geometry, &backdrop.myTexture, program, &cam, perspectiveMatrix, GL_TRIANGLES, backdrop.transformBy);
        
        if (!pauseAnim) {
            sun.transformBy = rotate(sun.transformBy, radians(.5f)*speed, vec3(0, -1, 0));
        }
        
        RenderScene(&sun.geometry, &sun.myTexture, program, &cam, perspectiveMatrix, GL_TRIANGLES, sun.transformBy);
         
        earth.translateBy = translate(earth.transformBy, vec3(0, 0, 1.f));
        earth.rotateBy = rotate(earth.transformBy, radians(2.f), vec3(0, 1, 0));
        modelMatrix = sun.transformBy * earth.translateBy * earth.rotateBy;
        RenderScene(&earth.geometry, &earth.myTexture, program, &cam, perspectiveMatrix, GL_TRIANGLES, modelMatrix);
        
        moon.translateBy = translate(moon.transformBy, vec3(0, 0, 1.f));
        moon.rotateBy = rotate(moon.transformBy, radians(2.5f), vec3(0, 1, 0));
        modelMatrix = (sun.transformBy * earth.translateBy * earth.rotateBy) *
                    sun.transformBy * earth.transformBy * moon.rotateBy * moon.translateBy;
        RenderScene(&moon.geometry, &moon.myTexture, program, &cam, perspectiveMatrix, GL_TRIANGLES, modelMatrix);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // clean up allocated resources before exit
    DestroyGeometry(&sun.geometry);
    DestroyGeometry(&earth.geometry);
    DestroyGeometry(&moon.geometry);
    glUseProgram(0);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    
    cout << "OpenGL [ " << version << " ] "
    << "with GLSL [ " << glslver << " ] "
    << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
            case GL_INVALID_ENUM:
                cout << "GL_INVALID_ENUM" << endl; break;
            case GL_INVALID_VALUE:
                cout << "GL_INVALID_VALUE" << endl; break;
            case GL_INVALID_OPERATION:
                cout << "GL_INVALID_OPERATION" << endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
            case GL_OUT_OF_MEMORY:
                cout << "GL_OUT_OF_MEMORY" << endl; break;
            default:
                cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;
    
    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
        << filename << endl;
    }
    
    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);
    
    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);
    
    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }
    
    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();
    
    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);
    
    // try linking the program with given attachments
    glLinkProgram(programObject);
    
    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }
    
    return programObject;
}

