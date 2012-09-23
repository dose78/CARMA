#!/bin/bash
cd "$( dirname "$0" )"

rm -f data.csv
echo "algorithm,m,k,n,threads,gflop/s" > data.csv

iterations=$1
echo -e "\e[0;36mwill run $iterations iterations\e[0m"

shift

for (( i=1; i<=$iterations; i++ )); do
  echo -e "\e[0;36mrunning iteration $i\e[0m"
  for alg in "$@"; do
    bash sweeper.sh $alg
  done
done

echo -e "\e[0;36mcollating data...\e[0m"

python collator.py $iterations

echo -e "\e[0;32mDONE\e[0m"
