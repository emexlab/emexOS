#include "_syscall.h"
#include <unistd.h>

// no errno access

ssize_t read(int fd, void *buf, size_t n) {
	return (ssize_t)_sc3(_SCAL_READ, fd, (long)buf, (long)n);
}

ssize_t write(int fd, const void *buf, size_t n) {
	return (ssize_t)_sc3(_SCAL_WRITE, fd, (long)buf, (long)n);
}

pid_t getpid(void) {
	return (pid_t)_sc1(_SCAL_GETPID, 0);
}

pid_t fork(void) {
	return (pid_t)_sc0(_SCAL_FORK);
}

void _exit(int status) {
	_sc1(_SCAL_EXIT, status);
	__builtin_unreachable();
}

int execve(const char *path, char *const argv[], char *const envp[]) {
	(void)envp;
	return (int)_sc2(_SCAL_EXECVE, (long)path, (long)argv);
}

int chdir(const char *path) { return (int)_sc1(_SCAL_CHDIR, (long)path); }
int mkdir(const char *path) { return (int)_sc1(_SCAL_MKDIR, (long)path); }
int rmdir(const char *path) { return (int)_sc1(_SCAL_UNLINK, (long)path); }
int unlink(const char *path) { return (int)_sc1(_SCAL_UNLINK, (long)path); }

char *getcwd(char *buf, size_t size) {
	long r = _sc2(_SCAL_GETCWD, (long)buf, (long)size);
	return (r > 0) ? buf : NULL;
}

off_t lseek(int fd, off_t offset, int whence) {
	return (off_t)_sc3(_SCAL_LSEEK, (long)fd, (long)offset, (long)whence);
}

int access(const char *pathname, int mode)
{
	(void)mode;

	/*try to open the file;
	 * if it succeeds it exists
		* */
	long fd = _sc2(_SCAL_OPEN, (long)pathname, 1 /*O_RDONLY*/);
	if (fd < 0) return -1;
	_sc1(_SCAL_CLOSE, fd);
	return 0;
}
