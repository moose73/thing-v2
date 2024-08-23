import os
import imageio, time
# path = '001.bmp'

import serial

ser = serial.Serial('/dev/cu.usbserial-3140' , 115200, timeout=1)

def conv_to_16b(px):
    return (px[0] // 8) << 11 | (px[1] // 4) << 5 | (px[2] // 8)
ctr = 0
for path in (sorted(['yhi.bmp'])):
    if path[-4:] == '.bmp':
        print(path)
        image= imageio.imread(path)
        for n, i in enumerate(image):
            for j in i:
                bt = int(conv_to_16b(j))
                
                time.sleep(0.002)
                ser.write(bt.to_bytes(2, 'big'))

                while (ser.in_waiting):
                    print((ser.read().decode('ascii')), end='')
                ctr += 1
                if (ctr % 1000) == 0:
                    print(ctr, f"{n} / {len(image)}")
