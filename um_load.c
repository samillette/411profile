//Timothy Colaneri
//Universal Machine on-disk load implementation

/* An invariant while loading an on disk program is recognized
in the Memset_T program variable which is a representation of the
UM's memory space. At any point while reading an on-disk program
this variable will contain a sequence of all of the previously
read instruction words. This should be a checkable property at
any point of input processing.*/ 

/****************************************************************/
#include"bitpack.h" //Bitpacking
#include"um_load.h" //Own header file
#include"seq.h"     //Hanson's sequence used to represent memory
#include<stdlib.h>  //Standard library

#define BYTESIZE 8
#define WORDSIZE 4
/****************************************************************/
extern Memseg_T Load_prog(FILE * fp){

    //GET THE FILE SIZE
    //Find the end of the file
    int a = ftell(fp);
    fseek(fp, 0L, SEEK_END);

    //ftell returns number of bytes to end; divide by bytes in word
    int size = ((ftell(fp))/WORDSIZE);
    fseek(fp, a,SEEK_SET);//rewind to the front of the file

    Memseg_T program = Memseg_init();
    uint32_t location = Memseg_map(program, size);

    //Iterate through all of the instructions in the passed in
    //program, assembeling each word and inserting it into a
    //Memory segment.
    for (int wordCTR = 0; wordCTR < size; wordCTR++){
        uint32_t last8 = getc(fp);
        last8 = last8 << 24;
        uint32_t midl8 = getc(fp);
        midl8 = midl8 << 16;
        uint32_t midf8 = getc(fp);
        midf8 = midf8 << 8;
        uint32_t first8 = getc(fp);

        uint32_t word = (((last8 | midl8) | midf8 ) | first8);

        //Store the instruction(word) into the memory space
        Memseg_store(program, word, location, wordCTR);
    }
    return program;
}