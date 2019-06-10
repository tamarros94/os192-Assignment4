#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

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
main(int argc, char *argv[])
{

	  int pid;
	  char path[16];
	  int fd;
	  char buf[50];

	  //test1:cmdline
	  if (strcmp(argv[1], "cmdline")==0){
		  pid = getpid();
		  strcpy(path, "/proc/");
		  itoa(pid, path+6);
		  memmove(path+strlen(path), "/cmdline", 9);
		  printf(1, "user tries to open path: %s\n", path);
		  fd =  open(path, O_RDONLY);
		  read(fd, buf, 50);
		  printf(1, "cmdline: %s\n", buf);
		  exit();
	  }
	  //test2 : cwd
	  if (strcmp(argv[1], "cwd")==0){
	  		  pid = getpid();
	  		  strcpy(path, "/proc/");
	  		  itoa(pid, path+6);
	  		  memmove(path+strlen(path), "/cwd", 9);
	  		  printf(1, "user tries to open path: %s\n", path);
	  		  fd =  open(path, O_RDONLY);
	  		  read(fd, buf, 50);
	  		  printf(1, "cwd: %s\n", buf);
	  		  exit();
	  	  }
	  //test3: exe
	  if (strcmp(argv[1], "exe")==0){
		  	  printf(1, "testing exe\n");
	  		  pid = getpid();
	  		  strcpy(path, "/proc/");
	  		  itoa(pid, path+6);
	  		  memmove(path+strlen(path), "/exe", 9);
	  		  printf(1, "user tries to open path: %s\n", path);
	  		  fd =  open(path, O_RDONLY);
	  		  read(fd, buf, 50);
	  		  printf(1, "exe: %s\n", buf);
	  		  exit();
	  	  }
	  //test4: fdinfo
	  	  if (strcmp(argv[1], "fdinfo")==0){
	  	  		  pid = getpid();
	  	  		  strcpy(path, "/proc/");
	  	  		  itoa(pid, path+6);
	  	  		  memmove(path+strlen(path), "/fdinfo", 9);
	  	  		  printf(1, "user tries to open path: %s\n", path);
	  	  		  fd =  open(path, O_RDONLY);
	  	  		  read(fd, buf, 50);
	  	  		  //printf(1, "fdinfo: %s\n", buf);
	  	  		  exit();
	  	  }//test4.5: fdinfo with file
		  if (strcmp(argv[1], "2fdinfo")==0){
		  		  pid = getpid();
		  		  strcpy(path, "/proc/");
		  		  itoa(pid, path+6);
		  		  memmove(path+strlen(path), "/fdinfo/0", 9);
		  		  printf(1, "user tries to open path: %s\n", path);
		  		  fd =  open(path, O_RDONLY);
		  		  read(fd, buf, 50);
		  		  printf(1, "fdinfo: %s\n", buf);
		  		  exit();
		  }
	  //test5: status
	  // try with and without: sbrk(10000);
	  if (strcmp(argv[1], "status")==0){
	  		  pid = getpid();
	  		  strcpy(path, "/proc/");
	  		  itoa(pid, path+6);
	  		  memmove(path+strlen(path), "/status", 9);
	  		  printf(1, "user tries to open path: %s\n", path);
	  		  fd =  open(path, O_RDONLY);
	  		  read(fd, buf, 50);
	  		  printf(1, "status: %s\n", buf);
	  		  exit();
	  	  }
	  printf(1,"enter arguments!\n");

	  exit();
}
