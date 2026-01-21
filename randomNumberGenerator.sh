#!/bin/bash
OUTPUT_FILE="random_numbers_shuf.txt"
MIN=1
MAX=2000
COUNT=25000
shuf -i $MIN-$MAX -n $COUNT > "$OUTPUT_FILE"
echo "Generated $COUNT random numbers (range $MIN-$MAX) in $OUTPUT_FILE"

