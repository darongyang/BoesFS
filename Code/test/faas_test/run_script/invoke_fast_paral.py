import threading
import sys
import os

args = sys.argv

delay = args[1]
func_id = args[2]
num_threads = int(args[3])


# 定义要执行的函数
def invoke_function(thread_name):
    print(f"Thread {thread_name} started")
    os.system("./invoke_fast.sh " + delay + " " + func_id)
    print(f"Thread {thread_name} completed")

# 创建线程
threads = []
for i in range(num_threads):
    thread = threading.Thread(target=invoke_function, args=(f"Thread {i}",))
    threads.append(thread)

# 启动线程
for thread in threads:
    thread.start()

# 等待线程执行完成
for thread in threads:
    thread.join()

print("所有线程完成")