#!/bin/bash
echo "Starting test..."
echo "Starting test with GPU config"
outfile="data2.csv"
rm $outfile

M=32
for eM in {1..8}
do
    ./data -b 600 -c 1 -m 7500 -n $outfile -s 60 -x $M -y $M -z $M
    M=$(($M * 2))
done

echo "Finished test with GPU config"
echo "Starting test with TPU config"

M=32
for eM in {1..8}
do
    ./data -b 900 -c 1 -m 22500 -n $outfile -s 60 -x $M -y $M -z $M
    M=$(($M * 2))
done

echo "Finished test with TPU config"
echo "Finished test..."
