#!/bin/bash

BUILD_TYPE=Release
BUILD_CONFIG=armhf

usage()
{
    echo "Usage: $0"
    echo "  -c (--cfg) - indicates the build target such as amd64, armhf (pi 1, zero), etc."
    echo "  -t (--type) - indicates the build type, Release or Debug"
    echo "  -h (--help) - print this usage statement"
}

while [ "$1" != "" ]; do
    case $1 in
        -t | --type )
            shift
            BUILD_TYPE=$1
            ;;
        -c | --cfg )
            shift
            BUILD_CONFIG=$1
            ;;
        -h | --help )
            usage
            exit
            ;;
        * )
        usage
        exit 1
    esac
    shift
done

BUILD_TYPE_MASSAGED=`echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]'`

BUILD_DIR="cmake-$BUILD_CONFIG-$BUILD_TYPE_MASSAGED"

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DAFM_CONFIG=$BUILD_CONFIG ..

cores=`grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}'`
make -j$cores

