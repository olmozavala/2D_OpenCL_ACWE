Created by Olmo Zavala-Romero
Florida State Unversity 18 Apr. 2012

This is an OpenCL implementation of the ActiveContours without edges
algorithm. 

I am still developing this code so you may encounter some small bugs. 


------- Required Software-----
This code has been tested with different flavors of Ubuntu and Nvidia cards. 
In order to compile you need to install:

CUDA ('Read CUDA_by_oz.txt for my own experience installing CUDA')
OpenGL    --> freeglut-dev
GLEW      --> libglew1.6-dev
FreeImage --> libfreeimage-dev
premake4  --> premake4

------- Compile---------
To compile the code you just need to run:
premake4 gmake
make
make config=release ---> If you don't want debugging text

------- Run --------------
You can send as parameter the numbers 1-12 and it will select one of the images.
Examples from 6-9 are the same image but with different resolution.

Run with:
./ActiveCountours 1
./ActiveCountours 2.... etc. 

'I' -> To start and stop iterations of the algorithm 
'B' -> To alternate between using all or one of the image channesl


-------- PROBLEM WITH MULTIPLE DEFINITIONS-------
Comment HEADERS+=*.h
-------- PROBLEM WITH MULTIPLE DEFINITIONS-------


With shortcuts
make clean
qm
mk
./RunActiveContoursQt

To compile:
make clean
qmake Proj.pro
make -j 8
./RunActiveContoursQt


----------- Keyboard -----------
'I' Starts and stops the segmentation.
'B' Tooggle using all bands to do the segmentation or only one ( red ).
'T' Shows the timings.
'S' Selects a new image. 
