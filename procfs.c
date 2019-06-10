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
  if(ip->minor==T_DIR) return 0;
  else return 1;
}

void 
procfsiread(struct inode* dp, struct inode *ip) {
    ip->major = PROCFS;
    ip->type = T_DEV;
    ip->valid = 1;
    if(ip->inum == namei("proc")->inum || ip->inum%1000==0 || (ip->inum > NINODES && ip->inum <= INODEINFO))
        ip->minor = T_DIR;
    else ip->minor = T_FILE;

}

int
procfsread(struct inode *ip, char *dst, int off, int n) {
    // case /proc directory
        // init first 2 dirents . .. (namei("proc")->inum, namei("")->inum)
        // init pid dirents (pid*1000)
        // init 3 dirents: ideinfo, filestat, inodeinfo (NINODES+1,+2,+3)
        // write dirents arr + off -> dst, return n

    // case /proc/pid
        // init first 2 dirents . .. (ip->inum+100, +200)
        // init 2 dirents: name, status (ip->inum+NAME, +STATUS)
        // write dirents arr + off -> dst, return n

    // case not a directory
        // if NINODES < inum < 1000 ---> type = inum (201=ideinfo, 202=filestat, 203=inodeinfo)
            // handle each type case
            // if the type is inodeinfo(directory):
                // init first 2 dirents . ..
                // init #<inodes_in_use> dirents (INODEINFO+i)
                // write dirents arr + off -> dst, return n
        // else get pid and type
            // find the process that matches the pid
                // handle each type case

    // case error

    return 0;
}

int
procfswrite(struct inode *ip, char *buf, int n)
{
  return 0;
}

void
procfsinit(void)
{
  devsw[PROCFS].isdir = procfsisdir;
  devsw[PROCFS].iread = procfsiread;
  devsw[PROCFS].write = procfswrite;
  devsw[PROCFS].read = procfsread;
}
