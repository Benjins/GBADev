declare -a projects=("topDown" "actionRpg" "sdkTest")

source ~/.bashrc

for i in "${projects[@]}"
do
   ./build.sh "$i"
done