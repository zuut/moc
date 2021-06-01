#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}" )"
scriptDir=$(pwd)

. "$scriptDir/environment.sh"
cd "$scriptDir"

PROGRAM=$0

fatal() {
    echo >&2 "$*"
    exit 1
}

[ ! -z "$(type -p cmake)" ] || fatal "Unable to find cmake"
[ ! -z "$(type -p bison)" ] || fatal "Unable to find bison.  Did you run the initial setup script?"
[ ! -z "$(type -p flex)" ] || fatal "Unable to find flex. Did you run the initial setup script?"


Usage() {
    printf "USAGE: $PROGRAM [build|test|clean|install|help]\n"
}

MAKE_DIR=$CMAKE_BUILD_DIR
if [ $# = 1 ] ; then
    case "$1" in
        build)
            MAKE_OPTIONS="--build ."
            MAKE_COMMAND=cmake
            shift
            ;;
        clean)
            cd $scriptDir
            exec rm -rf "$CMAKE_BUILD_DIR" "CMakeFiles" "moc/genSrc" "moc/CMakeFiles"  "uf/CMakeFiles" 
            ;;
        install)
            MAKE_OPTIONS="--build . --target install"
            MAKE_COMMAND=cmake
            shift
            ;;
        test)
            MAKE_OPTIONS=
            MAKE_COMMAND=./test.sh
            MAKE_DIR=$scriptDir
            shift
            ;;
        help)
            Usage;
            exit 0
            ;;
        *)
            MAKE_OPTIONS="--build ."
            MAKE_COMMAND=cmake
            ;;
    esac
else
    MAKE_OPTIONS="--build . "
    MAKE_COMMAND=cmake
fi

mkdir -p "${CMAKE_BUILD_DIR}"
cd "${CMAKE_BUILD_DIR}"

echo cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..

cd "$scriptDir"
cd "$MAKE_DIR"
echo $MAKE_COMMAND $MAKE_OPTIONS "$@"
exec $MAKE_COMMAND $MAKE_OPTIONS "$@"



