#include <QtGui/QMouseEvent>
#include <QFileInfo>
#include <QFileDialog>
#include <glew.h>
#include <fstream>

#include <GL/gl.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLWidget.h"
#include "GLManager/GLManager.h"
#include "FileManager/FileManager.h"
#include "debug.h"

#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) * (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))

// Couple of colors
#define RED     1.0f, 0.0f, 0.0f, 1.0f
#define GREEN   0.0f, 1.0f, 0.0f, 1.0f
#define BLUE    0.0f, 0.0f, 1.0f, 1.0f
#define YELLOW  1.0f, 1.0f, 0.0f, 1.0f
#define WHITE   1.0f, 1.0f, 1.0f, 1.0f

//If 1 then we perform the test for the next imageSize
#define TESTS 0

#define NUM_SAMPLERS = 1;

//Vertex colors are then defined only by vertex from 0 to 7 
float vertexColors[] = {
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f
};

float textCoords[] = {
    0.0f, 1.0f, //0
    1.0f, 1.0f, //1
    1.0f, 0.0f, //3
    0.0f, 0.0f, //2
};

const float size = 1;
const float zval = 1; //Bigger the number farther away
const float zvalROI = .09; //Bigger the number farther away

//Positions of a square that the user is drawing
float vertexPosSelection[] = {
    -0, 0, zvalROI, 1.0f, //Upper left
    0, 0, zvalROI, 1.0f, //Upper right
    0, -0, zvalROI, 1.0f, // Lower right
    -0, -0, zvalROI, 1.0f //Lower left
};


//Positions of a square
float vertexPositions[] = {
    -size, size, zval, 1.0f, //Upper left
    size, size, zval, 1.0f, //Upper right
    size, -size, zval, 1.0f, // Lower right
    -size, -size, zval, 1.0f //Lower left
};

//Indexes of the elements of an array
unsigned int vertexIndexes[] = {0, 1, 2, 3};

using namespace std;

/**
 * Constructor of the Widget. It sets the default
 * values of all its internal properties.
 */
GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    z = 1; // Default depth to show the pictures
    hsize = .9; // Default size for the 'square' to show the picture

    //Offset for the objects, in this case is only one object without offset
    // it is the 'rectangle' holding the image
    offsets[0] = glm::vec3(0.0f, 0.0f, 0.0f);

    tbo_in = 0; //Texture buffer object
    tbo_out = 0; //Texture buffer object
    sampler = 0;
    textUnit = 0;

    maxActCountIter = 3000;// Maximum number of ACWE iterations
    currIter = 0; // Current ACWE iteration
    iterStep = 30; //Number of ACWE iterations before retrieving result back to CPU
    acIterate = false;
    //Cool: 12, 13, 2, 6
    acExample = 1; //Example 7 is 128x128
    useAllBands = true; // Use all bands as an average for the ACWE algorithm

    mask = new int[4];

    imageSelected = false;//Indicates if the image has already been selected
    newMask = false;
    displaySegmentation = false;

    firstTimeImageSelected = true;

}// QGLWidget constructor

/**
 * Opens the 'select image' dialog. Stops all previous
 * segmentation and reloads everything.
 * TODO use exceptions or something similar to avoid returning ints
 */
