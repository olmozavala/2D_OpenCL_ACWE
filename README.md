OenCL Active Contours Without edges
====

This is an OpenCL implementation that computes the 2D Active Contours Without Edges. 
The algorithm is from the paper "Active Contours Without Edges"
from Chan and Vase.

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
to build the project. OpenGL and GLEW to display the results.
In ubuntu these libraries can be installed with:

    sudo apt-get install premake4 libfreeimage3 libfreeimage-dev freeglut-dev libglew-dev
    
Verify that the path of OPENCL in the 'premake4.lua' file
corresponds to the location of your opencl installation. In my case
it is set to '/usr/local/cuda'.

First  compile the submodules with:

    sh submodules_compile.sh

Then compile the code with:

    make clean
    qmake Project.pro
    make
    make config=release ---> If you don't want debugging text

Or using the bash scripts:

    sh compile.sh

# Run
Run the program with:

     ./dist/RunActiveContoursQt 

Or with the script file

    sh run.sh


# Options
This program have some options that didn't make to the top menus and 
are very important:

    'I' -> To start and stop iterations of the algorithm (once the ROI has been selected) 
    'B' -> To alternate between using all or one of the image channesl
    'S' -> Selects a new image. 
