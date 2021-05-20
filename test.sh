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


isApp() {
    if [ -x "$1" -a -f "$1" ] ; then
        return 0
    fi
    return 1
}
#for dir in ${CMAKE_BUILD_DIR} cmake-build-debug cmake-build-release build ; do
for dir in ${CMAKE_BUILD_DIR} ; do
    if isApp "$scriptDir/$dir/moc/moc"; then
        MOC_TOOL="$scriptDir/$dir/moc/moc"
        break
    elif isApp "$scriptDir/$dir/moc/moc.exe"; then
        MOC_TOOL="$scriptDir/$dir/moc/moc.exe"
        break
    elif isApp "$scriptDir/$dir/moc"; then
        MOC_TOOL="$scriptDir/$dir/moc"
        break
    elif isApp "$scriptDir/$dir/moc.exe"; then
        MOC_TOOL="$scriptDir/$dir/moc.exe"
        break
    fi
done

[ ! -z ${MOC_TOOL} ] || fatal "Run the build first"

if [ $# -gt 0 ] ; then
    echo "${MOC_TOOL}" "$@"
    exec "${MOC_TOOL}" "$@"
else
    set -e
    cd $scriptDir
    "${CMAKE_BUILD_DIR}/uf/ufPtrTest"
    "${CMAKE_BUILD_DIR}/uf/ufListTest"
    "${CMAKE_BUILD_DIR}/uf/ufIHshTTest"
    "${CMAKE_BUILD_DIR}/uf/ufSHshTTest"
    cd $scriptDir/test_app
    echo "$MOC_TOOL" -o "$scriptDir/${CMAKE_BUILD_DIR}/gen" -T test_app.tpl test_app.moc
    "$MOC_TOOL" -o "$scriptDir/${CMAKE_BUILD_DIR}/gen" -T test_app.tpl test_app.moc
fi