void GLWidget::SelectImage() {
    //    QString fileName = QFileDialog::getOpenFileName(this, tr("Select an image"), "/home", tr("Files (*.png *.jpg *.bmp)"));

    //QString fileName = "/media/USBSimpleDrive/Olmo/OpenCL_Examples/OZ_OpenCL/ActiveCountoursImg/images/RectTest1.png";
    //QString fileName = "/media/USBSimpleDrive/Olmo/OpenCL_Examples/OZ_OpenCL/ActiveCountoursImg/images/Ft.png";
    //QString fileName = "/media/USBSimpleDrive/Olmo/OpenCL_Examples/OZ_OpenCL/ActiveCountoursImg/images/Min.png";
    //
    // ------------------- For performance tests
    //displaySegmentation = true;
    // ------------------- For performance tests

    //Select by the user
    QString fileName;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::List);
    QStringList fileNames;
    acIterate = false;
    if (dialog.exec()){
        fileNames = dialog.selectedFiles();
        fileName = fileNames[0];
    }

    if (!fileName.isNull()) {
        inputImage = new char[fileName.length() + 1];
        outputImage = new char[fileName.length() + 9];

        strcpy(inputImage, fileName.toLatin1().constData());
        dout << "Input image: " << inputImage << endl;

        fileName = fileName.replace(QString("."), QString("_result."));
        strcpy(outputImage, fileName.toLatin1().constData());
        cout << "Output image: " << outputImage << endl;

        //Clear selection of mask
        updatingROI = false;
        startXmask = -1;
        startYmask = -1;
        endXmask = -1;
        endYmask = -1;

        //When we select a new image we stop showing
        // the 'segmentation' until a new ROI is selected
        // TODO clear ROI
        displaySegmentation = false;
        init();
        firstTimeImageSelected = false;
    } else {
        //TODO display a dialog informing the following text.
        cout << "The image haven't been selected. " << endl;
    }
}

