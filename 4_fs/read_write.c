#include "testfs.h"
#include "list.h"
#include "super.h"
#include "block.h"
#include "inode.h"

/* given logical block number, read the corresponding physical block into block.
 * return physical block number.
 * returns 0 if physical block does not exist.
 * returns negative value on other errors. */
 static int
 testfs_read_block(struct inode *in, int log_block_nr, char *block)
 {
 	//printf("log_block_nr:%d\n",log_block_nr);
 	int phy_block_nr = 0;
 	assert(log_block_nr >= 0);

 	// when log block number is less 0~9
 	if (log_block_nr < NR_DIRECT_BLOCKS) {
 		phy_block_nr = (int)in->in.i_block_nr[log_block_nr];
 	}

 	else {
 		log_block_nr -= NR_DIRECT_BLOCKS;

 		if (log_block_nr >= NR_INDIRECT_BLOCKS)
 		{
 			//need to implement DID part, not done yet.
 			if (in->in.i_dindirect > 0)
 			{
 	 		log_block_nr -= NR_INDIRECT_BLOCKS; //log block ranges from 0 ~ 4194303

 	 		int level1_block = DIVROUNDUP(log_block_nr + 1,2048) - 1;   // ranges 0~2047

	 		int level2_block= log_block_nr % 2048;


	 		read_blocks(in->sb, block, in->in.i_dindirect, 1);

	 		
	 		if (((int*)block)[level1_block]!=0)
	 		{
	 			read_blocks(in->sb, block, ((int*)block)[level1_block], 1);

	 			if (((int*)block)[level2_block]!=0)
	 			{
	 				phy_block_nr = ((int *)block)[level2_block];
	 				read_blocks(in->sb, block, phy_block_nr, 1);
	 				return phy_block_nr;	
	 			}

	 		}
	 	}
	 	return phy_block_nr;
	 	}

	 if (in->in.i_indirect > 0) {
	 	read_blocks(in->sb, block, in->in.i_indirect, 1);
	 	phy_block_nr = ((int *)block)[log_block_nr];
	 }
	}

	if (phy_block_nr > 0) {
		read_blocks(in->sb, block, phy_block_nr, 1);
	} else {

		/* we support sparse files by zeroing out a block that is not
		 * allocated on disk. */
		bzero(block, BLOCK_SIZE);
	}
	return phy_block_nr;
}

int
testfs_read_data(struct inode *in, char *buf, off_t start, size_t size)
{
	char block[BLOCK_SIZE];
	long block_nr = start / BLOCK_SIZE;
	long block_ix = start % BLOCK_SIZE;
	int ret;

	assert(buf);
	if (start + (off_t) size > in->in.i_size) {
		size = in->in.i_size - start;
	}
	if (block_ix + size > BLOCK_SIZE) {
		int req_b = DIVROUNDUP(block_ix+size,BLOCK_SIZE);
		int i;
		size_t read_block_size, remaining_bytes;
		remaining_bytes = size;

		read_block_size = BLOCK_SIZE - block_ix;
		remaining_bytes -= read_block_size;
		if ((ret = testfs_read_block(in, block_nr, block)) < 0)
			return ret;
		memcpy(buf, block+block_ix, read_block_size);
		block_nr++;

		for (i=1;i<req_b; i++)
		{
			memset(&block[0],0,sizeof(block));

			if ((ret = testfs_read_block(in, block_nr, block)) < 0)
				return ret;

			if (remaining_bytes >= BLOCK_SIZE)
			{
				read_block_size = BLOCK_SIZE;
				remaining_bytes -= BLOCK_SIZE;
			}
			else if (remaining_bytes <BLOCK_SIZE)
			{
				read_block_size= remaining_bytes;
				remaining_bytes=0;
			}
			else if (remaining_bytes ==0)
			{
				break;
			}
			memcpy(buf+size - (remaining_bytes + read_block_size), block, read_block_size);
			block_nr++;
		}

		return size;

	}
	if ((ret = testfs_read_block(in, block_nr, block)) < 0)
		return ret;
	memcpy(buf, block + block_ix, size);
	/* return the number of bytes read or any error */
	return size;
}

