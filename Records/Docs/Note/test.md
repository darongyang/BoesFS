

- 组x
  - 性能
    - native
    ```bash
    ```
    - boesfs
    ```bash
    ```
  - 消耗
    - 测试前
      - native
      ```
      ```
      - boesfs
      ```bash
      ```
    - 测试中
      - native
      ```bash
      ```
      - boesfs
      ```bash
      ```
    - 测试后
      - native
      ```
      ```
      - boesfs
      ```bash
      ```


## 组1

- 性能
  - native
  ```bash
  deletefile1          175592ops     2925ops/s   0.0mb/s      4.8ms/op [0.01ms - 200.11ms]
  listdir1             175596ops     2925ops/s  14.5mb/s      0.4ms/op [0.00ms - 203.28ms]
  statfile1            175596ops     2925ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.34ms]
  closefile2           175596ops     2925ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.64ms]
  fsyncfile1           175596ops     2925ops/s   0.0mb/s      7.3ms/op [0.00ms - 311.22ms]
  appendfilerand1      175646ops     2926ops/s  22.9mb/s      0.3ms/op [0.00ms - 67.86ms]
  openfile1            175646ops     2926ops/s   0.0mb/s      0.0ms/op [0.00ms - 15.17ms]
  closefile3           175646ops     2926ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.40ms]
  readfile1            175646ops     2926ops/s 385.0mb/s      0.2ms/op [0.00ms - 94.35ms]
  openfile2            175646ops     2926ops/s   0.0mb/s      0.0ms/op [0.00ms - 17.13ms]
  closefile1           175646ops     2926ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.05ms]
  wrtfile1             175646ops     2926ops/s 363.1mb/s      1.2ms/op [0.00ms - 196.88ms]
  createfile1          175646ops     2926ops/s   0.0mb/s      2.7ms/op [0.01ms - 185.23ms]
  62.287: IO Summary: 2283144 ops 38034.699 ops/s 2926/5852 rd/wr 785.5mb/s   5.6ms/op

  mkdir1               10000ops     9989ops/s   0.0mb/s      0.7ms/op [0.01ms - 89.76ms]
  2.018: IO Summary: 10000 ops 9989.152 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

  2.284: Per-Operation Breakdown
  rmdir1               10000ops     9981ops/s   0.0mb/s      0.6ms/op [0.01ms - 65.00ms]    
  ```
  - boesfs
  ```bash
  deletefile1          169208ops     2819ops/s   0.0mb/s      5.1ms/op [0.01ms - 90.19ms]
  listdir1             169230ops     2819ops/s  14.0mb/s      0.4ms/op [0.01ms - 80.61ms]
  statfile1            169230ops     2819ops/s   0.0mb/s      0.0ms/op [0.00ms -  9.31ms]
  closefile2           169230ops     2819ops/s   0.0mb/s      0.0ms/op [0.00ms - 50.45ms]
  fsyncfile1           169236ops     2819ops/s   0.0mb/s      7.1ms/op [0.00ms - 257.58ms]
  appendfilerand1      169245ops     2820ops/s  22.0mb/s      0.4ms/op [0.01ms - 61.65ms]
  openfile1            169246ops     2820ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.45ms]
  closefile3           169246ops     2820ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.26ms]
  readfile1            169246ops     2820ops/s 369.6mb/s      0.2ms/op [0.01ms - 55.66ms]
  openfile2            169247ops     2820ops/s   0.0mb/s      0.0ms/op [0.01ms - 17.21ms]
  closefile1           169247ops     2820ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.85ms]
  wrtfile1             169247ops     2820ops/s 351.0mb/s      1.3ms/op [0.01ms - 84.88ms]
  createfile1          169248ops     2820ops/s   0.0mb/s      3.0ms/op [0.02ms - 88.40ms]
  62.504: IO Summary: 2200106 ops 36653.169 ops/s 2820/5639 rd/wr 756.6mb/s   5.8ms/op

  mkdir1               10000ops     9991ops/s   0.0mb/s      3.2ms/op [0.01ms - 105.26ms]
  2.024: IO Summary: 10000 ops 9990.729 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

  rmdir1               10000ops     9981ops/s   0.0mb/s      0.8ms/op [0.01ms - 67.43ms]
  2.506: IO Summary: 10000 ops 9980.986 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
  ```
  - 消耗
  - 测试前
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        284M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        273M        2.5G        6.2M        1.0G        3.3G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
  - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        778M        976M        6.2M        2.1G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        784M        956M        6.2M        2.1G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      boes@debian:~/Final/boesfs/Code/test/performance_test$ 
      ```
  - 测试后
      - native
      ```
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        271M        2.5G        6.2M        1.0G        3.3G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ free -h
                  total        used        free      shared  buff/cache   available
      Mem:           3.8G        283M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```

## 组2

- 性能
  - native
  ```bash
  deletefile1          169913ops     2831ops/s   0.0mb/s      4.6ms/op [0.01ms - 95.39ms]
  listdir1             169935ops     2831ops/s  14.1mb/s      0.4ms/op [0.00ms - 52.20ms]
  statfile1            169935ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.24ms]
  closefile2           169935ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.37ms]
  fsyncfile1           169937ops     2831ops/s   0.0mb/s      7.6ms/op [0.00ms - 264.99ms]
  appendfilerand1      169941ops     2831ops/s  22.1mb/s      0.1ms/op [0.00ms - 56.80ms]
  openfile1            169943ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.63ms]
  closefile3           169943ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.23ms]
  readfile1            169943ops     2831ops/s 371.4mb/s      0.1ms/op [0.00ms - 53.16ms]
  openfile2            169943ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms - 20.11ms]
  closefile1           169943ops     2831ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.61ms]
  wrtfile1             169949ops     2831ops/s 352.3mb/s      1.4ms/op [0.00ms - 94.59ms]
  createfile1          169960ops     2831ops/s   0.0mb/s      3.1ms/op [0.01ms - 76.81ms]
  62.155: IO Summary: 2209220 ops 36804.181 ops/s 2831/5662 rd/wr 759.8mb/s   5.8ms/op

  mkdir1               10000ops     9994ops/s   0.0mb/s      0.3ms/op [0.01ms - 52.11ms]
  2.016: IO Summary: 10000 ops 9994.103 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
  
  rmdir1               10000ops     9968ops/s   0.0mb/s      0.5ms/op [0.01ms - 61.12ms]
  2.267: IO Summary: 10000 ops 9967.814 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op  
  ```
  - boesfs
  ```bash
  deletefile1          169623ops     2826ops/s   0.0mb/s      4.7ms/op [0.01ms - 87.83ms]
  listdir1             169646ops     2826ops/s  14.1mb/s      0.4ms/op [0.01ms - 42.68ms]
  statfile1            169648ops     2826ops/s   0.0mb/s      0.0ms/op [0.00ms - 15.76ms]
  closefile2           169648ops     2826ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.33ms]
  fsyncfile1           169655ops     2827ops/s   0.0mb/s      7.2ms/op [0.00ms - 186.53ms]
  appendfilerand1      169664ops     2827ops/s  22.1mb/s      0.2ms/op [0.01ms - 78.14ms]
  openfile1            169666ops     2827ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.97ms]
  closefile3           169666ops     2827ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.78ms]
  readfile1            169666ops     2827ops/s 371.3mb/s      0.2ms/op [0.01ms - 74.93ms]
  openfile2            169666ops     2827ops/s   0.0mb/s      0.0ms/op [0.01ms - 10.21ms]
  closefile1           169666ops     2827ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.50ms]
  wrtfile1             169666ops     2827ops/s 350.8mb/s      1.5ms/op [0.01ms - 78.14ms]
  createfile1          169672ops     2827ops/s   0.0mb/s      3.3ms/op [0.02ms - 83.15ms]
  62.368: IO Summary: 2205552 ops 36746.063 ops/s 2827/5653 rd/wr 758.2mb/s   5.8ms/op

  mkdir1               10000ops     9998ops/s   0.0mb/s      2.1ms/op [0.01ms - 105.38ms]
  2.021: IO Summary: 10000 ops 9997.601 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
  
  rmdir1               10000ops     9992ops/s   0.0mb/s      0.6ms/op [0.01ms - 66.00ms]
  2.594: IO Summary: 10000 ops 9992.236 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
  ```
- 消耗
  - 测试前
    - native
    ```bash
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        279M        2.4G        6.2M        1.2G        3.3G
    Swap:          2.0G          0B        2.0G
    Filesystem      Size  Used Avail Use% Mounted on
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           394M  6.2M  388M   2% /run
    /dev/sda1        47G   24G   21G  54% /
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    tmpfs           394M     0  394M   0% /run/user/1000
    ```
    - boesfs
    ```bash
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        285M        2.4G        6.2M        1.2G        3.3G
    Swap:          2.0G          0B        2.0G
    Filesystem      Size  Used Avail Use% Mounted on
    /dev/sda1        47G   24G   21G  54% /
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           394M  6.2M  388M   2% /run
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           394M     0  394M   0% /run/user/1000
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    /home/boes       47G   24G   21G  54% /home/boes
    ```
  - 测试中
    - native
    ```bash
    boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ free -h
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        778M        976M        6.2M        2.1G        2.8G
    Swap:          2.0G          0B        2.0G
    boes@debian:~/Final/boesfs/Code/test/performance_test/workloads$ df -h
    Filesystem      Size  Used Avail Use% Mounted on
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           394M  6.2M  388M   2% /run
    /dev/sda1        47G   25G   20G  57% /
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    tmpfs           394M     0  394M   0% /run/user/1000
    ```
    - boesfs
    ```bash
    boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        792M        801M        6.2M        2.3G        2.8G
    Swap:          2.0G          0B        2.0G
    boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
    Filesystem      Size  Used Avail Use% Mounted on
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           394M  6.2M  388M   2% /run
    /dev/sda1        47G   26G   20G  57% /
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    tmpfs           394M     0  394M   0% /run/user/1000
    ```
  - 测试后
    - native
    ```
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        280M        2.4G        6.2M        1.2G        3.3G
    Swap:          2.0G          0B        2.0G
    Filesystem      Size  Used Avail Use% Mounted on
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           394M  6.2M  388M   2% /run
    /dev/sda1        47G   24G   21G  54% /
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    tmpfs           394M     0  394M   0% /run/user/1000
    ```
    - boesfs
    ```bash
                  total        used        free      shared  buff/cache   available
    Mem:           3.8G        289M        2.4G        6.2M        1.2G        3.3G
    Swap:          2.0G          0B        2.0G
    Filesystem      Size  Used Avail Use% Mounted on
    /dev/sda1        47G   24G   21G  54% /
    udev            1.9G     0  1.9G   0% /dev
    tmpfs           2.0G     0  2.0G   0% /dev/shm
    tmpfs           394M  6.2M  388M   2% /run
    tmpfs           5.0M     0  5.0M   0% /run/lock
    tmpfs           394M     0  394M   0% /run/user/1000
    tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
    /home/boes       47G   24G   21G  54% /home/boes
    ```

## 组3


  - 性能
    - native
    ```bash
    deletefile1          179923ops     2997ops/s   0.0mb/s      4.3ms/op [0.01ms - 232.16ms]
    listdir1             179923ops     2997ops/s  14.9mb/s      0.4ms/op [0.00ms - 226.90ms]
    statfile1            179923ops     2997ops/s   0.0mb/s      0.0ms/op [0.00ms - 17.63ms]
    closefile2           179923ops     2997ops/s   0.0mb/s      0.0ms/op [0.00ms - 19.98ms]
    fsyncfile1           179923ops     2997ops/s   0.0mb/s      7.2ms/op [0.00ms - 259.14ms]
    appendfilerand1      179973ops     2998ops/s  23.5mb/s      0.1ms/op [0.00ms - 50.47ms]
    openfile1            179973ops     2998ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.23ms]
    closefile3           179973ops     2998ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.66ms]
    readfile1            179973ops     2998ops/s 394.5mb/s      0.1ms/op [0.00ms - 40.19ms]
    openfile2            179973ops     2998ops/s   0.0mb/s      0.0ms/op [0.00ms - 35.78ms]
    closefile1           179973ops     2998ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.74ms]
    wrtfile1             179973ops     2998ops/s 370.7mb/s      1.3ms/op [0.00ms - 224.18ms]
    createfile1          179973ops     2998ops/s   0.0mb/s      2.9ms/op [0.01ms - 220.20ms]
    62.231: IO Summary: 2339399 ops 38967.280 ops/s 2998/5996 rd/wr 803.6mb/s   5.5ms/op

    mkdir1               10000ops     9998ops/s   0.0mb/s      0.3ms/op [0.01ms - 35.87ms]
    2.020: IO Summary: 10000 ops 9997.651 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
        
    rmdir1               10000ops     9995ops/s   0.0mb/s      0.5ms/op [0.01ms - 40.57ms]
    2.262: IO Summary: 10000 ops 9995.042 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          159691ops     2660ops/s   0.0mb/s      5.0ms/op [0.01ms - 110.28ms]
    listdir1             159705ops     2660ops/s  13.2mb/s      0.4ms/op [0.01ms - 39.74ms]
    statfile1            159708ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms -  9.28ms]
    closefile2           159708ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms - 11.84ms]
    fsyncfile1           159712ops     2661ops/s   0.0mb/s      7.6ms/op [0.00ms - 215.08ms]
    appendfilerand1      159716ops     2661ops/s  20.8mb/s      0.2ms/op [0.01ms - 33.76ms]
    openfile1            159718ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.46ms]
    closefile3           159718ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.04ms]
    readfile1            159719ops     2661ops/s 349.7mb/s      0.2ms/op [0.01ms - 44.91ms]
    openfile2            159720ops     2661ops/s   0.0mb/s      0.0ms/op [0.01ms -  9.19ms]
    closefile1           159720ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.87ms]
    wrtfile1             159727ops     2661ops/s 329.3mb/s      1.6ms/op [0.01ms - 52.62ms]
    createfile1          159733ops     2661ops/s   0.0mb/s      3.5ms/op [0.02ms - 62.07ms]
    62.398: IO Summary: 2076295 ops 34588.345 ops/s 2661/5322 rd/wr 713.0mb/s   6.2ms/op

    mkdir1               10000ops     9984ops/s   0.0mb/s      2.1ms/op [0.01ms - 102.58ms]
    2.023: IO Summary: 10000 ops 9983.697 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9992ops/s   0.0mb/s      0.7ms/op [0.01ms - 75.31ms]
    2.562: IO Summary: 10000 ops 9991.517 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        285M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        288M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        790M        803M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        797M        777M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        288M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        293M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```


## 组4

  - 性能
    - native
    ```bash
    deletefile1          173864ops     2896ops/s   0.0mb/s      4.5ms/op [0.01ms - 191.63ms]
    listdir1             173880ops     2896ops/s  14.4mb/s      0.4ms/op [0.00ms - 76.03ms]
    statfile1            173880ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.15ms]
    closefile2           173881ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms - 28.91ms]
    fsyncfile1           173889ops     2896ops/s   0.0mb/s      7.2ms/op [0.00ms - 183.81ms]
    appendfilerand1      173902ops     2896ops/s  22.6mb/s      0.1ms/op [0.00ms - 63.37ms]
    openfile1            173903ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.36ms]
    closefile3           173903ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.72ms]
    readfile1            173903ops     2896ops/s 380.9mb/s      0.1ms/op [0.00ms - 59.44ms]
    openfile2            173903ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.89ms]
    closefile1           173903ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.45ms]
    wrtfile1             173903ops     2896ops/s 358.8mb/s      1.5ms/op [0.00ms - 191.51ms]
    createfile1          173907ops     2897ops/s   0.0mb/s      3.1ms/op [0.01ms - 92.55ms]
    62.255: IO Summary: 2260621 ops 37652.250 ops/s 2896/5793 rd/wr 776.7mb/s   5.7ms/op

    mkdir1               10000ops     9990ops/s   0.0mb/s      0.4ms/op [0.01ms - 51.44ms]
    2.018: IO Summary: 10000 ops 9990.110 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9982ops/s   0.0mb/s      0.4ms/op [0.01ms - 56.08ms]
    2.273: IO Summary: 10000 ops 9981.654 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          164414ops     2739ops/s   0.0mb/s      4.8ms/op [0.01ms - 112.75ms]
    listdir1             164417ops     2739ops/s  13.6mb/s      0.4ms/op [0.01ms - 77.98ms]
    statfile1            164417ops     2739ops/s   0.0mb/s      0.0ms/op [0.00ms - 17.75ms]
    closefile2           164417ops     2739ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.71ms]
    fsyncfile1           164433ops     2739ops/s   0.0mb/s      7.4ms/op [0.00ms - 165.84ms]
    appendfilerand1      164467ops     2740ops/s  21.4mb/s      0.2ms/op [0.01ms - 93.81ms]
    openfile1            164467ops     2740ops/s   0.0mb/s      0.0ms/op [0.00ms - 21.85ms]
    closefile3           164467ops     2740ops/s   0.0mb/s      0.0ms/op [0.00ms - 12.22ms]
    readfile1            164467ops     2740ops/s 359.5mb/s      0.2ms/op [0.01ms - 19.85ms]
    openfile2            164467ops     2740ops/s   0.0mb/s      0.0ms/op [0.01ms - 20.88ms]
    closefile1           164467ops     2740ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.41ms]
    wrtfile1             164467ops     2740ops/s 340.8mb/s      1.6ms/op [0.01ms - 110.61ms]
    createfile1          164467ops     2740ops/s   0.0mb/s      3.4ms/op [0.02ms - 101.74ms]
    62.428: IO Summary: 2137833 ops 35616.033 ops/s 2740/5480 rd/wr 735.3mb/s   6.0ms/op

    mkdir1               10000ops     9977ops/s   0.0mb/s      1.9ms/op [0.01ms - 126.95ms]
    2.039: IO Summary: 10000 ops 9977.381 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9993ops/s   0.0mb/s      0.5ms/op [0.01ms - 68.04ms]
    2.478: IO Summary: 10000 ops 9992.995 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```
      total        used        free      shared  buff/cache   available
      Mem:           3.8G        292M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        295M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        798M        817M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        803M        779M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        294M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        301M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   24G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   24G   21G  54% /home/boes
      ```
