//Timothy Colaneri
//Universal Machine on-disk load interface
#include<stdio.h>
#include"um_mem.h"

/* Load_prog takes in a file pointer to a UM program
and loads that program into a memory space representation
Memset_T. The memory space is then returned to the caller.*/
extern Memseg_T Load_prog(FILE * fp);