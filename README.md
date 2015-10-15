OenCL Active Contours Without edges
====

This is an OpenCL implementation that computes the 2D Active Contours Without Edges. 
The algorithm is from the paper "Active Contours Without Edges"
from Chan and Vase

# Clone
This project uses a couple of wrappers and utilities that are available
at the OZlib repository. It also uses the Signed Distance Function
project in order to initialize the algorithm. 

In order for these submodule to be added to your clone folder 
you need to do: 

First your normal cloning:

    git clone git@github.com:olmozavala/2D_OpenCL_ACWE.git

Then you cd into the folder and add the submodules with:
    
    git submodule update --init --recursive

# Update submodules


# Build
This code has been tested with different flavors of Ubuntu and Nvidia cards. 
It uses the FreeImage library for image manipulation and premake4
to build the project. In ubuntu this two libraries can be installed with:

    sudo apt-get install premake4 libfreeimage3 libfreeimage-dev
    
Verify that the path of OPENCL in the 'premake4.lua' file
corresponds to the location of your opencl installation. In my case
it is set to '/usr/local/cuda'.

First  compile the submodules with:

    sh submodules_compile.sh

To compile the code you just need to run:

    premake4 gmake
    make
    make config=release ---> If you don't want debugging text

Or using the bash scripts:

    sh compile.sh

# Run
You can send as parameter one number, from 1 to 6, and it will select one of 
sample images in the images folder. Run the program with:

     ./dist/SignedDistFunc #

Or with the script file

    sh run.sh

The results are stored at the 'images' folder

2D_OpenCL_SDF
2D_OpenCL_SDF

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
