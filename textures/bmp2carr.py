#!/usr/bin/python3

from PIL import Image
import numpy as np
import sys

# call this script with 2 arguments
# bmp2carr bmp_file_to_use c_array_name

im = Image.open(sys.argv[1])
p = np.array(im)

# convert to array of arrays of R,G,B values
l = p.reshape((len(p.flatten())//3, 3)).tolist()

i = 0
cbyte = 0

print('byte ', sys.argv[2], '[] = {', end='', sep='')

for r in l:
	# black and white pixels are allowed, anything else is transparent
	if (r[0] == 0 and r[1] == 0 and r[2] == 0):
		cbyte+=1
	elif (r[0] == 255 and r[1] == 255 and r[2] == 255):
		cbyte+=0
	else:
		cbyte+=2
		
	i += 1

	# is packing 4 pixels into a byte done?
	if (i == 4):
		print(cbyte, end=',')
		i = 0
		cbyte = 0

	cbyte <<= 2

print('};')