#!/bin/bash

make -C ../source/

if [ -e ../source/game-lv ]; then
    ../source/game-lv $*
fi
