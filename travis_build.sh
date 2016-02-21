declare -a projects=("topDown" "actionRpg" "sdkTest")

source ~/.bashrc

for i in "${projects[@]}"
do
	./pack-assets.sh "$i"
	./travis_build_project.sh "$i"
done