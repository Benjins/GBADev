set -e

declare -a projects=("topDown" "actionRpg" "sdkTest" "doubleShoot")

for i in "${projects[@]}"
do
	. ./pack-assets.sh "$i"
	. ./travis_build_project.sh "$i"
done