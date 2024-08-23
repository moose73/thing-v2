

from PIL import Image
import numpy as np
i = Image.open("/Users/sekharm/Downloads/images-2.jpeg")

w, h = 30, 30

i = i.resize((w, h))

pixels = i.load() # this is not a list, nor is it list()'able
width, height = i.size
print(width, height)
print(pixels[0, 0]) # this is a RGBA tuple


all_pixels = []
for x in range(width):
    for y in range(height):
        cpixel = pixels[x, y]
        # print(cpixel[0])
        wht = all([i > 230 for i in cpixel])
        # if (round(sum(cpixel)) / float(len(cpixel))) > 127:
            # all_pixels.append(255)
        # else:
            # all_pixels.append(0)
        all_pixels.append(0 if wht else 1)

all_pixels = np.array(all_pixels).reshape((width, height))

#rotate this 180 degrees
all_pixels = np.rot90(all_pixels, 4)
# all_pixels = np.rot()
for i in all_pixels:
    i[0] = 0
arr_name = "plane"
# print this as a 2D array readable in C
print("char %s[%d][%d] = {" % (arr_name, width, height))
for row in all_pixels:
    print("{", end="")
    for col in row:
        print("%d, " % col, end="")
    print("},")
print("};")


# print(all_pixels)