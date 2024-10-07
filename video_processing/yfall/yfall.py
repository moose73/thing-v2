import os
import imageio, time
# path = '001.bmp'
import re

import serial

ser = serial.Serial('/dev/cu.usbserial-10' , 921600, timeout=1)

def conv_to_16b(px):
    return (px[0] // 8) << 11 | (px[1] // 4) << 5 | (px[2] // 8)
ctr = 0
for path in (sorted(['yhi.bmp'])):
# print(sorted(os.listdir()))
# for path in ([i for i in sorted(os.listdir()) if re.match(r"\d{3}.bmp", i)][::2]):
    # print(path)
    # continue
    if path[-4:] == '.bmp':
        print(path)
        image= imageio.imread(path)
        for n, i in enumerate(image):
            for j in i:
                bt = int(conv_to_16b(j))
                
                time.sleep(0.0001)
                ser.write(bt.to_bytes(2, 'big'))

                while (ser.in_waiting):
                    print((ser.read().decode('ascii')), end='')
                ctr += 1
                if (ctr % 1000) == 0:
                    print(ctr, f"{n} / {len(image)}", end = "... ")
