
/* 
 * File:   FPSMovement.h
 * Author: Olmo Zavala Romero
 *
 */

#ifndef FPSMOV_H
#define	FPSMOV_H

#include "CameraMovement.h"
#include <glm/glm.hpp>

class FPSMovement : public CameraMovement{

public:
    FPSMovement(float fzNear, float fzFar, float FOV);
    FPSMovement(const FPSMovement& orig);
    virtual ~FPSMovement();

    void Keyboard(unsigned char key, int x, int y); 
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    //static int PRIMARY = GLUT_LEFT_BUTTON;//Which mouse button will be used for movement
    static const int PRIMARY = 4;//Which mouse button will be used for movement
    //static const int SECONDARY = GLUT_RIGHT_BUTTON;//

    int initX; //Initial position to rotate or translate in X
    int initY; //Initial position to rotate or translate in Y
    bool rotating; //Indicates if the primary button is pressed and the model will be rotated
    bool translating; //Indicates if the world is being translated by Ctrl+Mouse rather than rotated

    //TODO this value should be related with the width and height of the window
    float movementSpeed = .001;//It simply indicates how fast to move
};

#endif	/* FPSMovement_H */
