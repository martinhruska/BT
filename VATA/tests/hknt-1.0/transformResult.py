#! /bin/python

import sys
import re

if len(sys.argv) < 2:
  print "No argument"

f = open(sys.argv[1],'r')

state = 1

for l in f:
  if state == 0:
    state = 1
  elif state == 1:
    m =  re.search('.*(armcNFA_inclTest_[0-9]*): ([1-9][0-9.]*) states',l)
    res = m.group(1)+";"+m.group(2)+";"
    state = state+1
  elif state == 2:
    m =  re.search('.*(armcNFA_inclTest_[0-9]*): ([1-9][0-9.]*) states',l)
    res = res + m.group(1)+";"+m.group(2)+";"
    state = state+1
  elif state == 3:
    state = state+1
  elif state == 4:
    m =  re.search('([0-9][0-9]*\.[0-9][0-9]*) seconds.*',l)
    state = 0
    res = res+m.group(1)
    print res
