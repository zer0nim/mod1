#!/bin/bash

# 42 vm instalation dependencies (xubuntu 18.04 LTS)
sudo apt-get update
# install cmake > 3.13
sudo apt-get remove cmake
sudo snap install cmake --classic
# install opengl dependencies
sudo apt-get install libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev
