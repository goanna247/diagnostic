## Install KDevelop
sudo snap install kdevelop --classic

sudo apt install libevent-2.1-7 
^ library needed for kdevelop to run 

## Install CMake
sudo apt update
sudo apt install cmake


## To run the install script:
chmod +x install.sh

./install.sh

^Warning this has not yet been tested and may well not work.


When you get into kdevelop -> change the build settings and the executable settings (after building successfully) 

## Some libraries you may also need:
sudo apt install libwxgtk3.2-dev
sudo apt install pkg-config
sudo apt install libglew-dev


sudo apt install meson ninja-build
