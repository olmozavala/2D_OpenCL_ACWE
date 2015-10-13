
/* 
 * File:   CameraMovement.cpp
 * Author: Olmo Zavala Romero 
 * 
 */

#include "CameraMovement.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include "debug.h"

#define PI 3.14159

using namespace std;

float CameraMovement::CalcFrustrumScale(float FOVdeg)
{
    float degToRad = PI * 2.0f/360.0f;
    float fovRad = FOVdeg * degToRad;
    return 1.0f/ tan(fovRad/2.0f); 
}

void CameraMovement::Reshape(int w, int h)
{
/*
    projMatrix[0].x = fFrustumScale / (w/ (float)h);
    projMatrix[1].y = fFrustumScale;
*/
    projMatrix = glm::perspective(45.0f, (float)w/(float)h , 0.1f, 100.0f);
}

CameraMovement::CameraMovement(float fzNear, float fzFar, float FOV)
{
    dout << "Inside CameraMovement" << endl;

    //Defines the initial perspective matrix
    projMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);

    /*
    fFrustumScale = this->CalcFrustrumScale(FOV);

    projMatrix[0].x = fFrustumScale;
    projMatrix[1].y = fFrustumScale;
    projMatrix[2].z = (fzFar + fzNear) / (fzNear - fzFar);
    projMatrix[2].w = -1.0f;
    projMatrix[3].z = (2 * fzFar * fzNear) / (fzNear - fzFar);
    */

    projMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    viewMatrix = glm::lookAt(
        glm::vec3(0,0,-3), // Camera is at (4,3,3), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
}

glm::mat4 CameraMovement::getCameraMatrix(){
    return projMatrix;
}
glm::mat4 CameraMovement::getViewMatrix(){
    return viewMatrix;
}
glm::mat4 CameraMovement::getModelMatrix(){
    return modelMatrix;
}
glm::mat4 CameraMovement::getProjectionMatrix(){
    return projMatrix;
}
