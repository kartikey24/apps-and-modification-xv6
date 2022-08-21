#include "simplefs-ops.h"
extern struct filehandle_t file_handle_array[MAX_OPEN_FILES]; // Array for storing opened files

int simplefs_create(char *filename){
    /*
	    Create file with name `filename` from disk
	*/
	for(int i=0; i<NUM_INODES; i++){
		struct inode_t *inodeptr;
		simplefs_readInode(i,inodeptr);
		if(inodeptr->status==INODE_IN_USE){
			if(strcmp(filename,inodeptr->name)==0) return -1;
		}
		free(inodeptr);
    }

	int inode_num = simplefs_allocInode();
	if(inode_num==-1) return -1;

	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
    memcpy(inode->name,filename,1);
    inode->status = INODE_IN_USE;
    inode->file_size = 0;
    for(int i=0; i<MAX_FILE_SIZE; i++)
        inode->direct_blocks[i] = -1;
	simplefs_writeInode(inode_num, inode);
	free(inode);
	
    return inode_num;
}


void simplefs_delete(char *filename){
    /*
	    delete file with name `filename` from disk
	*/


	for(int i=0; i<NUM_INODES; i++){
		struct inode_t *inode;
		simplefs_readInode(i,inode);
		if(inode->status==INODE_IN_USE){
			if(strcmp(filename,inode->name)==0) {
				for (int i = 0; i < MAX_FILE_SIZE; i++){
        			int n = inode->direct_blocks[i];
					if(n!=-1) simplefs_freeDataBlock(n);
				}
				simplefs_freeInode(i);
			}
		}
		free(inode);
    }
}

int simplefs_open(char *filename){
    /*
	    open file with name `filename`
	*/
	int num = -1;
	for(int i=0; i<NUM_INODES; i++){
		struct inode_t *inode;
		simplefs_readInode(i,inode);
		if(inode->status==INODE_IN_USE){
			if(strcmp(filename,inode->name)==0) {
				num = i;
			}
		}
		free(inode);
    }

	if(num==-1) return -1;

	for(int i=0; i<MAX_OPEN_FILES; i++){
		if(file_handle_array[i].inode_number==-1){
			file_handle_array[i].offset=0;
			file_handle_array[i].inode_number = num;
			return i;
		}
	}
	return -1;
}

void simplefs_close(int file_handle){
    /*
	    close file pointed by `file_handle`
	*/

	file_handle_array[file_handle].inode_number = -1;

}

int simplefs_read(int file_handle, char *buf, int nbytes){
    /*
	    read `nbytes` of data into `buf` from file pointed by `file_handle` starting at current offset
	*/
	if(file_handle_array[file_handle].inode_number==-1) return -1;
	struct inode_t *inode;
	simplefs_readInode(file_handle_array[file_handle].inode_number,inode);
	if(file_handle_array[file_handle].offset + nbytes > inode->file_size) return -1;

	char *tmp;
	
		int n = inode->direct_blocks[0];
		if(n!=-1){
		char *tmp1;
		simplefs_readDataBlock(n,tmp1);
		tmp = strdup(tmp1);
		free(tmp1);
	
	for (int i = 1; i < MAX_FILE_SIZE; i++){
        int n = inode->direct_blocks[i];
		if(n==-1) break;
		char *tmp1;
		simplefs_readDataBlock(n,tmp1);
		strcat(tmp,tmp1);
		free(tmp1);
	}}
	buf = strdup(&tmp[file_handle_array[file_handle].offset]);
	free(tmp);
	free(inode);
    return 0;
}


int simplefs_write(int file_handle, char *buf, int nbytes){
    /*
	    write `nbytes` of data from `buf` to file pointed by `file_handle` starting at current offset
	*/
	if(file_handle_array[file_handle].inode_number==-1) return -1;
	struct inode_t *inode;
	simplefs_readInode(file_handle_array[file_handle].inode_number,inode);
	if(file_handle_array[file_handle].offset + nbytes > BLOCKSIZE*MAX_FILE_SIZE)
    	return -1;
	int offset = file_handle_array[file_handle].offset;
	int fail = 0;
	int k = 0;

	int block_start = offset/BLOCKSIZE;
	k = BLOCKSIZE-(offset%BLOCKSIZE);

	int num_block = (nbytes - k + BLOCKSIZE-1)/BLOCKSIZE;

	int new_block = block_start;
	if(offset%BLOCKSIZE !=0) new_block++;
	for (int i = new_block; i < new_block+num_block; i++){
        int n = simplefs_allocDataBlock();
		if(n==-1){
			fail = 1;
			break;
		}
		inode->direct_blocks[i]=n;
		
		simplefs_writeDataBlock(n,&buf[k]);
		k+=BLOCKSIZE;
	}
	k = BLOCKSIZE-(offset%BLOCKSIZE);
	if(fail==1){
		for (int i = new_block; i < new_block+num_block; i++){
			if(inode->direct_blocks[i]==-1) break;
			simplefs_freeDataBlock(inode->direct_blocks[i]);
			inode->direct_blocks[i]=-1;
		}
	}
	else if(offset%BLOCKSIZE !=0){
		char *tmp;
		simplefs_readDataBlock(inode->direct_blocks[block_start],tmp);
		strcat(tmp,buf);
		simplefs_writeDataBlock(inode->direct_blocks[block_start],tmp);
		free(tmp);
	}
	free(inode);
	if(fail) return -1;
	return 0;	
}


int simplefs_seek(int file_handle, int nseek){
    /*
	   increase `file_handle` offset by `nseek`
	*/
	if(file_handle_array[file_handle].inode_number==-1) return -1;
	struct inode_t *inode;
	simplefs_readInode(file_handle_array[file_handle].inode_number,inode);
	if(file_handle_array[file_handle].offset + nseek > inode->file_size) return -1;
	if(file_handle_array[file_handle].offset + nseek <0) return -1;
	file_handle_array[file_handle].offset = file_handle_array[file_handle].offset + nseek;
	free(inode);
    return 0;
}