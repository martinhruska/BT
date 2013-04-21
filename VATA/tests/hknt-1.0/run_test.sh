#! /bin/bash

CURRENT_DIR=`pwd`
OPTIONS="-incl"
CHECK_DIR=$1

for i in `ls $CHECK_DIR`
do
  for j in `ls $CHECK_DIR`
  do
    $CURRENT_DIR/hkc $OPTIONS $CHECK_DIR$i $CHECK_DIR$j
    echo "||||||||||||||||||||||||||||||||||||||||||||"
  done
done
