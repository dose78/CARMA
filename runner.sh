#!/bin/bash

bash script/clear_data.sh

iterations=$1
echo -e "\e[00;36mwill run $iterations iterations\e[00m"

shift

for (( i=1; i<=$iterations; i++ ))
do
  echo -e "\e[00;36mrunning iteration $i\e[00m"
  for folder in "$@"
  do
    cd $folder
    bash data_gatherer.sh "data$i.csv"
    cd ..
  done
done

echo -e "\e[00;36mcollating data...\e[00m"

python script/data_collator.py $iterations $@

echo -e "\e[00;32mDONE\e[00m"
