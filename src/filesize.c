#include <sys/stat.h>

long long filesize(int fd)
{
	struct stat filestat;
	drop(fstat(fd,&filestat),"file status error");
	return filestat.st_size;
}
