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

// TODO return actual size from read file operations written (if it's smaller than n)
#define NINODES 200
#define IDEINFO 201
#define FILESTAT 202
#define INODEINFO 203
#define NAME 100
#define STATUS 200


int
procfsisdir(struct inode *ip) {
    if (ip->inum == namei("proc")->inum) {
        ip->size = 1;
        ip->minor = T_DIR;
    }
    if (ip->minor == T_DIR) {
        return 1;
    } else return 0;
}

void
procfsiread(struct inode *dp, struct inode *ip) {
    ip->major = PROCFS;
    ip->type = T_DEV;
    ip->valid = 1;
    ip->nlink = 1;
    if ((ip->inum >= 1000 && ip->inum % 1000 == 0) ||
        (ip->inum == INODEINFO))
        ip->minor = T_DIR;
    else ip->minor = T_FILE;
}

void itoa1(int n, char *str) {
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
    struct dirent proc_dirents[NPROC + 5];

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
    dirents_size++;


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
    pid_dirents[3].name[6] = '\0';

    if (off >= 4 * sizeof(struct dirent)) {
        return 0;
    }
    memmove(dst, (char *) ((uint) pid_dirents + (uint) off), n);
    return n;
}

int handle_pid_files(struct inode *ip, char *dst, int off, int n) {
    int pid = ip->inum / 1000;
    int filenum = ip->inum % 1000;
    struct proc *p;
    p = get_proc_by_pid(pid);
    if (filenum == NAME) {
        char *name = p->name;
        memmove(dst, "name: ", strlen("name: "));
        memmove(dst + strlen("name: "), name, strlen(name) + 1);
    }
    if (filenum == STATUS) {
        int state = p->state;
        char proc_size[10];
        if (state == RUNNABLE) {
            memmove(dst, "status: runnable, size: ", strlen("status: runnable, size: "));
            itoa1(p->sz, proc_size);
            memmove(dst + strlen("status: runnable, size: "), proc_size, strlen(proc_size) + 1);
        }
            // state = running
        else {
            memmove(dst, "status: running, size: ", strlen("status: running, size: "));
            itoa1(p->sz, proc_size);
            memmove(dst + strlen("status: running, size: "), proc_size, strlen(proc_size) + 1);
        }
    }
    if(off > 0) return 0;
    return n;
}

int read_IDEINFO(struct inode *ip, char *dst, int off, int n) {
    char waiting_count[10];
    char waiting_read_count[10];
    char waiting_write_count[10];
    int buff_index = 0;

    // Waiting
    memmove(dst, "Waiting operations: ", strlen("Waiting operations: "));
    buff_index += strlen("Waiting operations: ");

    itoa1(count_waiting(), waiting_count);

    memmove(dst + buff_index, waiting_count, strlen(waiting_count));
    buff_index += strlen(waiting_count);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // Read
    memmove(dst + buff_index, "Read waiting operations: ", strlen("Read waiting operations: "));
    buff_index += strlen("Read waiting operations: ");

    itoa1(count_read_waiting(), waiting_read_count);

    memmove(dst + buff_index, waiting_read_count, strlen(waiting_read_count));
    buff_index += strlen(waiting_count);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // Write
    memmove(dst + buff_index, "Write waiting operations: ", strlen("Write waiting operations: "));
    buff_index += strlen("Write waiting operations: ");

    itoa1(count_write_waiting(), waiting_write_count);

    memmove(dst + buff_index, waiting_write_count, strlen(waiting_write_count));
    buff_index += strlen(waiting_write_count);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // Working blocks
    char *buf = get_working_blocks_list();
    memmove(dst + buff_index, buf, strlen(buf));
    buff_index += strlen(buf);
    memmove(dst + buff_index, "\0", 1);

    if(off > 0) return 0;

    return buff_index;
}

int read_FILESTAT(struct inode *ip, char *dst, int off, int n) {
    char free_fds[10];
    char unique_fds[10];
    char writeable_fds[10];
    char readable_fds[10];
    char ratio[10];

    int buff_index = 0;

    // free fds
    memmove(dst, "Free fds: ", strlen("Free fds: "));
    buff_index += strlen("Free fds: ");

    itoa1(count_free_fds(), free_fds);

    memmove(dst + buff_index, free_fds, strlen(free_fds));
    buff_index += strlen(free_fds);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // unique fds
    memmove(dst + buff_index, "Unique inode fds: ", strlen("Unique inode fds: "));
    buff_index += strlen("Unique inode fds: ");

    itoa1(count_unique_fds(), unique_fds);

    memmove(dst + buff_index, unique_fds, strlen(unique_fds));
    buff_index += strlen(unique_fds);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // writeable fds
    memmove(dst + buff_index, "Writeable fds: ", strlen("Writeable fds: "));
    buff_index += strlen("Writeable fds: ");

    itoa1(count_writeable_fds(), writeable_fds);

    memmove(dst + buff_index, writeable_fds, strlen(writeable_fds));
    buff_index += strlen(writeable_fds);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // readable fds
    memmove(dst + buff_index, "Readable fds: ", strlen("Readable fds: "));
    buff_index += strlen("Readable fds: ");

    itoa1(count_readable_fds(), readable_fds);

    memmove(dst + buff_index, readable_fds, strlen(readable_fds));
    buff_index += strlen(readable_fds);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // refs per fds
    memmove(dst + buff_index, "Refs per fds: ", strlen("Refs per fds: "));
    buff_index += strlen("Refs per fds: ");

    itoa1(get_refs_per_fds(), ratio);

    memmove(dst + buff_index, ratio, strlen(ratio));
    buff_index += strlen(ratio);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    memmove(dst + buff_index, "\0", 1);

    if(off > 0) return 0;

    return buff_index;
}

