#include "types.h"
#include "stat.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define NINODES 200
#define IDEINFO 201
#define FILESTAT 202
#define INODEINFO 203
#define NAME 300
#define STATUS 400

int
procfsisdir(struct inode *ip) {
    if (ip->minor == T_DIR) return 0;
    else return 1;
}

void
procfsiread(struct inode *dp, struct inode *ip) {
    ip->major = PROCFS;
    ip->type = T_DEV;
    ip->valid = 1;
    if (ip->inum == namei("proc")->inum || ip->inum % 1000 == 0 || (ip->inum > NINODES && ip->inum <= INODEINFO))
        ip->minor = T_DIR;
    else ip->minor = T_FILE;

}

void itoa(int n, char *str){
    int temp, len;
    temp = n;
    len = 1;
    while (temp/10!=0){
        len++;
        temp /= 10;
    }
    for (temp = len; temp > 0; temp--){
        str[temp-1] = (n%10)+48;
        n/=10;
    }
    str[len]='\0';
}

int
procfsread(struct inode *ip, char *dst, int off, int n) {
    struct dirent proc_dirents[NPROC + 2];
    struct inode *proc_inode = namei("proc");

    // case /proc/ directory
    if (ip == proc_inode)
        return init_proc_dirents(ip, dst, off, n);
    // init first 2 dirents . .. (namei("proc")->inum, namei("")->inum)
    // init pid dirents (pid*1000)
    // init 3 dirents: ideinfo, filestat, inodeinfo (NINODES+1,+2,+3)
    // write dirents arr + off -> dst, return n

    // case /proc/pid/
    if (ip->inum % 1000 == 0)
        return init_proc_pid_dirents(ip, dst, off, n);
    // init first 2 dirents . .. (ip->inum+100, +200)
    // init 2 dirents: name, status (ip->inum+NAME, +STATUS)
    // write dirents arr + off -> dst, return n

    // case /proc/inodeinfo/
    if (ip->inum == INODEINFO)
        return init_proc_inodeinfo_dirents(ip, dst, off, n);
    // init first 2 dirents . ..
    // init #<inodes_in_use> dirents (INODEINFO+i)
    // write dirents arr + off -> dst, return n

    // not a directory

    // case /proc/ideinfo
    if (ip->inum == IDEINFO)
        return read_IDEINFO(ip, dst, off, n);

    // case /proc/filestat
    if (ip->inum == FILESTAT)
        return read_FILESTAT(ip, dst, off, n);


    // case /proc/pid/<file>
    if (ip->inum > 1000)
        return handle_pid_files(ip, dst, off, n);
        // else get pid and type
        // find the process that matches the pid
        // handle each type case

    // case /proc/inodeinfo/<file>
    if(ip->inum > 200 && ip->inum < 1000)
        return read_inodeinfo_file(ip,dst,off,n);

    cprintf("procfsread error: inode %d wasn't found",ip->inum);
    // case error

    return 0;
}

int
procfswrite(struct inode *ip, char *buf, int n) {
    return 0;
}

void
procfsinit(void) {
    devsw[PROCFS].isdir = procfsisdir;
    devsw[PROCFS].iread = procfsiread;
    devsw[PROCFS].write = procfswrite;
    devsw[PROCFS].read = procfsread;
}
