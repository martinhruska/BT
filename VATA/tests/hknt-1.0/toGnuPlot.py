#! /bin/python

import sys

if len(sys.argv) < 3:
  print "no enought arguments"
  exit(1)

fhk = open(sys.argv[1],'r')
fv = open(sys.argv[2],'r')

for lh in fhk:
  sh = lh.split(";")
  for lv in fv:
    vh = lv.split(";")
    if len(vh) < 3:
      continue
    if (sh[0]==vh[0] and sh[2]==vh[1]):
      print sh[1]+" "+sh[3]+" "+sh[4]+" "+vh[7]
