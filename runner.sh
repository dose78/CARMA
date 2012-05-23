#!/bin/bash

iterations=$1
echo "will run $iterations iterations"

shift

for (( i=1; i<=$iterations; i++ ))
do
  echo "running iteration $i"
  for folder in "$@"
  do
    cd $folder
    bash data_gatherer.sh
    cd ..
  done
done
