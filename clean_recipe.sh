#/usr/bin/env bash
cd "$(dirname "${BASH_SOURCE[0]}" )"
scriptDir=$(pwd)
export RECIPE_FOLDER=${RECIPE_FOLDER:-recipe_check}

cd "$scriptDir"

[ ! -z "$RECIPE_FOLDER" ] && rm -rf "$RECIPE_FOLDER"


