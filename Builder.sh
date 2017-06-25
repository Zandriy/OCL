#! /bin/sh

if [[ "$OSTYPE" == "darwin"* ]]; then
    generator="Xcode"
elif [[ "$OSTYPE" == "linux"* ]]; then
    generator="CodeBlocks - Unix Makefiles"
else
    echo "Input Visual Studio version (2012, 2013, 2015 or 2017)"
    read vs_ver
    case $vs_ver in
       2012)
          generator="Visual Studio 11 2012 Win64"
          ;;
       2013)
          generator="Visual Studio 12 2013 Win64"
          ;;
       2015)
          generator="Visual Studio 14 2015 Win64"
          ;;
       2017)
          generator="Visual Studio 15 2017 Win64"
          ;;
       *)
          echo "Input correct version"
          exit 1
    esac
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