## 组5

  - 性能
    - native
    ```bash
    deletefile1          173499ops     2890ops/s   0.0mb/s      4.5ms/op [0.01ms - 115.29ms]
    listdir1             173521ops     2890ops/s  14.4mb/s      0.4ms/op [0.00ms - 109.98ms]
    statfile1            173522ops     2890ops/s   0.0mb/s      0.0ms/op [0.00ms - 12.50ms]
    closefile2           173523ops     2890ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.78ms]
    fsyncfile1           173529ops     2890ops/s   0.0mb/s      7.4ms/op [0.00ms - 188.94ms]
    appendfilerand1      173537ops     2891ops/s  22.6mb/s      0.1ms/op [0.00ms - 41.15ms]
    openfile1            173537ops     2891ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.91ms]
    closefile3           173537ops     2891ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.58ms]
    readfile1            173538ops     2891ops/s 378.7mb/s      0.1ms/op [0.00ms - 28.81ms]
    openfile2            173538ops     2891ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.61ms]
    closefile1           173538ops     2891ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.76ms]
    wrtfile1             173538ops     2891ops/s 358.6mb/s      1.5ms/op [0.00ms - 120.05ms]
    createfile1          173538ops     2891ops/s   0.0mb/s      3.1ms/op [0.01ms - 96.71ms]
    62.344: IO Summary: 2255895 ops 37576.711 ops/s 2891/5781 rd/wr 774.2mb/s   5.7ms/op

    mkdir1               10000ops     9982ops/s   0.0mb/s      0.7ms/op [0.01ms - 98.83ms]
    2.020: IO Summary: 10000 ops 9981.823 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9990ops/s   0.0mb/s      0.5ms/op [0.01ms - 45.02ms]
    2.299: IO Summary: 10000 ops 9989.561 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          165152ops     2751ops/s   0.0mb/s      4.8ms/op [0.02ms - 116.28ms]
    listdir1             165158ops     2751ops/s  13.7mb/s      0.4ms/op [0.01ms - 40.25ms]
    statfile1            165158ops     2751ops/s   0.0mb/s      0.0ms/op [0.00ms - 11.37ms]
    closefile2           165158ops     2751ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.97ms]
    fsyncfile1           165158ops     2751ops/s   0.0mb/s      7.3ms/op [0.00ms - 196.15ms]
    appendfilerand1      165207ops     2752ops/s  21.5mb/s      0.2ms/op [0.01ms - 37.28ms]
    openfile1            165207ops     2752ops/s   0.0mb/s      0.0ms/op [0.00ms - 16.38ms]
    closefile3           165207ops     2752ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.12ms]
    readfile1            165207ops     2752ops/s 360.0mb/s      0.2ms/op [0.01ms - 86.06ms]
    openfile2            165207ops     2752ops/s   0.0mb/s      0.0ms/op [0.01ms - 13.50ms]
    closefile1           165207ops     2752ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.26ms]
    wrtfile1             165207ops     2752ops/s 342.5mb/s      1.6ms/op [0.01ms - 79.62ms]
    createfile1          165207ops     2752ops/s   0.0mb/s      3.4ms/op [0.02ms - 76.77ms]
    62.748: IO Summary: 2147440 ops 35775.608 ops/s 2752/5505 rd/wr 737.7mb/s   6.0ms/op
    
    mkdir1               10000ops     9981ops/s   0.0mb/s      0.7ms/op [0.01ms - 76.66ms]
    2.021: IO Summary: 10000 ops 9980.737 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9987ops/s   0.0mb/s      0.7ms/op [0.01ms - 70.75ms]
    2.450: IO Summary: 10000 ops 9987.196 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        295M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        300M        1.3G        6.2M        2.3G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   26G   20G  57% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   26G   20G  57% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        530M        1.1G        6.2M        2.3G        3.0G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        802M        825M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        297M        2.4G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   24G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        303M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```


