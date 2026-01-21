#!/bin/bash
MAX_COUNT=2500000
OUTPUT_FILE="output.txt"
> "$OUTPUT_FILE"
for (( i=1; i<=$MAX_COUNT; i++ ))
do
    echo $RANDOM >> "$OUTPUT_FILE"
done

