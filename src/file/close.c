/*
 * close.c
 *
 * Posix C file close function
 * 
 * MIT License (see: LICENSE)
 * copyright (c) 2021 tomaz stih
 *
 * 10.08.2021   tstih
 *
 */
#include <errno.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/bdos.h>
#include <file/fcb.h>
#include <file/fd.h>

int close(int fd) {

    /* Get the descriptor. */
    fd_t* fdblk=_fd_get(fd);
    if (fdblk==NULL) {
        errno = EBADF;
        return -1;
    }

    /* Flush dirty DMA block. */
    if (fdblk->dmadirty) 
        fsync(fd); 

    /* Call file close on BDOS 
       TODO: Manage hardware error. */
    bdos_ret_t ret;
    bdosret(F_CLOSE,(uint16_t)&(fdblk->fcb),&ret);
    if (ret.reta==BDOS_FAILURE) {
        errno=EIO;
        return -1;
    }

    /* Finally, release the file descriptor. */
    _fd_set(fd,NULL);

    /* Free the memory. */
    free(fdblk);

    /* And return success. */
    errno = 0;
    return 0;
}