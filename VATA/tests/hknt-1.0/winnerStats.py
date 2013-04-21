#! bin/python

import sys

f = open(sys.argv[1],'r')

hs = 0
vs = 0

for l in f:
  r = l.split(' ')
  if (float(r[1]) > float(r[2])):
    hs = hs +1
  else:
    vs = vs+1

print hs,vs
print float(hs)/(hs+vs),float(vs)/(hs+vs)