## 组6

  - 性能
    - native
    ```bash
    deletefile1          175255ops     2920ops/s   0.0mb/s      4.4ms/op [0.01ms - 137.76ms]
    listdir1             175285ops     2920ops/s  14.5mb/s      0.4ms/op [0.00ms - 105.65ms]
    statfile1            175285ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.68ms]
    closefile2           175285ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.14ms]
    fsyncfile1           175292ops     2920ops/s   0.0mb/s      7.3ms/op [0.00ms - 202.93ms]
    appendfilerand1      175297ops     2920ops/s  22.8mb/s      0.1ms/op [0.00ms - 68.24ms]
    openfile1            175297ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.36ms]
    closefile3           175297ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.79ms]
    readfile1            175297ops     2920ops/s 384.0mb/s      0.1ms/op [0.00ms - 67.57ms]
    openfile2            175297ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.06ms]
    closefile1           175297ops     2920ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.34ms]
    wrtfile1             175300ops     2920ops/s 362.1mb/s      1.4ms/op [0.00ms - 158.73ms]
    createfile1          175303ops     2921ops/s   0.0mb/s      3.0ms/op [0.01ms - 169.89ms]
    62.235: IO Summary: 2278787 ops 37964.022 ops/s 2920/5841 rd/wr 783.4mb/s   5.6ms/op

    mkdir1               10000ops     9998ops/s   0.0mb/s      0.4ms/op [0.01ms - 50.90ms]
    2.015: IO Summary: 10000 ops 9997.721 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9987ops/s   0.0mb/s      0.4ms/op [0.01ms - 49.20ms]
    2.264: IO Summary: 10000 ops 9986.578 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          168378ops     2805ops/s   0.0mb/s      4.6ms/op [0.01ms - 110.49ms]
    listdir1             168391ops     2805ops/s  13.9mb/s      0.4ms/op [0.01ms - 43.11ms]
    statfile1            168392ops     2805ops/s   0.0mb/s      0.0ms/op [0.00ms - 12.02ms]
    closefile2           168392ops     2805ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.34ms]
    fsyncfile1           168394ops     2805ops/s   0.0mb/s      7.4ms/op [0.00ms - 231.48ms]
    appendfilerand1      168398ops     2805ops/s  21.9mb/s      0.2ms/op [0.01ms - 61.69ms]
    openfile1            168400ops     2805ops/s   0.0mb/s      0.0ms/op [0.00ms - 17.69ms]
    closefile3           168400ops     2805ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.17ms]
    readfile1            168401ops     2805ops/s 367.3mb/s      0.1ms/op [0.01ms - 32.59ms]
    openfile2            168401ops     2805ops/s   0.0mb/s      0.0ms/op [0.01ms - 21.64ms]
    closefile1           168401ops     2805ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.49ms]
    wrtfile1             168407ops     2805ops/s 348.9mb/s      1.5ms/op [0.01ms - 71.50ms]
    createfile1          168415ops     2806ops/s   0.0mb/s      3.3ms/op [0.02ms - 76.31ms]
    62.637: IO Summary: 2189170 ops 36468.605 ops/s 2805/5611 rd/wr 752.1mb/s   5.9ms/op

    mkdir1               10000ops     9997ops/s   0.0mb/s      2.3ms/op [0.01ms - 91.27ms]
    2.026: IO Summary: 10000 ops 9997.161 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9985ops/s   0.0mb/s      0.7ms/op [0.01ms - 65.06ms]
    2.600: IO Summary: 10000 ops 9984.803 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```
            total        used        free      shared  buff/cache   available
      Mem:           3.8G        297M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code$ ./ft1.sh 
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        303M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        803M        729M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        811M        748M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        299M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        305M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```

## 组7

  - 性能
    - native
    ```bash
    deletefile1          172038ops     2866ops/s   0.0mb/s      4.5ms/op [0.01ms - 102.14ms]
    listdir1             172055ops     2866ops/s  14.3mb/s      0.4ms/op [0.00ms - 46.62ms]
    statfile1            172055ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.75ms]
    closefile2           172055ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.71ms]
    fsyncfile1           172060ops     2866ops/s   0.0mb/s      7.5ms/op [0.00ms - 225.86ms]
    appendfilerand1      172064ops     2866ops/s  22.4mb/s      0.1ms/op [0.00ms - 36.26ms]
    openfile1            172064ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.49ms]
    closefile3           172064ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.59ms]
    readfile1            172064ops     2866ops/s 376.9mb/s      0.1ms/op [0.00ms - 38.82ms]
    openfile2            172064ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms - 18.63ms]
    closefile1           172064ops     2866ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.74ms]
    wrtfile1             172070ops     2866ops/s 354.8mb/s      1.4ms/op [0.00ms - 139.41ms]
    createfile1          172082ops     2867ops/s   0.0mb/s      3.1ms/op [0.01ms - 83.27ms]
    62.196: IO Summary: 2236799 ops 37260.864 ops/s 2866/5733 rd/wr 768.3mb/s   5.7ms/op

    mkdir1               10000ops     9991ops/s   0.0mb/s      0.4ms/op [0.01ms - 77.07ms]
    2.019: IO Summary: 10000 ops 9990.739 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9991ops/s   0.0mb/s      0.5ms/op [0.01ms - 48.93ms]
    2.273: IO Summary: 10000 ops 9991.068 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          165781ops     2762ops/s   0.0mb/s      4.8ms/op [0.01ms - 94.88ms]
    listdir1             165791ops     2762ops/s  13.7mb/s      0.4ms/op [0.01ms - 67.77ms]
    statfile1            165791ops     2762ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.13ms]
    closefile2           165791ops     2762ops/s   0.0mb/s      0.0ms/op [0.00ms -  9.18ms]
    fsyncfile1           165796ops     2762ops/s   0.0mb/s      7.3ms/op [0.00ms - 211.23ms]
    appendfilerand1      165801ops     2762ops/s  21.6mb/s      0.2ms/op [0.00ms - 62.11ms]
    openfile1            165801ops     2762ops/s   0.0mb/s      0.0ms/op [0.00ms - 15.56ms]
    closefile3           165801ops     2762ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.59ms]
    readfile1            165801ops     2762ops/s 361.9mb/s      0.2ms/op [0.01ms - 25.93ms]
    openfile2            165801ops     2762ops/s   0.0mb/s      0.0ms/op [0.01ms - 53.21ms]
    closefile1           165801ops     2762ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.74ms]
    wrtfile1             165807ops     2762ops/s 343.6mb/s      1.6ms/op [0.01ms - 81.82ms]
    createfile1          165819ops     2763ops/s   0.0mb/s      3.4ms/op [0.02ms - 69.12ms]
    62.532: IO Summary: 2155382 ops 35908.549 ops/s 2762/5525 rd/wr 740.9mb/s   6.0ms/op

    mkdir1               10000ops     9988ops/s   0.0mb/s      1.8ms/op [0.01ms - 87.92ms]
    2.033: IO Summary: 10000 ops 9988.224 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9987ops/s   0.0mb/s      0.8ms/op [0.01ms - 65.67ms]
    2.477: IO Summary: 10000 ops 9986.867 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        300M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        303M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
              total        used        free      shared  buff/cache   available
      Mem:           3.8G        588M        1.0G        6.2M        2.3G        3.0G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        810M        732M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                          total        used        free      shared  buff/cache   available
      Mem:           3.8G        301M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        307M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```

## 组8

  - 性能
    - native
    ```bash
    deletefile1          176079ops     2934ops/s   0.0mb/s      4.4ms/op [0.01ms - 124.81ms]
    listdir1             176081ops     2934ops/s  14.6mb/s      0.4ms/op [0.00ms - 89.99ms]
    statfile1            176081ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.17ms]
    closefile2           176081ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.74ms]
    fsyncfile1           176112ops     2934ops/s   0.0mb/s      7.3ms/op [0.00ms - 271.57ms]
    appendfilerand1      176131ops     2934ops/s  22.9mb/s      0.1ms/op [0.00ms - 96.05ms]
    openfile1            176131ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms -  9.69ms]
    closefile3           176131ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.01ms]
    readfile1            176131ops     2934ops/s 383.9mb/s      0.1ms/op [0.00ms - 91.72ms]
    openfile2            176131ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms - 16.20ms]
    closefile1           176131ops     2934ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.67ms]
    wrtfile1             176131ops     2934ops/s 365.5mb/s      1.4ms/op [0.00ms - 216.00ms]
    createfile1          176131ops     2934ops/s   0.0mb/s      3.0ms/op [0.01ms - 210.57ms]
    62.201: IO Summary: 2289482 ops 38144.010 ops/s 2934/5869 rd/wr 786.9mb/s   5.6ms/op

    mkdir1               10000ops     9997ops/s   0.0mb/s      0.6ms/op [0.01ms - 60.18ms]
    2.016: IO Summary: 10000 ops 9997.081 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9973ops/s   0.0mb/s      0.4ms/op [0.01ms - 51.78ms]
    2.282: IO Summary: 10000 ops 9973.461 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          166294ops     2771ops/s   0.0mb/s      4.7ms/op [0.01ms - 132.90ms]
    listdir1             166314ops     2771ops/s  13.8mb/s      0.4ms/op [0.01ms - 38.22ms]
    statfile1            166318ops     2771ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.61ms]
    closefile2           166318ops     2771ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.16ms]
    fsyncfile1           166321ops     2771ops/s   0.0mb/s      7.4ms/op [0.00ms - 278.79ms]
    appendfilerand1      166324ops     2771ops/s  21.7mb/s      0.2ms/op [0.00ms - 30.79ms]
    openfile1            166324ops     2771ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.52ms]
    closefile3           166324ops     2771ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.66ms]
    readfile1            166324ops     2771ops/s 363.5mb/s      0.1ms/op [0.01ms - 38.62ms]
    openfile2            166324ops     2771ops/s   0.0mb/s      0.0ms/op [0.01ms - 12.68ms]
    closefile1           166324ops     2771ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.84ms]
    wrtfile1             166328ops     2771ops/s 344.6mb/s      1.5ms/op [0.01ms - 92.94ms]
    createfile1          166333ops     2771ops/s   0.0mb/s      3.4ms/op [0.02ms - 62.82ms]
    62.644: IO Summary: 2162170 ops 36024.151 ops/s 2771/5542 rd/wr 743.6mb/s   5.9ms/op

    mkdir1               10000ops     9980ops/s   0.0mb/s      2.0ms/op [0.01ms - 106.35ms]
    2.024: IO Summary: 10000 ops 9980.189 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    
    rmdir1               10000ops     9994ops/s   0.0mb/s      0.6ms/op [0.01ms - 47.88ms]
    2.465: IO Summary: 10000 ops 9994.303 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```
                          total        used        free      shared  buff/cache   available
      Mem:           3.8G        300M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code$ ./ft1.sh 
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        307M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        808M        776M        6.2M        2.3G        2.8G
      Swap:          2.0G          0B        2.0G
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   26G   20G  57% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        303M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           394M  6.2M  388M   2% /run
      /dev/sda1        47G   25G   21G  54% /
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                    total        used        free      shared  buff/cache   available
      Mem:           3.8G        310M        2.3G        6.2M        1.2G        3.3G
      Swap:          2.0G          0B        2.0G
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sda1        47G   25G   21G  54% /
      udev            1.9G     0  1.9G   0% /dev
      tmpfs           2.0G     0  2.0G   0% /dev/shm
      tmpfs           394M  6.2M  388M   2% /run
      tmpfs           5.0M     0  5.0M   0% /run/lock
      tmpfs           394M     0  394M   0% /run/user/1000
      tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
      /home/boes       47G   25G   21G  54% /home/boes
      ```

