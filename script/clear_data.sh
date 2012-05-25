#!/bin/bash

dir="$( dirname "${BASH_SOURCE[0]}")/.."

find $dir -name '*.csv*' -exec rm -f {} \;
find $dir -name 'data_gatherer' -exec rm -f {} \;

echo -e "\e[00;32mdata cleared\e[00m"
