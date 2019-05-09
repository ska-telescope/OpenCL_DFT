# OpenCL_DFT

1. Instructions for installation


1.1 Install AMD Driver:

https://www.amd.com/en/support/kb/faq/gpu-635

1.1.1 Download

Download driver from https://www.amd.com/en/support

1.1.2 Extract

After the archive is downloaded, extract the contents to a location from which you can install it. 

1.1.3 Install

Once the archive is expanded on the local machine, run the included script (amdgpu-pro-install) to install the graphics stack. 

$sudo ./amdgpu-pro-install

$sudo reboot 


1.2 Install OpenCL:

$sudo apt update

$sudo apt install mesa-opencl-icd

$sudo apt install clinfo

$clinfo

$sudo apt install ocl-icd-opencl-dev


1.3 Install Cmake (build tools)

$ sudo apt install cmake


1.4 Install Google Test (unit testing) 

$ sudo apt install libgtest-dev

$ cd /usr/src/gtest

$ sudo cmake CMakeLists.txt

$ sudo make

$ sudo cp *.a /usr/lib


2. Instructions for usage


2.1 Build direct fourier transform project (from project folder)

$ mkdir build && cd build

$ cmake .. -DCMAKE_BUILD_TYPE=Release && make

Or 

$ mkdir build && cd build

$ cmake .. -DCMAKE_BUILD_TYPE=Debug && make

2.2 To execute the direct fourier transform (once configured and built), execute the following command (also assumes appropriate build folder):

$ ./dft


2.3 To execute unit testing, execute the following (also assumes appropriate build folder):

$ ./tests

