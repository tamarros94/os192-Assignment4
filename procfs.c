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
#define NAME 100
#define STATUS 200

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
    if (ip->inum == namei("proc")->inum ||
        (ip->inum >= 1000 && ip->inum % 1000 == 0) ||
        (ip->inum == INODEINFO))

        ip->minor = T_DIR;
    else ip->minor = T_FILE;
}

void itoa(int n, char *str) {
    int temp, len;
    temp = n;
    len = 1;
    while (temp / 10 != 0) {
        len++;
        temp /= 10;
    }
    for (temp = len; temp > 0; temp--) {
        str[temp - 1] = (n % 10) + 48;
        n /= 10;
    }
    str[len] = '\0';
}

int init_proc_dirents(struct inode *ip, char *dst, int off, int n) {
    struct dirent proc_dirents[NPROC + 2];

    // init first 2 dirents . .. (namei("proc")->inum, namei("")->inum)
    proc_dirents[0].inum = ip->inum;
    strncpy(proc_dirents[0].name, ".", 1);
    proc_dirents[0].name[1] = '\0';
    proc_dirents[1].inum = namei("")->inum;
    strncpy(proc_dirents[1].name, "..", 2);
    proc_dirents[1].name[2] = '\0';

    // init pid dirents (pid*1000)
    int dirents_size = init_pid_dirents(ip, proc_dirents);

    // init 3 dirents: ideinfo, filestat, inodeinfo (NINODES+1,+2,+3)
    proc_dirents[dirents_size].inum = IDEINFO;
    strncpy(proc_dirents[dirents_size].name, "ideinfo", 7);
    proc_dirents[dirents_size].name[7] = '\0';
    dirents_size++;

    proc_dirents[dirents_size].inum = FILESTAT;
    strncpy(proc_dirents[dirents_size].name, "filestat", 8);
    proc_dirents[dirents_size].name[8] = '\0';

    proc_dirents[dirents_size].inum = INODEINFO;
    strncpy(proc_dirents[dirents_size].name, "inodeinfo", 9);
    proc_dirents[dirents_size].name[9] = '\0';
    dirents_size++;

    if (off >= dirents_size * sizeof(struct dirent))
        return 0;

    memmove(dst, (char *) ((uint) proc_dirents + (uint) off), n);
    return n;
}

int init_proc_pid_dirents(struct inode *ip, char *dst, int off, int n) {
    struct dirent pid_dirents[4];

    // proc/pid/.
    pid_dirents[0].inum = ip->inum;
    strncpy(pid_dirents[0].name, ".", 1);
    pid_dirents[0].name[1] = '\0';

    // proc/pid/..
    pid_dirents[1].inum = namei("proc")->inum;
    strncpy(pid_dirents[1].name, "..", 2);
    pid_dirents[1].name[2] = '\0';

    // proc/pid/name
    pid_dirents[2].inum = ip->inum + NAME;
    strncpy(pid_dirents[2].name, "name", 4);
    pid_dirents[2].name[4] = '\0';

    // proc/pid/status
    pid_dirents[3].inum = ip->inum + STATUS;
    strncpy(pid_dirents[3].name, "status", 6);
    pid_dirents[3].name[7] = '\0';

    if (off >= 4 * sizeof(struct dirent)) {
        return 0;
    }
    memmove(dst, (char *) ((uint) pid_dirents + (uint) off), n);
    return n;
}

// <proc>
// -- .
// -- ..
// -- ideinfo (201)
// -- filestat (202)
// -- <pid directories> (pid*1000)
// -- -- .
// -- -- ..
// -- -- name
// -- -- status
// -- <inodeinfo> (203)
// -- -- .
// -- -- ..
// -- -- [index of inodes in use in the open inode table]
int
procfsread(struct inode *ip, char *dst, int off, int n) {
    struct inode *proc_inode = namei("proc");

    // case /proc/ directory
    if (ip == namei("proc"))
        return init_proc_dirents(ip, dst, off, n);
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
    if (ip->inum > 200 && ip->inum < 1000)
        return read_inodeinfo_file(ip, dst, off, n);

    cprintf("procfsread error: inode %d wasn't found", ip->inum);
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