void GLWidget::CreateSamplers() {
    int num_samplers = 1;
    glGenSamplers(num_samplers, &samplerID[0]);

    for (int samplerIx = 0; samplerIx < num_samplers; samplerIx++) {
        //Defines the Wraping parameter for all the samplers as GL_REPEAT
        //glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_R, GL_REPEAT);

        glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glSamplerParameteri(samplerID[samplerIx], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    //    Using GL_LINEAR interpolation for the sampler
    //glSamplerParameteri(samplerID[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glSamplerParameteri(samplerID[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(samplerID[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(samplerID[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void GLWidget::InitActiveCountours() {
    cout << "InitActiveCountours" << endl;
    int method = 2; //1 for OZ 2 for Voronoi
    float alpha = 0.5;
    float def_dt = .5;

    if(firstTimeImageSelected){
        switch (method) {
            case 1:
                clObj.loadProgram(SDFOZ, maxActCountIter, alpha, def_dt );
                break;
            case 2://Current
                clObj.loadProgram(SDFVORO, maxActCountIter, alpha, def_dt);
                break;
        }
    }

    clObj.loadImage( (char*) inputImage, (char*) outputImage,
            width, height);
}

void GLWidget::InitializeSimpleVertexBuffer() {

    GLManager::CreateBuffer(vbo_selection, vertexPosSelection, sizeof (vertexPosSelection),
            GL_ARRAY_BUFFER, GL_STREAM_DRAW, 0, 4, GL_FALSE, 0, 0, GL_FLOAT);

    GLManager::CreateElementBuffer(ebo, vertexIndexes, sizeof (vertexIndexes), GL_STATIC_DRAW);
}

/**
 * This method should clean any buffer (. 
 */
void GLWidget::DeleteBuffers(){
    glDeleteBuffers(1,&vbo_pos);
    glDeleteBuffers(1,&vbo_color);
    glDeleteBuffers(1,&vbo_tcord);
    glDeleteBuffers(1,&ebo);
}

/**
 * This function initializes the vertex positions. In this
 * case we simply have a big square that has the size of the window
 */
void GLWidget::InitializeVertexBuffer() {

    dout << "At InitializeVertexBuffer size is: (" << width << "," << height << ")" << endl;
    int maxDim = max(width, height);
    /*
    //Top left
    vertexPositions[0] = (float) -width / maxDim;
    vertexPositions[1] = (float) height / maxDim;

    //Top Right
    vertexPositions[4] = (float) width / maxDim;
    vertexPositions[5] = (float) height / maxDim;

    //Bottom Right
    vertexPositions[8] = (float) width / maxDim;
    vertexPositions[9] = (float) -height / maxDim;

    //Bottom Left 
    vertexPositions[12] = (float) -width / maxDim;
    vertexPositions[13] = (float) -height / maxDim;
    */

    GLManager::CreateBuffer(vbo_pos, vertexPositions, sizeof (vertexPositions),
            GL_ARRAY_BUFFER, GL_STATIC_DRAW, 0, 4, GL_FALSE, 0, 0, GL_FLOAT);

    GLManager::CreateBuffer(vbo_color, vertexColors, sizeof (vertexColors),
            GL_ARRAY_BUFFER, GL_STATIC_DRAW, 1, 4, GL_FALSE, 0, 0, GL_FLOAT);

    GLManager::CreateBuffer(vbo_tcord, textCoords, sizeof (textCoords),
            GL_ARRAY_BUFFER, GL_STATIC_DRAW, 2, 2, GL_FALSE, 0, 0, GL_FLOAT);

    GLManager::CreateElementBuffer(ebo, vertexIndexes, sizeof (vertexIndexes), GL_STATIC_DRAW);

    //Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLWidget::InitTextures() {

    BYTE* image = ImageManager::loadImageByte(inputImage, width, height);

    //Warning!!! imageFloat is now only used for displaying the internal values not for OpenGL
    //float* imageFloat = ImageManager::byteToFloat(imageByte, width * height * 4);

    //ImageManager::printImageBGRA(width, height, imageFloat);

    dout << "Size of byte: " << sizeof (BYTE) << endl;
    dout << "Size of char: " << sizeof (char) << endl;

    //GLManager::Create2DTexture(tbo_in, image, width, height, GL_UNSIGNED_INT_8_8_8_8_REV, GL_RGBA, GL_LINEAR, GL_LINEAR);

    //if( firstTimeImageSelected ){
        //cout << " 000000000000000000001111111111233333333312222" << endl;
        glGenTextures(1, &tbo_in);
        glGenTextures(1, &tbo_out);
    //}
    
    cout << "Updating the data of the textures .... " << endl;
    glBindTexture(GL_TEXTURE_2D, tbo_in);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE0+tbo_in);

    //GLManager::Create2DTexture(tbo_out, NULL, width, height, GL_FLOAT, GL_RGBA16, GL_LINEAR, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, tbo_out);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_BGRA, GL_FLOAT, NULL);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE0+tbo_out);
}

/**
 * Initializes the shaders for OpenGL. It also
 * initializes the OpenGL program, the camera and the 
 * uniforms */
void GLWidget::InitializeProgram() {
    std::vector<GLuint> shaderList;

    //Reads the vertex and fragment shaders
    string strVertexShader = FileManager::readFile("src/resources/shaders/VertShader.glsl");
    string strFragmentShader = FileManager::readFile("src/resources/shaders/FragShader.glsl");

    //dout << "Vertex shader:" << strVertexShader <<endl;
    //dout << "Fragment shader:" << strFragmentShader <<endl;
    shaderList.push_back(GLManager::CreateShader(GL_VERTEX_SHADER, strVertexShader));
    shaderList.push_back(GLManager::CreateShader(GL_FRAGMENT_SHADER, strFragmentShader));

    //Compiles and links the shaders into a program
    g_program.theProgram = GLManager::CreateProgram(shaderList);

    dout << "Program compiled and linked" << endl;
    //Gets the uniform id for the camera to clip martrix (perspective projection)
    g_program.cameraToClipMatrixUnif = glGetUniformLocation(g_program.theProgram, "perspectiveMatrix");

    //Gets the uniform for the model to camera matrix (movement of each object)
    modelToCameraMatrixUnif = glGetUniformLocation(g_program.theProgram, "modelMatrix");
    dout << "MatrixUnif: " << modelToCameraMatrixUnif << endl;

    GLuint textSamplerLoc = glGetUniformLocation(g_program.theProgram, "textSampler");
    dout << "textSamplerLoc: " << textSamplerLoc << endl;

    glUseProgram(g_program.theProgram); //Start using the builded program

    glUniform1i(textSamplerLoc, textUnit); //Binds the texture uniform with the texture like id
    glUniformMatrix4fv(g_program.cameraToClipMatrixUnif, 1, GL_FALSE, 
            glm::value_ptr(camera->getProjectionMatrix() * camera->getViewMatrix()));
    glUseProgram(0);

    std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

    dout << "-------- Compiling Simple Fragment shader program ----------" << endl;

    //Reads the vertex and fragment shaders
    string strSimpleFragmentShader = FileManager::readFile("src/resources/shaders/SimpleFragShader.glsl");

    shaderList.push_back(GLManager::CreateShader(GL_VERTEX_SHADER, strVertexShader));
    shaderList.push_back(GLManager::CreateShader(GL_FRAGMENT_SHADER, strSimpleFragmentShader));

    g_program.simpleFragProgram = GLManager::CreateProgram(shaderList);

    dout << "Simpler Program compiled and linked" << endl;
    dout << "--------------End of loading OpenGL Shaders -----------------" << endl;
}

/**
 * Initializes the vertex and textures once the image
 * has been loaded properly. 
 */
void GLWidget::init() {
    dout << "------- init()--------" << endl;

    //IMPORTANT!!!! The textures need to be initialized before the vertex buffers,
    //because it is in this function where the size of the images get read
    dout << "Initializing Textures... " << endl;
    InitTextures(); //Init textures
    dout << "Textures initialized!! " << endl;

    if(firstTimeImageSelected){
        //Create the Vertex Array Object (contains info of vertex, color, coords, and textures)
        glGenVertexArrays(1, &vaoID); //Generate 1 vertex array
        glGenVertexArrays(1, &vaoSimpleID); //Generate 1 vertex array
        glBindVertexArray(vaoID); //First VAO setup (only one this time)

        // Samplers that define how to treat the image on the corners,
        // and when we zoom in or out to the image
        CreateSamplers();

        dout << "Initializing Vertex buffers... " << endl;
        InitializeVertexBuffer(); //Init Vertex buffers
    }

    // This should be already after mask 
    dout << "Initializing OpenCL... " << endl;
    InitActiveCountours();

    dout << "Initializing images, arrays and buffers (CL)!! " << endl;
    cout << "SSSSSSSSSSSSSSS " << firstTimeImageSelected << endl;
    clObj.initImagesArraysAndBuffers(tbo_in, tbo_out, firstTimeImageSelected);

    dout << "Init SUCCESSFUL................" << endl;

    glBindVertexArray(vaoSimpleID); //First VAO setup (only one this time)
    InitializeSimpleVertexBuffer();
    dout << "Initializing simple VAO (for ROI)" << endl;

    glBindVertexArray(0); //Unbind any vertex array

    imageSelected = true;
}

/* This is the first call after the constructor.
 * This method initializes the state for OpenGL.
 */
void GLWidget::initializeGL() {

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
    }

    glEnable(GL_CULL_FACE); //Cull ('desechar') one or more faces of polygons
    glCullFace(GL_BACK); // Hide the 'back' face
    glFrontFace(GL_CW); //Which face is 'front' face, defindes as Clock Wise

    Timer tm_oclogl_init(ts, "OCLinit");
    tm_oclogl_init.start();

    //Initializes the camera perspective paramteres
    float fzNear = 1.0f;
    float fzFar = 1000.0f;
    float FOV = 45.0f;

    camera = new FPSMovement(fzNear, fzFar, FOV);

    dout << "Initializing OpenGL program... " << endl;
    InitializeProgram();
    dout << "OpenGL program initialized ... " << endl;

    tm_oclogl_init.end();
}

