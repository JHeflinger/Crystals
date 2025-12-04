# ensure build folder exists
if [ ! -d "build" ]; then
    mkdir "build"
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cd ..
fi

# initialize vars for building
SRC_DIR="src"
INCLUDES=""
SOURCES=""
LIBS=""
LINKS=""

# get src includes and sources
while IFS= read -r dir; do
    INCLUDES="$INCLUDES -I$dir"
done < <(find "$SRC_DIR" -type d)
while IFS= read -r file; do
    SOURCES="$SOURCES $file"
done < <(find "$SRC_DIR" -type f -name "*.cpp")

# build
cd build
cmake --build .
if [ $? -ne 0 ]; then
    exit 1
fi
cd ..
