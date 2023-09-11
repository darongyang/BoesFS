'''
    访问控制需求 V1.0
        - 某用户fs的fs_noaccess.txt文件对不可信应用隐藏, 不可见: /home/fs/fs_noaccess.txt。
        - 某用户fs的隐私文件fs_noopen.txt不允许不可信应用打开: /home/fs/fs_noopen.txt。
        - 某用户fs的隐私文件fs_noread.txt不允许不可信应用读取内容:/home/fs/fs_noread.txt。(但允许ls等来查看,请注意)
        - 某用户fs的隐私文件fs_nowrite.txt不允许不可信应用往文件写内容:/home/fs/fs_nowrite.txt。(但允许ls等来查看,允许读取,请注意)
        - 不允许不可信应用在当前目录以外的目录创建文件:非/home/fs/test/下。(内核未实现api)
        - 不允许不可信应用在当前目录以外的目录删除文件:非/home/fs/test/下。(内核未实现api)
        - 不允许不可信应用在当前目录以外的目录重命名文件:非/home/fs/test/下。(内核未实现api)
'''

import os

noaccess = ["/home/fs/fs_noaccess.txt"]
noopen = ["/home/fs/fs_noopen.txt"]
noread = ["/home/fs/fs_noread.txt"]
nowrite = ["/home/fs/fs_nowrite.txt"]
nocreate = ["/home/fs/fs_nocreate.txt"]
nodelete = ["/home/fs/fs_nodelete.txt"]
norename = ["/home/fs/fs_norename.txt"]
testpoints = 0
passpoints= []

def control_access_noaccess():
    global noaccess, testpoints, passpoints
    print("Test point 1: access ")
    if os.path.exists(noaccess[0]) :
        print("Test Fail !\n")
        testpoints += 1
    else :
        print("Test Pass !\n")
        passpoints.append(testpoints)
        testpoints += 1

def control_access_noopen():
    global noopen, testpoints, passpoints
    print("Test point 2: open ")
    try:
        f = open(noopen[0],'r')
        print("Test Fail !\n")
    except OSError as reason:
        print("Test Pass !\n")
        passpoints.append(testpoints)
    finally:
        testpoints += 1
        if f in locals():
            f.close()

def control_access_noread():
    global noread, testpoints, passpoints
    print("Test point 3: read ")

    # open
    try:
        f = open(noread[0],'r')
    except:
        print("Test Fail ! It can't open!\n")
        testpoints += 1
        return

    # read
    try:
        data = f.readline()
        print("Test Fail !")
    except :
        print("Test Pass !")
        passpoints.append(testpoints)
    finally:
        testpoints += 1
        if f in locals():
            f.close()

def control_access_nowrite():
    global noread, testpoints, passpoints
    print("Test point 4: write ")

    # open
    try:
        f = open(nowrite[0],'w')
    except:
        print("Test Fail ! It can't open!\n")
        testpoints += 1
        return

    # write
    try:
        f.write("test")
        print("Test Fail !")
    except :
        print("Test Pass !")
        passpoints.append(testpoints)
    finally:
        testpoints += 1
        if f in locals():
            f.close()

def control_access_nocreate():
    global nocreate, testpoints, passpoints
    print("Test point 4: create ")

    # create
    try:
        os.mknod(nocreate[0])
        print("Test Fail !")
    except :
        print("Test Pass !")
        passpoints.append(testpoints)
    finally:
        testpoints += 1

def control_access_nodelete():
    global nodelete, testpoints, passpoints
    print("Test point 5: delete ")

    # remove
    try:
        os.remove(nodelete[0])
        print("Test Fail !")
    except :
        print("Test Pass !")
        passpoints.append(testpoints)
    finally:
        testpoints += 1

def control_access_norename():
    global nodelete, testpoints, passpoints
    print("Test point 6: rename ")
    
    # rename
    try:
        os.rename(norename[0], norename[0] + ".bak")
        print("Test Fail !")
    except :
        print("Test Pass !")
        passpoints.append(testpoints)
    finally:
        testpoints += 1


def show_result():
    global testpoints, passpoints
    print("\nAll Test Finish!")
    print("Here is the result: ")
    print("All points:", testpoints)
    print("Pass points:", len(passpoints))



if __name__ == '__main__':
    print("\nWelcome To Control Access Test of Sandbox File System!\n\n")
    control_access_noaccess()
    control_access_noopen()
    control_access_noread()
    control_access_nowrite()
    # control_access_nocreate()
    control_access_nodelete()
    # control_access_norename()

    show_result()

