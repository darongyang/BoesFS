function make_kernel() {
    echo "[INFO]building BoesFS-in-Kernel"
    cd BoesFS-in-Kernel/
    make
    cd ..
}

function make_agent() {
    echo "[INFO]building BoesFS-Agent"
    work_dir=$HOME"/.boesfs"
    if [ ! -d "$work_dir" ]; then
    mkdir $work_dir
    fi
    acl_dir=$work_dir"/acl"
    if [ ! -d "$acl_dir" ]; then
    mkdir $acl_dir
    fi
    prog_dir=$acl_dir"/prog"
    if [ ! -d "$prog_dir" ]; then
    mkdir $prog_dir
    fi
    model_dir=$acl_dir"/model"
    if [ ! -d "$model_dir" ]; then
    mkdir $model_dir
    fi
    library_dir=$acl_dir"/library"
    if [ ! -d "$library_dir" ]; then
    mkdir $library_dir
    fi
    u_dir=$work_dir"/prog"
    if [ ! -d "$u_dir" ]; then
    mkdir $u_dir
    fi
    cd BoesFS-Agent/
    cp -r acl/model/* $model_dir
    cp -r acl/library/* $library_dir
    make
    sudo cp boesfs /usr/local/bin/boesfs
    sudo chmod +X /usr/local/bin/boesfs
    cd ..
}

function make_check_module() {
    echo "[INFO]building BoesFS-Check-Module"
    work_dir=$HOME"/.boesfs"
    if [ ! -d "$work_dir" ]; then
    mkdir $work_dir
    fi
    acl_dir=$work_dir"/acl"
    if [ ! -d "$acl_dir" ]; then
    mkdir $acl_dir
    fi
    prog_dir=$acl_dir"/prog"
    if [ ! -d "$prog_dir" ]; then
    mkdir $prog_dir
    fi
    cp -r BoesFS-Check-Module/acl/* $prog_dir
    cd $prog_dir
    LLC=llc CLANG=clang make
    cd -
    u_dir=$work_dir"/prog"
    if [ ! -d "$u_dir" ]; then
    mkdir $u_dir
    fi
    cp -r BoesFS-Check-Module/acl/*.h $u_dir
    cp -r BoesFS-Check-Module/api/* $u_dir
    cd $u_dir
    # LLC=llc CLANG=clang make
    # cp api_prog.o user_prog.o
    cd -
}

if [ $# == 0 ]
then
    make_kernel
    make_agent
    make_check_module
elif [ $1 == 1 ]
then
    make_kernel
elif [ $1 == 2 ]
then
    make_agent
elif [ $1 == 3 ]
then
    make_check_module
fi



 