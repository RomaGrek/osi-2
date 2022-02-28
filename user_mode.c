#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <linux/ptrace.h>
#include <stdbool.h>
 
#define WR_AVALUE _IOW('a','a',struct message*)
#define WR_SVALUE _IOW('a','c',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)


 
struct my_block_device
{
    int bd_openers;
    int bd_holders;
    bool bd_read_only;
    int bd_fsfreeze_count;

};

enum my_memblock_flags {
	MEMBLOCK_NONE		= 0x0,	/* No special request */
	MEMBLOCK_HOTPLUG	= 0x1,	/* hotpluggable region */
	MEMBLOCK_MIRROR		= 0x2,	/* mirrored region */
	MEMBLOCK_NOMAP		= 0x4,	/* don't add to kernel direct mapping */
	MEMBLOCK_DRIVER_MANAGED = 0x8,	/* always detected via a driver */
};


struct my_memblock_region;

struct my_memblock_type
{
        unsigned long cnt;
        unsigned long max;
        struct my_memblock_region* regions;
};

struct my_memblock
{
        bool bottom_up;
        struct my_memblock_type memory;
};

 
struct message {
        struct my_block_device bd;
        struct my_memblock mem;
};
 
 
int main(int argc, char *argv[]) {
    int fd;
    //struct bpf_redirect_info_short *bpf;
    int32_t value = atoi(argv[1]);
    char* pth = argv[2];
    struct message msg;
   
        printf("\nOpening Driver\n");
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
 
        printf("Writing Pid to Driver\n");
        ioctl(fd, WR_SVALUE, (int32_t*) &value);
 
    printf("Writing Path to Driver\n");
    ioctl(fd, WR_AVALUE, pth);
 
        printf("Reading Value from Driver\n");
        ioctl(fd, RD_VALUE, (struct message*) &msg);
       
        printf("BLOCK_DEVICE:\n");
        printf("holders: %d\n", msg.bd.bd_holders);
        printf("openers: %d\n", msg.bd.bd_openers);
        printf("read_only_flag: %d\n", msg.bd.bd_read_only);
        printf("count of freeze processes: %d\n", msg.bd.bd_fsfreeze_count);

        printf("MEMBLOCK:\n");
        printf("bottom up flag (memblock): %d\n", msg.mem.bottom_up);
        printf("cnt (memblock_type): %ld\n", msg.mem.memory.cnt);
        printf("max (memblock_type): %ld\n", msg.mem.memory.max);
        
        printf("address of memblock_region: %p\n", msg.mem.memory.regions);

       
    


   
     printf("_________________________\n");
     
        printf("Closing Driver\n");
        //close(fd);
 
}
