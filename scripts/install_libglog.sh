#!/usr/bin/env bash

set -euo pipefail
git submodule update --init --recursive

# change working directory
SCRIPT_PATH=$(dirname ${BASH_SOURCE})
cd $SCRIPT_PATH

# go to library directory and build library
cd ../lib/glog/
./autogen.sh
./configure
make

# install library files (needs root privileges)
sudo make install
