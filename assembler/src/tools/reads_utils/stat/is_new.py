#!/usr/bin/python -O

import sys

inFile = open(sys.argv[1])
outFile = open(sys.argv[2], 'w')
maxLen = int(sys.argv[3])

hist = {0:0}
for i in range(1,maxLen):
	hist[i] = 0

while (1):
	line = inFile.readline()

	if not line:
		break

	pos1 = int(line.split(' ')[0])
	len1 = int(line.split(' ')[1])

        line = inFile.readline()

        if not line:
                break

       	pos2 = int(line.split(' ')[0])
       	len2 = int(line.split(' ')[1])

	hist[pos2 - pos1 + len2] += 1

sum = 0
for i in range(0,maxLen):
	sum += hist[i]
        outFile.write(str(i) + ' ' + str(hist[i]) + '\n')

print("Total mate-pairs: " + str(sum))

inFile.close()
outFile.close()
