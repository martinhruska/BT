#! /bin/bash

CURRENT_DIR=`pwd`
OPTIONS="-incl"
CHECK_DIR=$1

for d in `ls $CHECK_DIR`
do
				d=$CHECK_DIR/$d
				for i in `ls $d`
				do
					for j in `ls $d`
					do
						$CURRENT_DIR/hkc $OPTIONS $d/$i $d/$j
						echo "||||||||||||||||||||||||||||||||||||||||||||"
					done
				done
				echo "||||||||||||||||||||||||||||||||||||||||||||"
done