void GLWidget::resizeGL(int w, int h) {
    dout << "Resizing GL ......." << endl;

    // NEVER TOUCH THIS TWO VALUES ARE NECESSARY
    winWidth = w;//Updating the width of the window for the ROI
    winHeight = h;//Updating the height of the window for the ROI

    camera->Reshape(w,h);
    glUniformMatrix4fv(g_program.cameraToClipMatrixUnif, 1, GL_FALSE, 
            glm::value_ptr(camera->getProjectionMatrix() * camera->getViewMatrix()));

}

/**
 * This is the main OpenGL loop. Here the display of results is made
 */
void GLWidget::paintGL() {
    glFlush();

    //Check if we already have an image selected, if not nothing should be done
    if (imageSelected) {
        // ----------------- For the case of perfoming tests ------------------
        if(TESTS){

            if(acIterate){
                cout << "--------------- Initializing mask and making SDF..........." << endl;
                for(int i = 0; i < 1; i++){
                    mask[0] = (int) width/4;
                    mask[1] = (int) 3*width/4;
                    mask[2] = (int) height/4;
                    mask[3] = (int) 3*height/4;
                    Timer tm_ocl_sdf(ts, "SDF");

                    tm_ocl_sdf.start();
                    clObj.createRGBAMask(width, height, mask[0], mask[1], mask[2], mask[3]);
                    clObj.runSDF();
                    tm_ocl_sdf.end();

                    newMask = false;

                    cout << "iterating ....." << currIter << endl;
                    Timer tm_ocl_ac(ts, "ACont");
                    tm_ocl_ac.start();

                    clObj.iterate(100, useAllBands); //Iterate the ActiveCountours n times
                    tm_ocl_ac.end();
                    cout << "Current run: " << i << endl;
                }

                ts.dumpTimings();
            }
            acIterate = false;
            // ----------------- For the case of perfoming tests ------------------
        }else{
            //----------- Normal execution ------------
            if (newMask) {
                cout << "--------------- Initializing mask and making SDF..........." << endl;

                Timer tm_ocl_sdf(ts, "SDF");

                tm_ocl_sdf.start();
                clObj.createRGBAMask(width, height, mask[0], mask[1], mask[2], mask[3]);
                clObj.runSDF();
                tm_ocl_sdf.end();

                newMask = false;
            }

            if ((currIter < maxActCountIter) && acIterate) {

                dout << "iterating ....." << currIter << endl;
                Timer tm_ocl_ac(ts, "ACont");
                tm_ocl_ac.start();

                clObj.iterate(iterStep, useAllBands); //Iterate the ActiveCountours n times
                currIter += iterStep;
                tm_ocl_ac.end();
                dout << "Current iter: " << currIter << endl;
            }
        }
        //----------- Normal execution ------------


        //Clears the screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g_program.theProgram);

        glBindVertexArray(vaoID); //First VAO setup (only one this time)

        //Sets the camera
        modelMatrix = camera->getModelMatrix();

        glUniformMatrix4fv(modelToCameraMatrixUnif, 1, 
                GL_FALSE, glm::value_ptr(modelMatrix));

        glUniformMatrix4fv(g_program.cameraToClipMatrixUnif, 1, GL_FALSE, 
                glm::value_ptr(camera->getProjectionMatrix() * camera->getViewMatrix()));

        // Binds the texture of the image
        glActiveTexture(GL_TEXTURE0 + textUnit);
        glBindTexture(GL_TEXTURE_2D, tbo_in);
        glBindSampler(textUnit, samplerID[0]);

        // Draws the frame of the image
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);

        //-------- TEXTURES ----------
        if (displaySegmentation) {
            glBindTexture(GL_TEXTURE_2D, tbo_out);
            glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
        }

        glBindSampler(textUnit, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(0); //Unbind VAO
        glUseProgram(0); //Unbind program


        if(!displaySegmentation){
            glUseProgram(g_program.simpleFragProgram);
            glBindVertexArray(vaoSimpleID);

            GLManager::CreateBuffer(vbo_selection, vertexPosSelection, sizeof (vertexPosSelection),
                        GL_ARRAY_BUFFER, GL_STREAM_DRAW, 0, 4, GL_FALSE, 0, 0, GL_FLOAT);

            /*
            glGenBuffers(1,&vbo_selection);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_selection);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosSelection), vertexPosSelection, GL_STREAM_DRAW);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);//Define where to read the data
            glEnableVertexAttribArray(0);//Enable the vertex atrib array
            */

            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
        }


        glBindVertexArray(0); //Unbind VAO
        glUseProgram(0); //Unbind program

    }
    update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    camera->mouseReleaseEvent(event);

    endXmask = event->x();
    endYmask = event->y();

    /*
       dout << "Updating mask........ " << endl;
       dout << "Start at: (" << startXmask << "," << startYmask << ")" << endl;
       dout << "Ends at: (" << endXmask << "," << endYmask << ")" << endl;

       dout << "Image size : (" << width << "," << height << ")" << endl;
       dout << "Window size : (" << winWidth << "," << winHeight << ")" << endl;
       */

    mask[0] = (int) ((startXmask * width) / winWidth);
    mask[1] = (int) ((endXmask * width) / winWidth);

    mask[2] = height - (int) ((endYmask * height) / winHeight);
    mask[3] = height - (int) ((startYmask * height) / winHeight);

    dout << "Corresp mask start: (" << mask[0] << "," << mask[2] << ")" << endl;
    dout << "Corresp mask end: (" << mask[1] << "," << mask[3] << ")" << endl;

    newMask = true; //Run SDF (start displaying segmentation) 

    updatingROI = false; //Stop drawing user ROI, start displaying segmentation
}

