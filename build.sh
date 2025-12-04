# ensure build folder exists
if [ ! -d "build" ]; then
    mkdir "build"
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cd ..
fi

# build
cd build
cmake --build .
if [ $? -ne 0 ]; then
    exit 1
fi
cd ..
