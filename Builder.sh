#! /bin/sh

if [[ "$OSTYPE" == "darwin"* ]]; then
    generator="Xcode"
else
    generator="Visual Studio 12 2013 Win64"
fi

args="$1"
if [ "$args" == "" ]; then
    echo "Input the build type (r or d)"
    read build_type	
	if [ "$build_type" != "r" ] && [ "$build_type" != "d" ]; then
        echo "Incorrect build type"
	    exit
    fi
elif [[ "$args" == "-r"* ]]; then
    build_type="r"
elif [[ "$args" == "-d"* ]]; then
    build_type="d"
fi

if [ "$build_type" == "r" ]; then
    build_type="Release"
else
    build_type="Debug"
fi

build_path="build"

if [ ! -d "${build_path}" ]; then
    mkdir "${build_path}"
fi

if [ -d "${build_path}" ]; then
    cd "${build_path}"
	cmake -G "$generator" -DCMAKE_BUILD_TYPE="$build_type" ../ex_01
else
    echo "Build directory cannot be created"
fi
