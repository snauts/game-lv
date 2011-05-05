#!/bin/bash

if [ ! -e ../source/game-lv ]; then
    make -C ../source/
fi

if [ -e ../source/game-lv ]; then
    ../source/game-lv -f
fi