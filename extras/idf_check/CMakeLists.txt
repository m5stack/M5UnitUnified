# ESP-IDF build check for M5UnitUnified. Not part of the library package.
#
#   idf.py -C extras/idf_check set-target esp32
#   idf.py -C extras/idf_check build
#
# The check is that the build succeeds AND that adapter_i2c.cpp reports
# "Support I2C_Class"; see main/main.cpp.
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(m5unitunified_idf_check)