/* given logical block number, allocate a new physical block, if it does not
 * exist already, and return the physical block number that is allocated.
 * returns negative value on error. */
 static int
 testfs_allocate_block(struct inode *in, int log_block_nr, char *block)
 {
 	if (log_block_nr>4196361)
 		return -EFBIG;

 	// //printf("log_block_nr:%d\n",log_block_nr);
 	// }
 	int phy_block_nr;
 	char indirect[BLOCK_SIZE];
 	char dindirect[BLOCK_SIZE];
 	char dindirect2[BLOCK_SIZE];
 	int indirect_allocated = 0;
 	//int dindirect_allocated = 0;
 	
 	assert(log_block_nr >= 0);
 	phy_block_nr = testfs_read_block(in, log_block_nr, block);

	/* phy_block_nr > 0: block exists, so we don't need to allocate it, 
	   phy_block_nr < 0: some error */
 	if (phy_block_nr != 0)
 		return phy_block_nr;



	/* allocate a direct block */
 	if (log_block_nr < NR_DIRECT_BLOCKS) {
 		assert(in->in.i_block_nr[log_block_nr] == 0);
 		phy_block_nr = testfs_alloc_block_for_inode(in);
		
 		if (phy_block_nr >= 0) {
 			in->in.i_block_nr[log_block_nr] = phy_block_nr;
 		}
 		return phy_block_nr;
 	}
 		
 	log_block_nr -= NR_DIRECT_BLOCKS;

 	if (log_block_nr >= NR_INDIRECT_BLOCKS)
 	{
 	
 		log_block_nr -= NR_INDIRECT_BLOCKS; //log block ranges from 0 ~ 4194303
 		
 	
 		int level1_block = DIVROUNDUP(log_block_nr+1,2048) - 1;   // ranges 0~2047
 		
 		int level2_block= log_block_nr % 2048;

 		/* allocate an dindirect block */
 		if (in->in.i_dindirect == 0) 	
 		{	
 			bzero(dindirect, BLOCK_SIZE);
 			phy_block_nr = testfs_alloc_block_for_inode(in);
		
 			if (phy_block_nr < 0)
 				return phy_block_nr;
 			in->in.i_dindirect = phy_block_nr;

 		}

 		else   /* read dindirect block */
 		{
 			read_blocks(in->sb, dindirect, in->in.i_dindirect, 1); 
 		}

 		// Allocate 2nd level indirect block
 		if (((int *)dindirect)[level1_block] == 0)
 		{
 			bzero(dindirect2, BLOCK_SIZE);
 			phy_block_nr = testfs_alloc_block_for_inode(in);
 			if (phy_block_nr < 0)
 				return phy_block_nr;

 
 			((int*)dindirect)[level1_block]=phy_block_nr;

 			write_blocks(in->sb, dindirect, in->in.i_dindirect,1);
 		}

			// if (log_block_nr==51200){
			// testfs_free_block_from_inode(in,phy_block_nr);
			// return -ENOSPC;
			// }

 		// Read 2nd level indirect block
 		else 
 		{	
 			read_blocks(in->sb, dindirect2, ((int*)dindirect)[level1_block], 1);
 		}

 		// allocate direct block */
 		phy_block_nr=testfs_alloc_block_for_inode(in);

 		if (phy_block_nr == -ENOSPC)
		{
			// printf("1:%d \n",((int*)dindirect)[level1_block]);
			// printf("B\n");
			testfs_free_block_from_inode(in,((int*)dindirect)[level1_block]);
			((int*)dindirect)[level1_block]=0;
			write_blocks(in->sb, dindirect, in->in.i_dindirect,1);
			return phy_block_nr;
		}

	 	if (phy_block_nr >=0)  // update 2nd level indirect block
	 	{
	 		((int*)dindirect2)[level2_block]=phy_block_nr;
	 		write_blocks(in->sb, dindirect2, ((int*)dindirect)[level1_block], 1);
	 	}
	 	
	 	return phy_block_nr;
 		// Allocate 2nd-level indirect block

	 }

 	/**** Allocating in 1st level indirect blocks ****/

	if (in->in.i_indirect == 0) 	/* allocate an indirect block */
	 {	
	 	bzero(indirect, BLOCK_SIZE);
	 	phy_block_nr = testfs_alloc_block_for_inode(in);
	 	if (phy_block_nr < 0)
	 		return phy_block_nr;
	 	indirect_allocated = 1;
	 	in->in.i_indirect = phy_block_nr;
	 } 

	 else 
	{	/* read indirect block */
	 	read_blocks(in->sb, indirect, in->in.i_indirect, 1);
	}

	/* allocate direct block */
	assert(((int *)indirect)[log_block_nr] == 0);	
	phy_block_nr = testfs_alloc_block_for_inode(in);

	if (phy_block_nr >= 0) {
		/* update indirect block */
		((int *)indirect)[log_block_nr] = phy_block_nr;
		write_blocks(in->sb, indirect, in->in.i_indirect, 1);
	} 

	else if (indirect_allocated) 
	{
		/* free the indirect block that was allocated */
		testfs_free_block_from_inode(in, in->in.i_indirect);
	}

	return phy_block_nr;

}

