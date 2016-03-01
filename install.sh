#!/usr/bin/env sh

# TODO: check that we are in top level (.git?)
# clear build directory
rm -rf build
mkdir build && cd build || exit 1

# build
cmake .. -DCMAKE_BUILD_TYPE=Release
make install

# link compile_commands.json for source code completion
if [ -f compile_commands.json ]; then
    cd .. || exit 1
    if [ ! -e compile_commands.json ]; then
        echo "linking build/compile_commands.json to top dir..."
        ln -s build/compile_commands.json ./
    fi
else
    echo "no compile_commands.json in build directory"
fi
