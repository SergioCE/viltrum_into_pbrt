#!/bin/bash
# NOTE : Quote it else use array to avoid problems #
FILES="*"
for FILE in */; do
  echo "Prcessing $FILE file..."
  # take action on each file. $f store current file name
  cat "$f"
done