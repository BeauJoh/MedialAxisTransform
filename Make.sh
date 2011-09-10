#!/bin/sh
if [[ ($# = 0) ]]; then
    #build it here!
    echo "Building Code!"
    cd build/
    cmake ..
    make
    cd MedialAxisTransform/
    cp MedialAxisTransform ../../binary
    echo "Compiled code is located in current directory and known as binary"
    echo "arguments are:"
    echo "\t\t-k to specify kernel followed by a path to kernel"
    echo "\t\t-i to specify input image stack followed by a path to images"
    echo "\t\t-o to specify output image stack followed by a path to where to write images to"
    echo "Example usage:"
    echo "\t ./binary -k MedialAxisTransform/my_kernel.cl -i DataResources/High-Res-Stage-24-Take-4/out.png -o DataResources/output/result.png"
fi

if [[ ($# > 1) ]]; then
    #too many arguments for Builder!
    echo "\nMake.sh only supports:"
    echo "\t'Make.sh'\t\t\twhich will place a binary MedialAxisTransform program in your current directory."
    echo "\t'Make.sh clean'\t\t\twhich will remove meta data generated when building the file"
    echo "\n"
    echo "\tp.s. Please leave all directory structures in-tact\n\n"
    exit
fi

if echo "$1" | grep -q "clean"; then
    #deletion time!
    echo "\tcleaning build directory..."
    cd build/
    make clean
    rm -r *
    echo "\tbuild directory now clean"
    echo "\tremoving binary..."
    rm ../binary
    echo "\tbinary now removed"
    echo "\tdone... all clean :)"
    exit
fi