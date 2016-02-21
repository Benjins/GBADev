curl -L http://sourceforge.net/projects/devkitpro/files/Automated%20Installer/devkitARMupdate.pl/download -o devkitARMupdate.pl
chmod +x ./devkitARMupdate.pl
wd='pwd'
./devkitARMupdate.pl "$wd/devkitpro"
echo "export DEVKITPRO=$wd/devkitpro" >> ~/.bashrc
echo "export DEVKITARM=$wd/devkitpro/devkitARM" >> ~/.bashrc
echo "export PATH=$PATH:$wd/devkitpro/devkitARM/bin" >> ~/.bashrc
source ~/.bashrc