#!/bin/bash
cd "$(dirname "${BASH_SOURCE[0]}" )"
scriptDir=$(pwd)
MANUAL_BUILDS="$scriptDir/cmake-build-manual"


fatal() {
    echo >&2 "$*"
    exit 1
}

SUDO=""
if [ "$(id -u)" != "0" ]; then
  SUDO="sudo"
fi

$SUDO apt install -y \
      bison \
      flex 