void GLWidget::mousePressEvent(QMouseEvent *event) {

    camera->mousePressEvent(event);

    dout << "************ INIT POS*************" << endl;
    int currX = event->x();
    int currY = event->y();

    float newX = currX / (float) winWidth;
    float newY = (winHeight - currY) / (float) winHeight;

    startXmask = event->x();
    startYmask = event->y();

    newX = newX*2 - 1;
    newY = newY*2 - 1;

    //------ Initialize ROI all into one point ----
    //Upper left x,y
    vertexPosSelection[0] = newX;
    vertexPosSelection[1] = newY;

    //Upper right x,y
    vertexPosSelection[4] = newX;
    vertexPosSelection[5] = newY;

    //Lower right x,y
    vertexPosSelection[8] = newX;
    vertexPosSelection[9] = newY;

    //Lower left x,y
    vertexPosSelection[12] = newX;
    vertexPosSelection[13] = newY;

    updatingROI = true;

}

/**
 * This function catches the mouse move event. It is used when
 * the user is selecting a ROI. It updates the position of the
 * square to display.
 * @param event
 */
void GLWidget::mouseMoveEvent(QMouseEvent *event) {

    camera->mouseMoveEvent(event);
    if (updatingROI) {
        int currX = event->x();
        int currY = event->y();

        float newX = currX / (float) winWidth;
        float newY = (winHeight - currY) / (float) winHeight;

        //dout << currX << "/" << winWidth << "....." << currY << "/" << winHeight << endl;

        newX = newX*2 -1;
        newY = newY*2 -1;

        //Upper right x,y
        vertexPosSelection[4] = newX;

        //Lower right x,y
        vertexPosSelection[8] = newX;
        vertexPosSelection[9] = newY;

        //Lower left x,y
        vertexPosSelection[13] = newY;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
    camera->keyReleaseEvent(event);
}
/**
 * Management of all the keyboards pressed.
 */
void GLWidget::keyPressEvent(QKeyEvent* event) {

    camera->keyPressEvent(event);
    dout << "Key = " << (unsigned char) event->key() << endl;

    //printMatrix(camera->getCameraMatrix());
    switch (event->key()) {

        case 105:// Case 'I' start and stops Active Contours
        case 73:
            //Start iterating
            acIterate = !acIterate;

            // After running the SDF for the first time we 
            // start displaying the segmentation.
            displaySegmentation = true;
            ts.dumpTimings();

            break;
        case 66:// Case 'B' toggle using all bands or only red band
        case 98:
            useAllBands = !useAllBands;
            break;
        case 116:// Case 'T' shows the timings
        case 84:
            ts.dumpTimings();
            break;
        case 'S':
            SelectImage();
            break;
        case Qt::Key_Escape:
            close();
            break;
        default:
            event->ignore();
            break;

    }

    QWidget::keyPressEvent(event);

    updateGL();
    //UpdatePerspective();
    //glutPostRedisplay();
}

void GLWidget::printGLMmatrix(glm::mat4 matrix)
{
    printf("%2.2f \t %2.2f \t %2.2f \t %2.2f \n", matrix[0].x, matrix[0].y, matrix[0].z, matrix[0].w);
    printf("%2.2f \t %2.2f \t %2.2f \t %2.2f \n", matrix[1].x, matrix[1].y, matrix[1].z, matrix[1].w);
    printf("%2.2f \t %2.2f \t %2.2f \t %2.2f \n", matrix[2].x, matrix[2].y, matrix[2].z, matrix[2].w);
    printf("%2.2f \t %2.2f \t %2.2f \t %2.2f \n", matrix[3].x, matrix[3].y, matrix[3].z, matrix[3].w);
}