## 组9

  - 性能
    - native
    ```bash
    deletefile1          165737ops     2761ops/s   0.0mb/s      4.7ms/op [0.01ms - 151.76ms]
    listdir1             165743ops     2761ops/s  13.7mb/s      0.4ms/op [0.00ms - 140.91ms]
    statfile1            165747ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.57ms]
    closefile2           165747ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.84ms]
    fsyncfile1           165755ops     2761ops/s   0.0mb/s      7.9ms/op [0.00ms - 364.28ms]
    appendfilerand1      165761ops     2761ops/s  21.6mb/s      0.1ms/op [0.00ms - 55.63ms]
    openfile1            165761ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.93ms]
    closefile3           165761ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms -  2.92ms]
    readfile1            165761ops     2761ops/s 361.4mb/s      0.1ms/op [0.00ms - 30.54ms]
    openfile2            165761ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms - 26.20ms]
    closefile1           165761ops     2761ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.21ms]
    wrtfile1             165765ops     2762ops/s 343.8mb/s      1.5ms/op [0.00ms - 165.92ms]
    createfile1          165779ops     2762ops/s   0.0mb/s      3.1ms/op [0.01ms - 155.24ms]
    62.234: IO Summary: 2154839 ops 35898.586 ops/s 2761/5523 rd/wr 740.4mb/s   6.0ms/op

    mkdir1               10000ops     9975ops/s   0.0mb/s      0.6ms/op [0.01ms - 83.78ms]
    2.018: IO Summary: 10000 ops 9975.311 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

    rmdir1               10000ops     9976ops/s   0.0mb/s      0.6ms/op [0.01ms - 66.48ms]
    2.308: IO Summary: 10000 ops 9976.466 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          160085ops     2667ops/s   0.0mb/s      5.0ms/op [0.01ms - 90.90ms]
    listdir1             160090ops     2667ops/s  13.3mb/s      0.4ms/op [0.01ms - 54.86ms]
    statfile1            160090ops     2667ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.95ms]
    closefile2           160090ops     2667ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.59ms]
    fsyncfile1           160090ops     2667ops/s   0.0mb/s      7.6ms/op [0.00ms - 221.73ms]
    appendfilerand1      160140ops     2668ops/s  20.8mb/s      0.2ms/op [0.01ms - 32.77ms]
    openfile1            160140ops     2668ops/s   0.0mb/s      0.0ms/op [0.00ms - 20.04ms]
    closefile3           160140ops     2668ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.15ms]
    readfile1            160140ops     2668ops/s 350.0mb/s      0.2ms/op [0.01ms - 29.49ms]
    openfile2            160140ops     2668ops/s   0.0mb/s      0.0ms/op [0.01ms - 19.63ms]
    closefile1           160140ops     2668ops/s   0.0mb/s      0.0ms/op [0.00ms - 11.04ms]
    wrtfile1             160140ops     2668ops/s 330.9mb/s      1.6ms/op [0.01ms - 90.76ms]
    createfile1          160140ops     2668ops/s   0.0mb/s      3.5ms/op [0.02ms - 98.58ms]
    62.482: IO Summary: 2081565 ops 34675.281 ops/s 2668/5335 rd/wr 714.9mb/s   6.2ms/op

        mkdir1               10000ops     9997ops/s   0.0mb/s      2.2ms/op [0.01ms - 139.69ms]
    2.023: IO Summary: 10000 ops 9997.201 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

        rmdir1               10000ops     9981ops/s   0.0mb/s      0.7ms/op [0.01ms - 65.63ms]
    2.546: IO Summary: 10000 ops 9981.385 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```bash
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        304M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        308M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
              total        used        free      shared  buff/cache   available
        Mem:           3.8G        814M        750M        6.2M        2.3G        2.8G
        Swap:          2.0G          0B        2.0G
        boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```bash
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        305M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      2.546: Shutting down processes
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        311M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```

