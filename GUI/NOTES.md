# Installation

this program is currently only intended to run on Debian or Ubuntu distros. anyone using windows machines can utilize WSL2 as a substitute, if this is the case for you then make sure you have the latest version by running the following command in a powershell window:
~~~
wsl --update
~~~

installing all required dependencies to run this program can be done using the following commands:
~~~
sudo apt update
~~~
~~~
sudo apt install build-essential cmake ninja-build qt6-base-dev
~~~

the commands above will install Qt 6.4.2 by default; however, this project uses Qt version 6.5. To update Qt, use the following comamnds and then restart your shell:
~~~
sudo apt install pipx
pipx ensurepath
pipx install aqtinstall
~~~
note: we use pipx because some Ubuntu versions, like 24.04, block system-wide pip installs

once your shell has been restarted, use the next batch of commands to finish updating Qt:
~~~
aqt install-qt linux desktop 6.5.3 gcc_64 -O ~/Qt
~~~
~~~
sudo apt install build-essential cmake ninja-build libgl1-mesa-dev \
    libxkbcommon-x11-0 libxcb-cursor0 libxcb-icccm4 libxcb-image0 \
    libxcb-keysyms1 libxcb-render-util0 libxcb-xinerama0 libxcb-shape0
~~~



# Building and Running

now that all dependencies have been installed, to build and run this project:
1. First, navigate into the directory the project is installed within
2. run the following commands to build the project:
~~~
cmake -B build -S . -G Ninja -DCMAKE_PREFIX_PATH=$HOME/Qt/6.5.3/gcc_64
cmake --build build
~~~
3. finally, to run the project, run this command:
~~~
./build/FaultMonitoringSystem
~~~
