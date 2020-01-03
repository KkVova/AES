#!/bin/bash
mkdir build
clang-cl -Igoogletest/googletest/include -Igoogletest/googletest -c googletest/googletest/src/gtest-all.cc -o build/gtest-all.o
ar -rv build/libgtest.lib build/gtest-all.o
clang-cl -I ./ -Igoogletest/googletest/include/ test.cpp aes/aes.cpp diffway/diffway.cpp helper/helper.cpp build/libgtest.lib -o ../AES/build/test.exe 
./build/test.exe --gtest_filter=-*Precompute_MixColumns_And_InvMixColumns*:*Precompute_SubBytes_And_Inv_SubBytes*
rm -rf build