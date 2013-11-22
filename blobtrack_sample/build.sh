#!/bin/bash

for i in *.cpp
do
	g++ -g `pkg-config --cflags opencv` -o `basename $i .cpp`.out $i `pkg-config --libs opencv`;
done
