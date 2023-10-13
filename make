#/bin/sh
clang -D__LINUX__ -O3 -or4 *.cpp
ls -l ./r4
cp r4 ~/.local/bin/
