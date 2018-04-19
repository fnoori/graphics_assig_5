//
//  objectReader.h
//  graphics_assig_5_1
//
//  Created by Farzam Noori on 2018-04-03.
//  Copyright © 2018 Farzam Noori. All rights reserved.
//

#ifndef objectReader_h
#define objectReader_h

#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"

using namespace glm;
using namespace std;

class ObjectReader
{
private:
    const int BUFF_SIZE = 500;
    vector<vec3> tmpVerticies;
    vector<vec2> tmpUvs;
    vector<vec3> tmpNormals;
    
    vector<vec3> outVertices;
    vector<vec2> outUvs;
    vector<vec3> outNormals;
    
    vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    
public:
    ObjectReader();
    void printLines(const char* filename);
    void findSphere(const char* filename);
    
    void processData();
    vector<vec3> getVertices();
    vector<vec2> getUvs();
    vector<vec3> getNormals();
};

#endif /* objectReader_h */
