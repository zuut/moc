#/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}" )"
scriptDir=$(pwd)

export RECIPE_FOLDER=${RECIPE_FOLDER:-recipe_check}
cd "$scriptDir"

./clean_recipe.sh
mkdir -p "${RECIPE_FOLDER}"
cd "${RECIPE_FOLDER}"

conan install  .. --install-folder=cmake-build-debug # [--profile XXXX]
conan source ..  --source-folder=raw_src
conan build ..  --install-folder=cmake-build-debug   --source-folder=raw_src
conan package .. --build-folder=cmake-build-debug --package-folder=package-folder  --source-folder=raw_src
