#!/bin/bash
mkdir build
clang++ -Igoogletest/googletest/include -Igoogletest/googletest -c googletest/googletest/src/gtest-all.cc -o build/gtest-all.o
ar -rv build/libgtest.a build/gtest-all.o
clang++ -I ./ -Igoogletest/googletest/include/ test.cpp aes.cpp helper.cpp build/libgtest.a -o ../AES/build/test -pthread
./build/test
rm -rf build