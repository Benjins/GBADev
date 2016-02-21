curl -L http://sourceforge.net/projects/devkitpro/files/Automated%20Installer/devkitARMupdate.pl/download -o devkitARMupdate.pl
chmod +x ./devkitARMupdate.pl
wd=`pwd`
./devkitARMupdate.pl "$wd/devkitpro"