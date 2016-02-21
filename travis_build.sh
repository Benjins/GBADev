declare -a projects=("topDown" "actionRpg" "sdkTest")

for i in "${projects[@]}"
do
   ./build.sh "$i"
done