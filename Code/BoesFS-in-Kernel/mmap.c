/*
 * Copyright (c) 1998-2022 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2022 Stony Brook University
 * Copyright (c) 2003-2022 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "boesfs.h"

static int boesfs_fault(struct vm_fault *vmf)
{
	int err;
        struct vm_area_struct *vma = vmf->vma;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = BOESFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);

	lower_file = boesfs_lower_file(file);
	/*
	 * XXX: vm_ops->fault may be called in parallel.  Because we have to
	 * resort to temporarily changing the vma->vm_file to point to the
	 * lower file, a concurrent invocation of boesfs_fault could see a
	 * different value.  In this workaround, we keep a different copy of
	 * the vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.  A
	 * better fix would be to change the calling semantics of ->fault to
	 * take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	vmf->vma = &lower_vma; /* override vma temporarily */
	err = lower_vm_ops->fault(vmf);
	vmf->vma = vma; /* restore vma*/
	return err;
}

static int boesfs_page_mkwrite(struct vm_fault *vmf)
{
	int err = 0;
        struct vm_area_struct *vma = vmf->vma;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;

	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = BOESFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);
	if (!lower_vm_ops->page_mkwrite)
		goto out;

	lower_file = boesfs_lower_file(file);
	/*
	 * XXX: vm_ops->page_mkwrite may be called in parallel.
	 * Because we have to resort to temporarily changing the
	 * vma->vm_file to point to the lower file, a concurrent
	 * invocation of boesfs_page_mkwrite could see a different
	 * value.  In this workaround, we keep a different copy of the
	 * vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.
	 * A better fix would be to change the calling semantics of
	 * ->page_mkwrite to take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	vmf->vma = &lower_vma; /* override vma temporarily */
	err = lower_vm_ops->page_mkwrite(vmf);
	vmf->vma = vma; /* restore vma */
out:
	return err;
}

static ssize_t boesfs_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	/*
	 * This function should never be called directly.  We need it
	 * to exist, to get past a check in open_check_o_direct(),
	 * which is called from do_last().
	 */
	return -EINVAL;
}

static sector_t boesfs_bmap(struct address_space *mapping, sector_t block)
{
	int err = 0;
	struct inode *inode;
	struct inode *lower_inode;

	inode = (struct inode *) mapping->host;
	lower_inode = boesfs_lower_inode(inode);
	if (lower_inode->i_mapping->a_ops->bmap)
		err = lower_inode->i_mapping->a_ops->bmap(lower_inode->i_mapping,
							  block);
	return err;
}

const struct address_space_operations boesfs_aops = {
	.direct_IO = boesfs_direct_IO,
	.bmap = boesfs_bmap,
};

const struct vm_operations_struct boesfs_vm_ops = {
	.fault		= boesfs_fault,
	.page_mkwrite	= boesfs_page_mkwrite,
};
