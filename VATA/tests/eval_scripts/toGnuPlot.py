#! /bin/python

import sys

# @param1 option for ac, hkc, ca
# @param2 hkc ocaml transoformed file
# @param3 vata file


if len(sys.argv) < 4:
  print "no enought arguments"
  exit(1)

fhk = open(sys.argv[2],'r')

for lh in fhk:
  fv = open(sys.argv[3],'r')
  sh = lh.split(";")
  for lv in fv:
    vh = lv.split(";")
    if len(vh) < 3:
      continue
    if (sh[0]==vh[0] and sh[2]==vh[1]):
      if (sys.argv[1] == "-a"):
        print str((int(sh[1])+int(sh[3])))+" "+vh[3].strip()+" "+vh[4].strip()
      if (sys.argv[1] == "-c"):
        print str((int(sh[1])+int(sh[3])))+" "+vh[2].strip()+" "+vh[4].strip()
      if (sys.argv[1] == "-o"):
        print str((int(sh[1])+int(sh[3])))+" "+sh[4].strip()+" "+vh[4].strip()
  fv.close()
