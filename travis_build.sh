declare -a projects=("topDown" "actionRpg" "sdkTest")

source ~/.bashrc

for i in "${projects[@]}"
do
   ./travis_build_project.sh "$i"
done