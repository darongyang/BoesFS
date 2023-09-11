rmDirIfExist() {
    if [ -d "$1" ]; then
        rm -r "$1"
    fi
}

rmDirIfExist "dir"
rmDirIfExist "rmdir"

mkdir "mkdir" "dir"

files=("lookup" "open" "read" "read1" "read2" "read3" "write" "unlink" "canAll" "wrunlink" "dir/test")

for ele in ${files[*]}; do
    echo "${ele}123" > "$ele.txt"
done

