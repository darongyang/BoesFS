import mmap

with open('make.sh','r') as f:
    with mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as m:
        print(m.read(10))