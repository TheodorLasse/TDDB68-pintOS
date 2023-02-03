#!/bin/bash

COMPILER="$1"
if [ -z "${COMPILER}" ]; then
    COMPILER="gcc -m32"
    echo "No parameter given, defaulting to ${COMPILER}"
fi

PINTOS=~/tddb68/pintos
cd ${PINTOS}/src/
make clean CC="${COMPILER}"

cd ${PINTOS}/src/utils
make CC="${COMPILER}"
ln -s $(which qemu-system-i386) qemu
chmod +x pintos pintos-mkdisk backtrace pintos-gdb
cd ${PINTOS}/src/threads
make CC="${COMPILER}"

cd ${PINTOS}/src/examples
make CC="${COMPILER}"
cd ${PINTOS}/src/userprog
make CC="${COMPILER}"

cd ${PINTOS}/src/userprog/build
pintos-mkdisk fs.dsk 2
pintos -v --qemu -- -f -q

#pintos -v --qemu -p ../../examples/mytest -a prog -- -q


#pintos -v --qemu -- run 'lab4test1 arg1 arg2 arg3'

make check
#pintos -v --qemu -p ../../examples/lab4test1 -a prog -- -q
#pintos -v --qemu -p ../../examples/printf -a printf -- -q
#pintos -v --qemu -- -q ls
#pintos -v --qemu -- run prog