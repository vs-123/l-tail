#!/bin/sh
FILE=".test.log"
QUERY="INFO"

echo "[INFO] BENCHMARKING L-TAIL..."
/usr/bin/time -l ./l-tail --once $FILE "$QUERY" > /dev/null 2> l-tail-mem.txt

echo "[INFO] BENCHMARKING GREP..."
/usr/bin/time -l grep "$QUERY" $FILE > /dev/null 2> grep-mem.txt

echo "[RESULTS]"
echo "===  L-TAIL  ==="
cat l-tail-mem.txt

echo "===  GREP  ==="
cat grep-mem.txt
