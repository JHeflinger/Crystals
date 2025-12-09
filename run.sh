./build.sh
if [ $? -ne 0 ]; then
    exit 1
fi
./build/build/spectrum $1 $2 $3 $4 $5 $6
if [ -f "out.png" ]; then
    open out.png
fi
