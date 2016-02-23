declare -a projects=("topDown" "actionRpg" "sdkTest")

wd=`pwd`
echo "export DEVKITPRO=$wd/devkitpro" >> ~/.bashrc
echo "export DEVKITARM=$wd/devkitpro/devkitARM" >> ~/.bashrc
echo "export PATH=$PATH:$wd/devkitpro/devkitARM/bin" >> ~/.bashrc
source ~/.bashrc

for i in "${projects[@]}"
do
	. ./pack-assets.sh "$i"
	. ./build.sh "$i"
done