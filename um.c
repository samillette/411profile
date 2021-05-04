//Timothy Colaneri
//Universal Machine driver

/**********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include"um_load.h"
#include"um_exec.h"
/**********************************************************/

/**********************************************************/
//This main function is the driver for the universal
//machine
int main(int argc, char * argv[]){

    //Check number of arguments
    if (argc != 2){
        fprintf(stderr,"Error, incorrect arguments.\n");
        exit(1);
    }

    //Open the file
    FILE *fp = fopen(argv[1],"rb");
    if (fp == NULL){
        fprintf(stderr,"Error opening file.\n");
        exit(1);
    }

    //Execute UM
    Memseg_T program = Load_prog(fp);//Read and load on-disk program
    fclose(fp);                     //Close the file
    uint32_t registers[8] = { 0 };  //initialize registers
    Interp_prog(program,registers); //interpret the program
    Memseg_free(program);           //Free the memory

    return 0;//Successful exit
}
/**********************************************************/