int init_proc_inodeinfo_dirents(struct inode *ip, char *dst, int off, int n) {
    struct dirent inodeinfo_dirents[NINODE + 2];

    // proc/inodeinfo/.
    inodeinfo_dirents[0].inum = ip->inum;
    strncpy(inodeinfo_dirents[0].name, ".", 1);
    inodeinfo_dirents[0].name[1] = '\0';

    // proc/inodeinfo/..
    inodeinfo_dirents[1].inum = namei("proc")->inum;
    strncpy(inodeinfo_dirents[1].name, "..", 2);
    inodeinfo_dirents[1].name[2] = '\0';

    int dirent_size = init_inodeinfo_dirents(ip, inodeinfo_dirents);


    if (off >= dirent_size * sizeof(struct dirent)) {
        return 0;
    }
    memmove(dst, (char *) ((uint) inodeinfo_dirents + (uint) off), n);
    return n;
}

int read_inodeinfo_file(struct inode *ip, char *dst, int off, int n) {
    int cache_idx = ip->inum % 300;
    char device[10];
    char inum[10];
    char valid[10];
    char type[10];
    char major[10];
    char minor[10];
    char nlinks[10];
    char blocks_used[10];

    int buff_index = 0;

    // device num
    memmove(dst, "Device: ", strlen("Device: "));
    buff_index += strlen("Device: ");

    itoa1(get_device_num(cache_idx), device);

    memmove(dst + buff_index, device, strlen(device));
    buff_index += strlen(device);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // inum
    memmove(dst + buff_index, "Inode number: ", strlen("Inode number: "));
    buff_index += strlen("Inode number: ");

    itoa1(get_inum(cache_idx), inum);

    memmove(dst + buff_index, inum, strlen(inum));
    buff_index += strlen(inum);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // valid
    memmove(dst + buff_index, "is valid: ", strlen("is valid: "));
    buff_index += strlen("is valid: ");

    itoa1(is_valid(cache_idx), valid);

    memmove(dst + buff_index, valid, strlen(valid));
    buff_index += strlen(valid);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // type
    memmove(dst + buff_index, "type: ", strlen("type: "));
    buff_index += strlen("type: ");

    itoa1(get_type(cache_idx), type);

    memmove(dst + buff_index, type, strlen(type));
    buff_index += strlen(type);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // major
    memmove(dst + buff_index, "major minor: ", strlen("major minor: "));
    buff_index += strlen("major minor: ");

    itoa1(get_major(cache_idx), major);

    memmove(dst + buff_index, major, strlen(major));
    buff_index += strlen(major);

    memmove(dst + buff_index, ",", 1);
    buff_index++;

    // minor
    itoa1(get_minor(cache_idx), minor);

    memmove(dst + buff_index, minor, strlen(minor));
    buff_index += strlen(minor);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // hard links
    memmove(dst + buff_index, "hard links: ", strlen("hard links: "));
    buff_index += strlen("hard links: ");

    itoa1(get_nlinks(cache_idx), nlinks);

    memmove(dst + buff_index, nlinks, strlen(nlinks));
    buff_index += strlen(nlinks);

    memmove(dst + buff_index, "\n", 1);
    buff_index++;

    // blocks used
    memmove(dst + buff_index, "blocks used: ", strlen("blocks used: "));
    buff_index += strlen("blocks used: ");

    itoa1(get_used_blocks(cache_idx), blocks_used);

    memmove(dst + buff_index, blocks_used, strlen(blocks_used));
    buff_index += strlen(blocks_used);

    memmove(dst + buff_index, "\0", 1);

    if(off > 0) return 0;
    return buff_index;
}

// <proc>
// -- .
// -- ..
// -- ideinfo (201)
// -- filestat (202)
// -- <pid directories> (pid*1000)
// -- -- .
// -- -- ..
// -- -- name (1100)
// -- -- status (1200)
// -- <inodeinfo> (203)
// -- -- .
// -- -- ..
// -- -- [index of inodes in use in the open inode table]
int
procfsread(struct inode *ip, char *dst, int off, int n) {
    // case /proc/ directory
    if (ip->inum == namei("proc")->inum) {
        return init_proc_dirents(ip, dst, off, n);
    }
    // write dirents arr + off -> dst, return n

    // case /proc/pid/
    if (ip->inum % 1000 == 0){
    return init_proc_pid_dirents(ip, dst, off, n);}
    // init first 2 dirents . .. (ip->inum+100, +200)
    // init 2 dirents: name, status (ip->inum+NAME, +STATUS)
    // write dirents arr + off -> dst, return n

    // case /proc/inodeinfo/
    if (ip->inum == INODEINFO){
        return init_proc_inodeinfo_dirents(ip, dst, off, n);}
    // init first 2 dirents . ..
    // init #<inodes_in_use> dirents (INODEINFO+i)
    // write dirents arr + off -> dst, return n

    // not a directory

    // case /proc/ideinfo
    if (ip->inum == IDEINFO){
        return read_IDEINFO(ip, dst, off, n);}

    // case /proc/filestat
    if (ip->inum == FILESTAT){
        return read_FILESTAT(ip, dst, off, n);}


    // case /proc/pid/<file>
    if (ip->inum > 1000){
        return handle_pid_files(ip, dst, off, n);}
    // else get pid and type
    // find the process that matches the pid
    // handle each type case

    // case /proc/inodeinfo/<file>
    if (ip->inum >= 300 && ip->inum < 1000){
        return read_inodeinfo_file(ip, dst, off, n);}

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
    cprintf("procfswrite\n");
    devsw[PROCFS].isdir = procfsisdir;
    devsw[PROCFS].iread = procfsiread;
    devsw[PROCFS].write = procfswrite;
    devsw[PROCFS].read = procfsread;
}