int
testfs_write_data(struct inode *in, const char *buf, off_t start, size_t size)
{

	char block[BLOCK_SIZE];
	long block_nr = start / BLOCK_SIZE;
	//printf("__________ Enters write data with block_nr=%ld \n",block_nr);
	long block_ix = start % BLOCK_SIZE;
	int ret;
	// int req_b= ((start+size)/BLOCK_SIZE);
	long long part=start;

	if (block_ix + size > BLOCK_SIZE) {
		int req_b= DIVROUNDUP(block_ix+size,BLOCK_SIZE);
	
		int i;
		size_t write_block_size, remaining_bytes;
		remaining_bytes = size;

		write_block_size = BLOCK_SIZE - block_ix;
		remaining_bytes = remaining_bytes-write_block_size;
		
		ret = testfs_allocate_block(in, block_nr, block);
		if (ret < 0)
				return ret;
		
		memcpy(block+block_ix, buf, write_block_size);
		write_blocks(in->sb,block,ret,1);

		part += write_block_size;

		block_nr++;

		for (i=1; i<req_b; i++)
		{
			memset(&block[0],0,sizeof(block));
			ret = testfs_allocate_block(in, block_nr, block);

			if (ret < 0){
				break;
			}
			if (remaining_bytes >= BLOCK_SIZE)
			{
				write_block_size=BLOCK_SIZE;
				remaining_bytes=remaining_bytes - BLOCK_SIZE;
			}

			else if (remaining_bytes < BLOCK_SIZE)
			{
				write_block_size=remaining_bytes;
				remaining_bytes=0;
			}

			else if (remaining_bytes == 0)
			{
				break;
			}

			part += write_block_size;

			memcpy(block, buf+size - (remaining_bytes + write_block_size), write_block_size);
			write_blocks(in->sb, block, ret, 1);
			block_nr++;
		}

		

		if (size > 0)
			in->in.i_size = MAX(in->in.i_size, part);
		in->i_flags |= I_FLAGS_DIRTY;
		if (ret<0)
			return ret;
		return size;
	}
	/* ret is the newly allocated physical block number */
	ret = testfs_allocate_block(in, block_nr, block);
	if (ret < 0)
		return ret;
	memcpy(block + block_ix, buf, size);
	write_blocks(in->sb, block, ret, 1);
	/* increment i_size by the number of bytes written. */
	if (size > 0)
		in->in.i_size = MAX(in->in.i_size, start + (off_t) size);
	in->i_flags |= I_FLAGS_DIRTY;
	/* return the number of bytes written or any error */
	return size;
}

int
testfs_free_blocks(struct inode *in)
{
	int i;
	int e_block_nr;

	/* last block number */
	e_block_nr = DIVROUNDUP(in->in.i_size, BLOCK_SIZE);

	/* remove direct blocks */
	for (i = 0; i < e_block_nr && i < NR_DIRECT_BLOCKS; i++) {
		if (in->in.i_block_nr[i] == 0)
			continue;
		testfs_free_block_from_inode(in, in->in.i_block_nr[i]);
		in->in.i_block_nr[i] = 0;
	}
	e_block_nr -= NR_DIRECT_BLOCKS;
	/* remove indirect blocks */
	if (in->in.i_indirect > 0) {
		char block[BLOCK_SIZE];
		read_blocks(in->sb, block, in->in.i_indirect, 1);
		for (i = 0; i < e_block_nr && i < NR_INDIRECT_BLOCKS; i++) {
			testfs_free_block_from_inode(in, ((int *)block)[i]);
			((int *)block)[i] = 0;
		}
		testfs_free_block_from_inode(in, in->in.i_indirect);
		in->in.i_indirect = 0;
	}

	e_block_nr -= NR_INDIRECT_BLOCKS;
	
	if (e_block_nr >= 0) 
	{
		if (in->in.i_dindirect > 0)
		{
		
		int j;
		char block[BLOCK_SIZE];
		char block2[BLOCK_SIZE];
		read_blocks(in->sb, block, in->in.i_dindirect, 1);

			for (i=0; i< e_block_nr && i < NR_INDIRECT_BLOCKS; i++)
			{

			if (((int *)block)[i]!= 0)
			{
				read_blocks(in->sb, block2, ((int *)block)[i], 1);
				
				for (j=0; j<NR_INDIRECT_BLOCKS; j++)
				{
					testfs_free_block_from_inode(in, ((int *)block2)[j]);
					((int *)block2)[j] = 0;
				}
			}
			testfs_free_block_from_inode(in, ((int *)block)[i]);
			((int *)block)[i] = 0;

			}

		testfs_free_block_from_inode(in, in->in.i_dindirect);
		in->in.i_dindirect = 0;
		}
	}

	in->in.i_size = 0;
	in->i_flags |= I_FLAGS_DIRTY;
	return 0;
}