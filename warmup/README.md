## Instructions

Your new syscall should look like this: int getsyscallinfo(void) .

Your system call returns the number of calls made since the system booted on success, and -1 on failure.

The count for a system call should only be incremented before the call is finished executing, not after.

## Report
### [Warmup xv6](https://github.com/OSUsatoru/xv6-projects/blob/main/warmup/Warmup.pdf)