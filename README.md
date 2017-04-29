# OpenGL: Hiding / Textures / Lighting

## Check OpenGL version

```sh
glxinfo | grep "OpenGL version"
```

## Installing OpenGL required and dev libraries on Ubuntu

```sh
sudo apt-get install build-essential mesa-common-dev libglew-dev libglu1-mesa-dev libx11-dev libxi-dev libxrandr-dev freeglut3 freeglut3-dev
```

## Building the SDK libraries

1. Ensure Premake utility (version 4.3 or later) is installed on your system
```sh
sudo apt-get install premake4
```
2. Go to glsdk directory and execute Premake to generate the Makefile
```sh
premake4 gmake
```
3. Build the libraries with make
```sh
make
```

More info: [Instructions for building OpenGL SDK components](http://glsdk.sourceforge.net/docs/html/pg_build.html)