## 组10

  - 性能
    - native
    ```bash
    deletefile1          173882ops     2896ops/s   0.0mb/s      4.5ms/op [0.01ms - 99.84ms]
    listdir1             173886ops     2896ops/s  14.4mb/s      0.4ms/op [0.00ms - 48.68ms]
    statfile1            173886ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.37ms]
    closefile2           173886ops     2896ops/s   0.0mb/s      0.0ms/op [0.00ms -  6.65ms]
    fsyncfile1           173886ops     2896ops/s   0.0mb/s      7.5ms/op [0.00ms - 256.55ms]
    appendfilerand1      173936ops     2897ops/s  22.6mb/s      0.1ms/op [0.00ms - 33.22ms]
    openfile1            173936ops     2897ops/s   0.0mb/s      0.0ms/op [0.00ms - 13.95ms]
    closefile3           173936ops     2897ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.83ms]
    readfile1            173936ops     2897ops/s 380.1mb/s      0.1ms/op [0.00ms - 22.45ms]
    openfile2            173936ops     2897ops/s   0.0mb/s      0.0ms/op [0.00ms - 10.20ms]
    closefile1           173936ops     2897ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.30ms]
    wrtfile1             173936ops     2897ops/s 359.6mb/s      1.4ms/op [0.00ms - 99.21ms]
    createfile1          173936ops     2897ops/s   0.0mb/s      3.0ms/op [0.01ms - 101.25ms]
    62.226: IO Summary: 2260914 ops 37652.448 ops/s 2897/5793 rd/wr 776.7mb/s   5.7ms/op

        mkdir1               10000ops     9978ops/s   0.0mb/s      0.4ms/op [0.01ms - 54.04ms]
    2.017: IO Summary: 10000 ops 9978.397 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

        rmdir1               10000ops     9985ops/s   0.0mb/s      0.6ms/op [0.01ms - 47.80ms]
    2.340: IO Summary: 10000 ops 9984.703 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          159755ops     2661ops/s   0.0mb/s      4.9ms/op [0.01ms - 130.42ms]
    listdir1             159758ops     2661ops/s  13.2mb/s      0.4ms/op [0.01ms - 56.77ms]
    statfile1            159759ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.78ms]
    closefile2           159760ops     2661ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.61ms]
    fsyncfile1           159760ops     2661ops/s   0.0mb/s      7.6ms/op [0.00ms - 221.36ms]
    appendfilerand1      159803ops     2662ops/s  20.8mb/s      0.2ms/op [0.01ms - 37.56ms]
    openfile1            159803ops     2662ops/s   0.0mb/s      0.0ms/op [0.00ms - 17.45ms]
    closefile3           159803ops     2662ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.66ms]
    readfile1            159803ops     2662ops/s 350.0mb/s      0.2ms/op [0.01ms - 26.70ms]
    openfile2            159803ops     2662ops/s   0.0mb/s      0.0ms/op [0.01ms - 20.03ms]
    closefile1           159803ops     2662ops/s   0.0mb/s      0.0ms/op [0.00ms -  4.76ms]
    wrtfile1             159803ops     2662ops/s 331.1mb/s      1.6ms/op [0.01ms - 106.71ms]
    createfile1          159803ops     2662ops/s   0.0mb/s      3.5ms/op [0.02ms - 71.41ms]
    62.405: IO Summary: 2077216 ops 34599.876 ops/s 2662/5324 rd/wr 715.1mb/s   6.2ms/op

        mkdir1               10000ops     9994ops/s   0.0mb/s      2.4ms/op [0.01ms - 102.56ms]
    2.023: IO Summary: 10000 ops 9993.614 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

        rmdir1               10000ops     9966ops/s   0.0mb/s      0.8ms/op [0.01ms - 84.74ms]
    2.483: IO Summary: 10000 ops 9965.807 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        305M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
        boes@debian:~/Final/boesfs/Code$ ./ft1.sh 
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        311M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
            boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        812M        718M        6.2M        2.3G        2.8G
        Swap:          2.0G          0B        2.0G
        boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        816M        707M        6.2M        2.4G        2.8G
        Swap:          2.0G          0B        2.0G
        boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        308M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        314M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```



