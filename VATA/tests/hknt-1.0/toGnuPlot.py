#! /bin/python

import sys

if len(sys.argv) < 3:
  print "no enought arguments"
  exit(1)

fhk = open(sys.argv[1],'r')

for lh in fhk:
  fv = open(sys.argv[2],'r')
  sh = lh.split(";")
  for lv in fv:
    vh = lv.split(";")
    if len(vh) < 3:
      continue
    if (sh[0]==vh[0] and sh[2]==vh[1]):
      print str((int(sh[1])+int(sh[3])))+" "+vh[2].strip()+" "+vh[7].strip()
  fv.close()
