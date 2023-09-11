import os

fd = os.open("test.txt", os.O_RDWR|os.O_CREAT)

os.write(fd, "test")

os.fsync(fd)

os.close(fd)