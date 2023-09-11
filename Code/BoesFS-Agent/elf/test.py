import time
import os

flag = 0

while flag < 40:
    flag += 1
    time.sleep(1)
    result = os.access("/home/boes/test.txt",0)
    if result :
        print("success")
    else:
        print("fail")