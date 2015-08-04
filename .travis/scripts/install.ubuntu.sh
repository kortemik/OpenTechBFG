#!/bin/bash

pushd .
cd ${HOME}
if [ "$CXX" = "g++" ]; then sudo sed -i 's/precise/trusty/g' /etc/apt/sources.list; fi
if [ "$CXX" = "mingw" ]; then sudo sed -i 's/precise/trusty/g' /etc/apt/sources.list; fi
sudo apt-get update -qq
sudo apt-get install libsdl1.2-dev libopenal-dev
if [ "$CXX" = "g++" ]; then sudo apt-get install -qqy gcc g++; fi
if [ "$CXX" = "mingw" ]; then sudo apt-get install -qqy g++-mingw-w64-x86-64 cmake; fi
popd 
