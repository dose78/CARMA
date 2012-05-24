#!/bin/bash

bash clear_data.sh

iterations=$1
echo "will run $iterations iterations"

shift

for (( i=1; i<=$iterations; i++ ))
do
  echo "running iteration $i"
  for folder in "$@"
  do
    cd $folder
    bash data_gatherer.sh "data$i.csv"
    cd ..
  done
done
echo -e "\e[00;32mDONE\e[00m"