## 组 11

  - 性能
    - native
    ```bash
    deletefile1          168782ops     2812ops/s   0.0mb/s      4.6ms/op [0.01ms - 106.94ms]
    listdir1             168787ops     2812ops/s  14.0mb/s      0.4ms/op [0.00ms - 64.08ms]
    statfile1            168787ops     2812ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.67ms]
    closefile2           168787ops     2812ops/s   0.0mb/s      0.0ms/op [0.00ms -  7.49ms]
    fsyncfile1           168787ops     2812ops/s   0.0mb/s      7.5ms/op [0.00ms - 190.54ms]
    appendfilerand1      168837ops     2813ops/s  22.0mb/s      0.2ms/op [0.00ms - 34.68ms]
    openfile1            168837ops     2813ops/s   0.0mb/s      0.0ms/op [0.00ms - 14.88ms]
    closefile3           168837ops     2813ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.31ms]
    readfile1            168837ops     2813ops/s 368.5mb/s      0.1ms/op [0.00ms - 26.70ms]
    openfile2            168837ops     2813ops/s   0.0mb/s      0.0ms/op [0.00ms - 12.08ms]
    closefile1           168837ops     2813ops/s   0.0mb/s      0.0ms/op [0.00ms -  3.84ms]
    wrtfile1             168837ops     2813ops/s 349.8mb/s      1.5ms/op [0.00ms - 84.41ms]
    createfile1          168837ops     2813ops/s   0.0mb/s      3.2ms/op [0.01ms - 68.51ms]
    62.257: IO Summary: 2194626 ops 36559.841 ops/s 2813/5625 rd/wr 754.2mb/s   5.8ms/op

        mkdir1               10000ops     9990ops/s   0.0mb/s      0.7ms/op [0.01ms - 67.21ms]
    2.020: IO Summary: 10000 ops 9989.601 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

        rmdir1               10000ops     9973ops/s   0.0mb/s      0.5ms/op [0.01ms - 55.26ms]
    2.283: IO Summary: 10000 ops 9973.093 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
    - boesfs
    ```bash
    deletefile1          152895ops     2547ops/s   0.0mb/s      5.2ms/op [0.01ms - 112.10ms]
    listdir1             152902ops     2547ops/s  12.7mb/s      0.4ms/op [0.01ms - 40.43ms]
    statfile1            152903ops     2547ops/s   0.0mb/s      0.0ms/op [0.00ms - 11.17ms]
    closefile2           152903ops     2547ops/s   0.0mb/s      0.0ms/op [0.00ms - 41.51ms]
    fsyncfile1           152912ops     2547ops/s   0.0mb/s      7.9ms/op [0.00ms - 223.79ms]
    appendfilerand1      152913ops     2547ops/s  19.9mb/s      0.2ms/op [0.00ms - 34.31ms]
    openfile1            152913ops     2547ops/s   0.0mb/s      0.0ms/op [0.00ms - 21.52ms]
    closefile3           152913ops     2547ops/s   0.0mb/s      0.0ms/op [0.00ms -  5.29ms]
    readfile1            152913ops     2547ops/s 335.4mb/s      0.2ms/op [0.01ms - 23.23ms]
    openfile2            152913ops     2547ops/s   0.0mb/s      0.0ms/op [0.01ms - 19.15ms]
    closefile1           152913ops     2547ops/s   0.0mb/s      0.0ms/op [0.00ms -  8.03ms]
    wrtfile1             152917ops     2547ops/s 315.4mb/s      1.7ms/op [0.01ms - 111.63ms]
    createfile1          152931ops     2547ops/s   0.0mb/s      3.7ms/op [0.02ms - 74.71ms]
    62.681: IO Summary: 1987841 ops 33109.379 ops/s 2547/5094 rd/wr 683.3mb/s   6.5ms/op

        mkdir1               10000ops     9976ops/s   0.0mb/s      2.1ms/op [0.01ms - 85.98ms]
    2.025: IO Summary: 10000 ops 9975.928 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op

        rmdir1               10000ops     9981ops/s   0.0mb/s      1.1ms/op [0.01ms - 79.43ms]
    2.530: IO Summary: 10000 ops 9981.285 ops/s 0/0 rd/wr   0.0mb/s   0.0ms/op
    ```
  - 消耗
    - 测试前
      - native
      ```bash
                total        used        free      shared  buff/cache   available
        Mem:           3.8G        308M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
        Filebench Version 1.5-alpha3
      ```
      - boesfs
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        313M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```
    - 测试中
      - native
      ```bash
      boes@debian:~/Final/boesfs/Code/test/performance_test$ free -h
                    total        used        free      shared  buff/cache   available
        Mem:           3.8G        814M        729M        6.2M        2.3G        2.8G
        Swap:          2.0G          0B        2.0G
        boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        818M        701M        6.2M        2.4G        2.8G
        Swap:          2.0G          0B        2.0G
        boes@debian:~/Final/boesfs/Code/test/performance_test$ df -h
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   26G   20G  57% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
    - 测试后
      - native
      ```bash
                   total        used        free      shared  buff/cache   available
        Mem:           3.8G        310M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           394M  6.2M  388M   2% /run
        /dev/sda1        47G   25G   21G  54% /
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        tmpfs           394M     0  394M   0% /run/user/1000
      ```
      - boesfs
      ```bash
                            total        used        free      shared  buff/cache   available
        Mem:           3.8G        317M        2.3G        6.2M        1.2G        3.3G
        Swap:          2.0G          0B        2.0G
        Filesystem      Size  Used Avail Use% Mounted on
        /dev/sda1        47G   25G   21G  54% /
        udev            1.9G     0  1.9G   0% /dev
        tmpfs           2.0G     0  2.0G   0% /dev/shm
        tmpfs           394M  6.2M  388M   2% /run
        tmpfs           5.0M     0  5.0M   0% /run/lock
        tmpfs           394M     0  394M   0% /run/user/1000
        tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
        /home/boes       47G   25G   21G  54% /home/boes
      ```


====================== 全部放在一个里面 ==============

  - native
  ```bash
  62.988: Per-Operation Breakdown
  rmdir1               619951ops    10324ops/s   0.0mb/s      0.6ms/op [0.02ms - 263.50ms]
  mkdir1               619961ops    10324ops/s   0.0mb/s      4.1ms/op [0.02ms - 281.63ms]
  deletefile1          56644ops      943ops/s   0.0mb/s     15.1ms/op [0.02ms - 152.61ms]
  listdir1             56665ops      944ops/s   4.5mb/s      1.3ms/op [0.01ms - 109.51ms]
  statfile1            56667ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 19.93ms]
  closefile2           56667ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 21.85ms]
  fsyncfile1           56668ops      944ops/s   0.0mb/s     20.1ms/op [0.01ms - 306.64ms]
  appendfilerand1      56675ops      944ops/s   7.4mb/s      1.3ms/op [0.01ms - 100.23ms]
  openfile1            56675ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 49.94ms]
  closefile3           56675ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 19.12ms]
  readfile1            56675ops      944ops/s 123.5mb/s      0.4ms/op [0.01ms - 71.39ms]
  openfile2            56675ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 27.95ms]
  closefile1           56675ops      944ops/s   0.0mb/s      0.0ms/op [0.01ms - 71.94ms]
  wrtfile1             56676ops      944ops/s 117.4mb/s      2.9ms/op [0.01ms - 129.31ms]
  createfile1          56686ops      944ops/s   0.0mb/s      9.9ms/op [0.02ms - 133.36ms]
  62.989: IO Summary: 1976635 ops 32916.776 ops/s 944/1888 rd/wr 252.8mb/s  34.2ms
  ```
  - boesfs
  ```bash
  rmdir1               443462ops     7382ops/s   0.0mb/s      0.9ms/op [0.02ms - 281.28ms]
  mkdir1               443484ops     7382ops/s   0.0mb/s      5.8ms/op [0.02ms - 281.89ms]
  deletefile1          55421ops      923ops/s   0.0mb/s     15.2ms/op [0.03ms - 142.72ms]
  listdir1             55424ops      923ops/s   4.4mb/s      1.3ms/op [0.02ms - 82.84ms]
  statfile1            55424ops      923ops/s   0.0mb/s      0.0ms/op [0.01ms - 14.67ms]
  closefile2           55424ops      923ops/s   0.0mb/s      0.0ms/op [0.01ms - 33.09ms]
  fsyncfile1           55424ops      923ops/s   0.0mb/s     19.7ms/op [0.01ms - 265.55ms]
  appendfilerand1      55473ops      923ops/s   7.2mb/s      1.4ms/op [0.01ms - 97.56ms]
  openfile1            55473ops      923ops/s   0.0mb/s      0.1ms/op [0.01ms - 42.96ms]
  closefile3           55473ops      923ops/s   0.0mb/s      0.0ms/op [0.01ms - 12.72ms]
  readfile1            55473ops      923ops/s 119.6mb/s      0.5ms/op [0.02ms - 76.97ms]
  openfile2            55473ops      923ops/s   0.0mb/s      0.1ms/op [0.01ms - 43.87ms]
  closefile1           55473ops      923ops/s   0.0mb/s      0.0ms/op [0.01ms - 34.33ms]
  wrtfile1             55473ops      923ops/s 114.8mb/s      3.2ms/op [0.02ms - 127.41ms]
  createfile1          55474ops      923ops/s   0.0mb/s     10.4ms/op [0.03ms - 140.35ms]
  62.557: IO Summary: 1607848 ops 26764.898 ops/s 923/1847 rd/wr 246.1mb/s  35.0ms/op
  ```

  - native
  ```bash
  rmdir1               543064ops     9041ops/s   0.0mb/s      0.7ms/op [0.01ms - 248.27ms]
  mkdir1               543081ops     9042ops/s   0.0mb/s      4.7ms/op [0.02ms - 265.69ms]
  deletefile1          55784ops      929ops/s   0.0mb/s     15.4ms/op [0.02ms - 184.03ms]
  listdir1             55797ops      929ops/s   4.4mb/s      1.3ms/op [0.01ms - 109.02ms]
  statfile1            55797ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 22.94ms]
  closefile2           55797ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 26.48ms]
  fsyncfile1           55800ops      929ops/s   0.0mb/s     19.7ms/op [0.01ms - 336.40ms]
  appendfilerand1      55808ops      929ops/s   7.3mb/s      1.4ms/op [0.01ms - 96.54ms]
  openfile1            55811ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 114.74ms]
  closefile3           55811ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 37.91ms]
  readfile1            55811ops      929ops/s 121.4mb/s      0.5ms/op [0.01ms - 67.52ms]
  openfile2            55811ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 147.74ms]
  closefile1           55811ops      929ops/s   0.0mb/s      0.0ms/op [0.01ms - 26.74ms]
  wrtfile1             55811ops      929ops/s 116.0mb/s      3.0ms/op [0.01ms - 140.67ms]
  createfile1          55819ops      929ops/s   0.0mb/s     10.1ms/op [0.02ms - 180.71ms]
  63.208: IO Summary: 1811613 ops 30161.060 ops/s 929/1858 rd/wr 249.1mb/s  34.6ms/op
  63.208: Shutting down processes
  ```
  - boesfs
  ```bash
  rmdir1               408256ops     6792ops/s   0.0mb/s      1.0ms/op [0.02ms - 214.16ms]
  mkdir1               408284ops     6792ops/s   0.0mb/s      6.2ms/op [0.02ms - 220.98ms]
  deletefile1          54551ops      908ops/s   0.0mb/s     15.6ms/op [0.03ms - 175.74ms]
  listdir1             54561ops      908ops/s   4.3mb/s      1.3ms/op [0.02ms - 91.53ms]
  statfile1            54562ops      908ops/s   0.0mb/s      0.0ms/op [0.01ms - 22.55ms]
  closefile2           54562ops      908ops/s   0.0mb/s      0.0ms/op [0.01ms - 21.73ms]
  fsyncfile1           54569ops      908ops/s   0.0mb/s     19.4ms/op [0.01ms - 327.02ms]
  appendfilerand1      54575ops      908ops/s   7.1mb/s      1.5ms/op [0.01ms - 100.13ms]
  openfile1            54577ops      908ops/s   0.0mb/s      0.1ms/op [0.02ms - 43.86ms]
  closefile3           54577ops      908ops/s   0.0mb/s      0.0ms/op [0.01ms - 27.31ms]
  readfile1            54577ops      908ops/s 118.3mb/s      0.5ms/op [0.02ms - 82.28ms]
  openfile2            54577ops      908ops/s   0.0mb/s      0.1ms/op [0.02ms - 37.84ms]
  closefile1           54577ops      908ops/s   0.0mb/s      0.0ms/op [0.01ms - 58.21ms]
  wrtfile1             54579ops      908ops/s 113.1mb/s      3.2ms/op [0.02ms - 105.15ms]
  createfile1          54590ops      908ops/s   0.0mb/s     10.8ms/op [0.03ms - 151.61ms]
  62.543: IO Summary: 1525974 ops 25386.986 ops/s 908/1816 rd/wr 242.8mb/s  35.5ms/op
  ```
