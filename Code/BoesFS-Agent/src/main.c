/*
   Based on userns_child_exec.c from Michael Kerrisk.

   Copyright 2018, Michael Kerrisk
   Copyright 2018, Ashish Bijlani
   Licensed under GNU General Public License v2 or later

   Execute a process under SandFS sandbox.

   It first create a child process that executes a shell command in new
   namespace(s); allow UID and GID mappings to be specified when
   creating a user namespace.
*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>

#include <utils.h>
#include <interact.h>
#include <non_interact.h>




int main(int argc, char *argv[])
{
	if(argc == 1)
		// argc=1，即用户只运行了./boesfs不带参数，调用interactive()进入交互模式
		interactive();
	else
		non_interactive(argc, argv);
	return 0;
}