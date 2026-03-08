#!/bin/sh

OUTPUT=".test.log"
echo "[INFO] GENERATING $OUTPUT..."

perl -e 'for ($i=1; ; $i++) { printf "INFO: abd%x - This is a test log entry.\n", $i }' | head -c 1073741824 > $OUTPUT

echo "[SUCCESS] DONE"
