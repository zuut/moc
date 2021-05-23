#/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}" )"
scriptDir=$(pwd)

cd "$scriptDir"

fatal() {
    echo >&2 $*
    exit 1
}

MOC_VERSION=$(grep "version = " conanfile.py | awk 'NR==1{print $3}' | awk -F\" '{print $2 }')
[ ! -z "$MOC_VERSION" ] || fatal USAGE $0 {version}. e.g. $0 0.9.1

BUILD_FOLDER=cmake-build-debug 
rm -rf "${BUILD_FOLDER}"
mkdir "${BUILD_FOLDER}"
cd "${BUILD_FOLDER}"
printf "\n\n\nSETTING UP CONAN CMAKEFILES\n";
conan install ..
printf "\n\n\nUSING CONAN TO BUILD BINARIES\n";
conan build ..
printf "\n\n\nPACKAGING CONAN BINARIES\n";
conan package ..
printf "\n\n\nEXPORTING CONAN PACKAGE TO LOCAL CACHE\n";
echo "Using version ${MOC_VERSION} from conanfile.py"

echo "conan export-pkg -f .. _/_"
conan export-pkg -f .. _/_
