#!/bin/bash

# Configuration #
PROJECT_NAME="MapEdit"
MAJOR_VER=0
MINOR_VER=0
PATCH_VER=1
PROJECT_DIR="/home/felix/Development/mapedit"
REQUIRED_LIBS=(GL GLEW X11 freetype stdc++)
HEADER_DIR="-I../inc/common -I../inc/linux -I/usr/include/freetype2"
PLATFORM_SOURCES_DIR="src/linux/"
COMMON_SOURCES_DIR="src/common/"
GCC_OPTIONS="-Wextra -Wall -std=c++11 -ggdb"
GLOBAL_DEFINES=(HARDWARE_RENDERER LANG=EN_US)

######################################################

cd $PROJECT_DIR

# Set GCC libs #
REQUIRED_LIBS_STRING=""
for i in "${REQUIRED_LIBS[@]}"
do
    REQUIRED_LIBS_STRING+=" -l$i"
done

# Set GCC defines #
GLOBAL_DEFINES_STRING=""
for i in "${GLOBAL_DEFINES[@]}"
do
    GLOBAL_DEFINES_STRING+=" -D$i"
done

    GLOBAL_DEFINES_STRING+=" -DMAJOR_VER=$MAJOR_VER -DMINOR_VER=$MINOR_VER -DPATCH_VER=$PATCH_VER -DPROJECT_NAME=$PROJECT_NAME"
# Set source directories #
COMMON_SOURCES=""
for file in $COMMON_SOURCES_DIR/*.cpp
do
    name=${file##*/}
    base=${name%.cpp} 
    if ! [[ -z $name ]]; then
        COMMON_SOURCES+="../$COMMON_SOURCES_DIR$name "
    fi
done

PLATFORM_SOURCES=""
for file in $PLATFORM_SOURCES_DIR/*.cpp
do
    name=${file##*/}
    base=${name%.cpp} 
    if ! [[ -z $name ]]; then
        PLATFORM_SOURCES+="../$PLATFORM_SOURCES_DIR$name "
    fi
done

# Generate final string for GCC sources #
GCC_SOURCES=""
if ! [[ -z $COMMON_SOURCES ]]; then
    GCC_SOURCES+="$COMMON_SOURCES"
fi
if ! [[ -z $PLATFORM_SOURCES ]]; then
    GCC_SOURCES+="$PLATFORM_SOURCES"
fi

# Now build in ./build #
cd build || exit

echo $GLOBAL_DEFINES_STRING

g++ -o $PROJECT_NAME $GLOBAL_DEFINES_STRING $GCC_OPTIONS $HEADER_DIR $GCC_SOURCES $REQUIRED_LIBS_STRING

mv $PROJECT_NAME ../dist/linux-64 || exit

cd ../dist/linux-64
./$PROJECT_NAME

