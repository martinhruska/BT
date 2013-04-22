import sys

if (len(sys.argv) == 1):
  print("Please enter the file to process")
  exit(1);

file = open(sys.argv[1],'r')

temp = []
for i in range(0,6):
  temp = file.readline()
neco = temp.split(';')[2:-2] 
print(neco)
res = [1.0]*len(neco)
i=0
for line in file:
  i+=1
  values = line.split()[1:-1]
  j = 0
  for value in values:
    if (value[0] == "-"):
      res[j] += 5
    else:
      res[j] += float(value[0:-1])
    j+=1

r = [""]*len(res)
for x in range(0,len(res)):
  r[x] = "%0.5f"%(res[x]/i)

print(r)
file.close()
