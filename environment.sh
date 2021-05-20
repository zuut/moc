export PROJECT_NAME=Moc

envScriptDir=$(command cd $(dirname "${BASH_SOURCE[0]}" ) && command pwd)

export CMAKE_OPTIONS=

export CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Debug}
export CMAKE_BUILD_DIR=${CMAKE_BUILD_DIR:-cmake-build-debug}
#export CMAKE_INSTALL_PREFIX=/usr/local #
export CMAKE_INSTALL_PREFIX=${envScriptDir}/../installed/share
[ ! -z "$CMAKE_INSTALL_PREFIX" ] && CMAKE_OPTIONS="${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}"
export CMAKE_OPTIONS;

