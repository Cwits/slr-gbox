# Software Setup Guide

This part shouldn’t be that difficult than hardware assembly, but still may require some linux knowledge.  

## Preparation
Install and update Raspberry Pi OS on SD card or HDD, login and do update and upgrade. (Don't forget to configure SSH access and user\password before flashing image).  
I'm using Raspberry Pi OS Lite, so there will be no extra dependencies that we don't need. If you want to use different OS than some extra steps may be involved.  

There is plenty of guides in internet how to do this step, so I won't explain it in details.  

After updating you need to install required dependencies:  
- libevdev-dev (required by GestLib)  
- nlohmann-json3-dev  
- libgtest-dev  
- libsdl2-dev  
- libsndfile1-dev  
- jackd2  
- libjack-jackd2-dev  

Or by copy-paste following command:  
### ⚠️ **WARNINIG**: During jackd installation, when prompted about real-time priority, choose “Yes”.  

> sudo apt install libevdev-dev nlohmann-json3-dev libgtest-dev libsdl2-dev libsndfile1-dev jackd2 libjack-jackd2-dev

You may also need some basic components for building, but usually they are included in OS image:

- gcc
- g++
- build-essential
- cmake
- python3

## Configuring system  
### DTO:  
From docs/dto folder copy the slraudio.dtbo file to /boot/firmware/overlays/  

> sudo cp slraudio.dtbo /boot/firmware/overlays/

### Enabling i2s:  
Open config.txt located in /boot/firmware/  

> sudo nano /boot/firmware/config.txt

And modify it by uncommenting:  

> dtparam=i2s=on

As well as adding:  

> dtoverlay=slraudio

### Configuring jack  
Configure for jack as described in this link  
https://jackaudio.org/faq/linux_rt_config.html  

### Reboot the Raspberry Pi  

> sudo reboot  

## Building and Running 

> git clone --recurse-submodules https://github.com/Cwits/slr-gbox.git  
> cd slr-gbox  
> mkdir build && cd build  
> cmake ..  
> make -j4  
> ./slr  

## Configuring to autorun  
If you want the software to start automatically on boot, you can create a systemd service or modify rc.local. (Details to be added later.)  

Edit ~/.bash_profile by adding path to executable at the end of the file(if there is no such file than create it)

>if [ -z "$SSH_CONNECTION" ]; then
>	pathToBuild/slr
>fi

## Tips and hints:  
If you plan to modify the code frequently, skip automatic startup during development.  

You can also build and run the application in WSL or any generic Linux environment, but there will be no audio IO since than "Dummy Driver" will be used(not Jack).  
This simplifies development since you don’t need to compile directly on the Raspberry Pi.  
A Fake Gesture Driver is included to simulate touchscreen input using the keyboard and mouse:

> Z - Tap  
> X - Hold  
> C - Drag  
> V - Swipe  
> B - Double Tap  
> N - Double Tap Swipe  

**Important Notes** 
- Pretty frequently SDL in WSL misses mouse and keyboard events, so actions can be unfinished/untriggered.
- If you are cloning repository to other directory than $HOME than you must change _pathPostfix variable in SettingsManager.cpp accordingly