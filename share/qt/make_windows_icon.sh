#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/era.ico

convert ../../src/qt/res/icons/era-16.png ../../src/qt/res/icons/era-32.png ../../src/qt/res/icons/era-48.png ${ICON_DST}
