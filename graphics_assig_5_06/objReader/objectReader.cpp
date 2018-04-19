//
//  objectReader.cpp
//  graphics_assig_5_1
//
//  Created by Farzam Noori on 2018-04-03.
//  Copyright © 2018 Farzam Noori. All rights reserved.
//

#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "objectReader.h"

using namespace std;
using namespace glm;

ObjectReader :: ObjectReader()
{}

void ObjectReader :: findSphere(const char *filename)
{
    ifstream f (filename);
    char buffer [BUFF_SIZE];
    
    vec3 vertex;
    vec2 uv;
    vec3 normal;
    int vertexIndex[3], uvIndex[3], normalIndex[3];
    
    while (f) {
        f.getline(buffer, BUFF_SIZE);
        if (sscanf(buffer, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
            
            tmpVerticies.push_back(vertex);
            
        } else if (sscanf(buffer, "vt %f %f", &uv.x, &uv.y) == 2) {
            
            tmpUvs.push_back(uv);
            
        } else if (sscanf(buffer, "vn %f %f %f", &normal.x, &normal.y, &normal.z) == 3) {
            
            tmpNormals.push_back(normal);
            
        } else if (sscanf(buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]) == 9) {
            
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
}

void ObjectReader :: processData()
{
    for (int i = 0; i < vertexIndices.size(); i++) {
        int vertexIndex = vertexIndices[i];
        vec3 vertex = tmpVerticies[vertexIndex - 1];
        
        outVertices.push_back(vertex);
    }
    
    for (int i = 0; i < uvIndices.size(); i++) {
        int uvIndex = uvIndices[i];
        vec2 uv = tmpUvs[uvIndex - 1];
        
        outUvs.push_back(uv);
    }
    
    for (int i = 0; i < normalIndices.size(); i++) {
        int normalIndex = normalIndices[i];
        vec3 normal = tmpNormals[normalIndex - 1];
        
        outNormals.push_back(normal);
    }
}

vector<vec3> ObjectReader :: getVertices()
{
    return outVertices;
}

vector<vec2> ObjectReader :: getUvs()
{
    return outUvs;
}

vector<vec3> ObjectReader :: getNormals()
{
    return outNormals;
}

void ObjectReader :: printLines(const char *filename)
{
    ifstream f (filename);
    char buffer [BUFF_SIZE];
    
    while (f) {
        f.getline(buffer, BUFF_SIZE);
    }
    
    f.close();
}

