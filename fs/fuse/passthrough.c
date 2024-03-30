// SPDX-License-Identifier: GPL-2.0

#include "fuse_i.h"

#include <linux/fuse.h>

#include <linux/uio.h>

#define PASSTHROUGH_IOCB_MASK                                                  \
	(IOCB_APPEND | IOCB_DSYNC | IOCB_HIPRI | IOCB_NOWAIT | IOCB_SYNC)

static void fuse_copyattr(struct file *dst_file, struct file *src_file)
{
	struct inode *dst = file_inode(dst_file);
	struct inode *src = file_inode(src_file);

	i_size_write(dst, i_size_read(src));
}

ssize_t fuse_passthrough_read_iter(struct kiocb *iocb_fuse,
				   struct iov_iter *iter)
{
	ssize_t ret;
	struct file *fuse_filp = iocb_fuse->ki_filp;
	struct fuse_file *ff = fuse_filp->private_data;
	struct file *passthrough_filp = ff->passthrough.filp;

	if (!iov_iter_count(iter))
		return 0;

	ret = vfs_iter_read(passthrough_filp, iter, &iocb_fuse->ki_pos,
			    iocb_to_rw_flags(iocb_fuse->ki_flags,
					     PASSTHROUGH_IOCB_MASK));

	return ret;
}

ssize_t fuse_passthrough_write_iter(struct kiocb *iocb_fuse,
				    struct iov_iter *iter)
{
	ssize_t ret;
	struct file *fuse_filp = iocb_fuse->ki_filp;
	struct fuse_file *ff = fuse_filp->private_data;
	struct inode *fuse_inode = file_inode(fuse_filp);
	struct file *passthrough_filp = ff->passthrough.filp;

	if (!iov_iter_count(iter))
		return 0;

	inode_lock(fuse_inode);

	file_start_write(passthrough_filp);
	ret = vfs_iter_write(passthrough_filp, iter, &iocb_fuse->ki_pos,
			     iocb_to_rw_flags(iocb_fuse->ki_flags,
					      PASSTHROUGH_IOCB_MASK));
	file_end_write(passthrough_filp);
	if (ret > 0)
		fuse_copyattr(fuse_filp, passthrough_filp);

	inode_unlock(fuse_inode);

	return ret;

int fuse_passthrough_open(struct fuse_dev *fud,
			  struct fuse_passthrough_out *pto)
{
	return -EINVAL;
}

int fuse_passthrough_setup(struct fuse_conn *fc, struct fuse_file *ff,
			   struct fuse_open_out *openarg)
{
	return -EINVAL;
}

void fuse_passthrough_release(struct fuse_passthrough *passthrough)
{
}
