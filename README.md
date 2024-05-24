# Overview
This is a simple 3D graphics program that links with the OpenHaptics library to control force-feedback devices, like the SenseAble PHANToM.
The libraries and drivers for these devices are now rather old, so this app acts as a stand-alone server that can be compiled once and used
with a number of other modern apps created in Unity or whatever other package you wish.  The server listens on the network for commands that
tell it what forces to apply.  The graphics are simple and intended only for debugging purposes.

# Dependencies
1. OpenHaptics library and compatable force-feedback device.  (Tested only with the Phantom Premium 1.0 device.)
2. MinVR3.cpp library for network communication.

# Build
Follow the usual cmake convention:
1. mkdir build
2. cd build
3. cmake-gui .. &
4. Configure.  Generate.  Build.

# Examples
1. example_client:  A simple C++ client used for testing the server.
2. example_unity_client:  An example MinVR3 Unity app that calls the server.
