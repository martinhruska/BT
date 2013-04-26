#! bin/python

import sys

f = open(sys.argv[1],'r')

hs = 0
vs = 0
acch = 0.0
accv = 0.0

for l in f:
  r = l.split(' ')
  if (float(r[1]) > float(r[2])):
    if (float(r[2]) == 0):
      accv = accv + 10
    else:
      accv = accv + float(r[1]) / float(r[2])
    vs = vs +1
  elif float(r[1]) < float(r[2]) :
    if (float(r[1]) == 0):
      acch = acch + 10
    else:
      acch = acch + float(r[2]) / float(r[1])
    hs = hs+1

print hs,vs
print float(hs)/(hs+vs),float(vs)/(hs+vs)
print acch/(hs+vs), accv/(hs+vs)
