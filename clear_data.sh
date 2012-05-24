#!/bin/bash

find . -name '*.csv*' -exec rm -f {} \;
find . -name 'data_gatherer' -exec rm -f {} \;

echo -e "\e[00;32mdata cleared\e[00m"