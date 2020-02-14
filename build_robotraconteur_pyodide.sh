#!/bin/bash

set -e

PYODIDE_ROOT=$HOME/pyodide
PYODIDE_PACKAGE_ABI=1

BOOST_VERSION=1.71.0

CURRENT_DIR=$(dirname "$(readlink -f '$0')")

BOOST_VERSION2="${BOOST_VERSION//./_}"
BOOST_TARBALL="$CURRENT_DIR/build_boost/boost_$BOOST_VERSION2.tar.bz2"

BOOST_BUILD_DIR="$CURRENT_DIR/build_boost/boost_$BOOST_VERSION2"
BOOST_LIB_DIR=$BOOST_BUILD_DIR/stage/lib

BOOST_EXTRA_FLAGS="-fpic"

BOOST_LIBS_COMMA=$(echo $BOOST_LIBS | sed -e 's/[[:space:]]/,/g')

source $PYODIDE_ROOT/emsdk/emsdk/emsdk_env.sh

SIDE_C_FLAGS="-s DISABLE_EXCEPTION_CATCHING=0 -s EXCEPTION_DEBUG=0 -s ASSERTIONS=0 -fpic -O3 -std=c++14"
SIDE_LDFLAGS="-s \"BINARYEN_METHOD='native-wasm'\" -Werror -s EMULATED_FUNCTION_POINTERS=1 -s EMULATE_FUNCTION_POINTER_CASTS=1 -s WASM=1  --memory-init-file 0 -s EXPORT_ALL=1 -s DISABLE_EXCEPTION_CATCHING=0 -s EXCEPTION_DEBUG=0 -s ASSERTIONS=0 -O3 -std=c++14"

#
#-s \"BINARYEN_TRAP_MODE='clamp'\"

mkdir -p build_boost
if [ ! -s $BOOST_TARBALL ]; then
	( cd $CURRENT_DIR/build_boost && wget https://dl.bintray.com/boostorg/release/$BOOST_VERSION/source/boost_$BOOST_VERSION2.tar.bz2  && tar xf boost_$BOOST_VERSION2.tar.bz2)
	echo Do download
fi

if [ ! -s $BOOST_BUILD_DIR/b2 ]; then
	( cd $BOOST_BUILD_DIR && ./bootstrap.sh --with-libraries=date_time,filesystem,system,regex,chrono,random)
fi

if [ ! -s $BOOST_BUILD_DIR/stage/lib/libboost_regex.bc ]; then
	( cd $BOOST_BUILD_DIR && $BOOST_BUILD_DIR/b2 toolset=emscripten link=static --compileflags="$SIDE_C_FLAGS" --linkflags="-fpic $SIDE_LDFLAGS" --with-date_time --with-filesystem --with-system --with-regex --with-chrono --with-random --disable-icu )
fi

mkdir -p build

echo $BOOST_LIB_DIR

if [ -f $CURRENT_DIR/build/CMakeCache.txt ]; then
	rm $CURRENT_DIR/build/CMakeCache.txt
fi

(cd $CURRENT_DIR/build && cmake .. -DCMAKE_TOOLCHAIN_FILE=$PYODIDE_ROOT/emsdk/emsdk/emscripten/tag-1.38.30/cmake/Modules/Platform/Emscripten.cmake -DBUILD_PYTHON=ON -DBOOST_INCLUDEDIR=$BOOST_BUILD_DIR \
-DBOOST_LIBRARYDIR=$BOOST_BUILD_DIR/stage/lib -DBoost_ADDITIONAL_VERSIONS="1.71;1.71.0" \
-DBoost_DATE_TIME_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_date_time.bc -DBoost_DATE_TIME_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_date_time.bc \
-DBoost_FILESYSTEM_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_filesystem.bc -DBoost_FILESYSTEM_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_filesystem.bc \
-DBoost_SYSTEM_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_system.bc -DBoost_SYSTEM_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_system.bc \
-DBoost_REGEX_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_regex.bc -DBoost_SYSTEM_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_regex.bc \
-DBoost_CHRONO_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_chrono.bc -DBoost_CHRONO_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_crono.bc \
-DBoost_RANDOM_LIBRARY_RELEASE=$BOOST_LIB_DIR/libboost_random.bc -DBoost_RANDOM_LIBRARY_DEBUG=$BOOST_LIB_DIR/libboost_random.bc \
-DPYTHON_EXECUTABLE=$PYODIDE_ROOT/cpython/build/3.7.0/host/bin/python3 \
-DPYTHON_LIBRARY=$PYODIDE_ROOT/cpython/installs/python-3.7.0/lib/libpython3.7.a \
-DPYTHON_INCLUDE_DIR=$PYODIDE_ROOT/cpython/installs/python-3.7.0/include/python3.7 \
-DCMAKE_SHARED_LINKER_FLAGS="$SIDE_LDFLAGS" \
-DCMAKE_MODULE_LINKER_FLAGS="$SIDE_LDFLAGS" \
-DCMAKE_CXX_FLAGS="$SIDE_C_FLAGS" \
-DCMAKE_C_FLAGS="$SIDE_C_FLAGS" \
)

( cd $CURRENT_DIR/build && make VERBOSE=1 -j1 )

PYODIDE_RR_DIR=$PYODIDE_ROOT/root/lib/python3.7/site-packages/RobotRaconteur

mkdir -p $PYODIDE_RR_DIR

cp $CURRENT_DIR/build/out/Python/RobotRaconteur/*.py $PYODIDE_RR_DIR

touch $PYODIDE_ROOT/root/.rrbuilt

( cd $PYODIDE_ROOT && make )
