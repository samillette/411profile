//Timothy Colaneri
//Universal Machine interpreter implementation

/************************************************************************************/
#include"bitpack.h"
#include"um_exec.h"
#include<stdlib.h>
#include<stdio.h>
/************************************************************************************/
//In the world of ideas, struct Codeword represents a intruction word in a universal
//machine format. The integer opcode represents the operation code and a, b, and c
//represent the different values found within the word. This should be a checkable
// property throught execution of a program.
//
typedef struct Codeword {
    unsigned opcode, a, b, c;
}Codeword;
/*************************************************************************PROTOTYPES*/
static Codeword get_codeword(uint32_t word, Codeword codeword);
static void interp_word(Codeword word, unsigned *registers, Memseg_T program, unsigned * idx);

//In the world of ideas, a program can be represented as a tree with its given
//grammar, these following functions represent the different type of nodes that
//exist within the UM language. As we walk down a tree, we process the nodes
//type bases on its opcode instruction.
static inline void cond_move(uint32_t* registers, unsigned a, unsigned b, unsigned c);
static inline void seg_store(uint32_t* registers, unsigned a, unsigned b, unsigned c, Memseg_T prog);
static inline void seg_load(uint32_t* registers, unsigned a, unsigned b, unsigned c, Memseg_T prog);
static inline void add(uint32_t* registers, unsigned a, unsigned b, unsigned c);
static inline void mult(uint32_t* registers, unsigned a, unsigned b, unsigned c);
static inline void divide(uint32_t* registers, unsigned a, unsigned b, unsigned c);
static inline void nand(uint32_t* registers, unsigned a, unsigned b, unsigned c);
static inline void map_seg(uint32_t* registers, unsigned b, unsigned c, Memseg_T prog);
static inline void unmap_seg(uint32_t* registers, unsigned c, Memseg_T prog);
static inline void output(uint32_t* registers, unsigned c);
static inline void input(uint32_t* registers, unsigned c);
static inline void load_prog(uint32_t* registers, unsigned b, unsigned c, Memseg_T prog, unsigned *idx);
static inline void load_value(uint32_t* registers, unsigned a,uint32_t x);
static inline void halt(Memseg_T program);
/************************************************************************************/
//Interp_prog includes the main cycle in which UM interpretation is conducted.
//This fuction takes in a memory segment populated with a program and a pointer
//to an array representing registers and interprets that passed in program.
extern void Interp_prog(Memseg_T program, unsigned *registers){

    uint32_t word = 0;//instruction word
    Codeword codeword;//Unpacked word
    unsigned ctr = 0; //program instruction counter

    //Interpret the program
    while(1){
        word = Memseg_load(program,0,ctr);
        codeword = get_codeword(word,codeword);
        interp_word(codeword,registers,program, &ctr);
        ++ctr;
    }
}
/************************************************************************************/
//Function get_codeword takes in a 32 bit word and a codeword struct and then
//populates the structs members with the values found within the passed in word.
//This function will determine what type of operation was called and which
//values need to be retrieved as a result of the call.
static Codeword get_codeword(uint32_t word, Codeword codeword){

    unsigned opcode = Bitpack_getu(word, 4, 28);

    //Determine if we have a 3 or 2 register instruction and then assemble the
    //designated codeword
    if (opcode == 13){
        codeword.opcode = Bitpack_getu(word, 4, 28);
        codeword.a = Bitpack_getu(word, 3, 25);
        codeword.b = Bitpack_getu(word, 25, 0);
    }
    else{
        codeword.opcode = Bitpack_getu(word, 4 , 28);
        codeword.a = Bitpack_getu(word, 3, 6);
        codeword.b = Bitpack_getu(word, 3, 3);
        codeword.c = Bitpack_getu(word, 3, 0);
    }

    return codeword;
}
/************************************************************************************/
//Function interp_word determines what the operation code of the passed in codeword is
//and then passes control into the corresponding function which will correctly interpret
//the individual words instruction.
static void interp_word(Codeword word, unsigned *registers, Memseg_T program, unsigned * idx){

    //INSTRUCTION SWITCH                                        INSTRUCTION:
    switch(word.opcode){
        case 0:                                                 //CONDITIONAL MOVE
            cond_move(registers,word.a,word.b,word.c);
            break;
        case 1:                                                 //SEGMENTED LOAD
            seg_load(registers,word.a,word.b,word.c,program);
            break;
        case 2:                                                 //SEGMENTED STORE
            seg_store(registers, word.a,word.b,word.c,program);
            break;
        case 3:                                                 //ADDITION
            add(registers,word.a,word.b,word.c);
            break;
        case 4:                                                 //MULTIPLICATION
            mult(registers,word.a,word.b,word.c);
            break;
        case 5:                                                 //DIVISION
            divide(registers,word.a,word.b,word.c);
            break;
        case 6:                                                 //BITWISE NAND
            nand(registers,word.a,word.b,word.c);
            break;
        case 7:                                                 //HALT
            halt(program);
            break;
        case 8:                                                 //MAP SEGMENT
            map_seg(registers, word.b, word.c, program);
            break;
        case 9:                                                 //UNMAP SEGMENT
            unmap_seg(registers, word.c, program);
            break;
        case 10:                                                //IO OUTPUT
            output(registers, word.c);
            break;
        case 11:                                                //IO INPUT
            input(registers, word.c);
            break;
        case 12:                                                //LOAD PROGRAM
            load_prog(registers,word.b,word.c,program,idx);
            break;
        case 13:                                                //LOAD VALUE
            load_value(registers, word.a, (uint32_t)word.b);
            break;           
    }
}
/********************************************************************************************/
//Function cond_move interprets a conditional move instruction in the universal machine
static inline void cond_move(uint32_t *registers, unsigned a, unsigned b, unsigned c){
    if (registers[c] != 0){
        registers[a] = registers[b];
    }
}
/********************************************************************************************/
//Function seg_load interprets a segmented load instruction in the universal machine
static inline void seg_load(uint32_t *registers, unsigned a, unsigned b, unsigned c, Memseg_T prog){
    registers[a] = Memseg_load(prog, registers[b],registers[c]);
}
/********************************************************************************************/
//Function seg_store interprets a segmented store instruction in the universal machine
static inline void seg_store(uint32_t *registers, unsigned a, unsigned b, unsigned c, Memseg_T prog){
    Memseg_store(prog, registers[c], registers[a], registers[b]);
}
/********************************************************************************************/
//Function add interprets an addition instruction in the universal machine
static inline void add(uint32_t *registers,unsigned a,unsigned b, unsigned c){
    registers[a] = registers[b] + registers[c];
}
/********************************************************************************************/
//Function multi interprets a multiplication instruction in the universal machine
static inline void mult(uint32_t *registers,unsigned a,unsigned b, unsigned c){
    registers[a] = registers[b] * registers[c];
}
/********************************************************************************************/
//Function divide interprets a division instruction in the universal machine
static inline void divide(uint32_t *registers,unsigned a,unsigned b, unsigned c){
    registers[a] = registers[b] / registers[c];
}
/********************************************************************************************/
//Function nand interprets a bitwase NAND instruction in the universal machine
static inline void nand(uint32_t *registers,unsigned a,unsigned b, unsigned c){
    registers[a] = ~(registers[b] & registers[c]);
}
/********************************************************************************************/
//Function halt interprets a halt instruction in the universal machine
static inline void halt(Memseg_T program){
    Memseg_free(program);
    exit(0);
}
/********************************************************************************************/
//Function map_seg interprets a map segment instruction in the universal machine
static inline void map_seg(uint32_t *registers, unsigned b, unsigned c, Memseg_T program){
    unsigned location = Memseg_map(program, registers[c]);
    registers[b] = location;
}
/********************************************************************************************/
//Function unmap_seg interprets a unmap segment instruction in the universal machine
static inline void unmap_seg(uint32_t *registers, unsigned c, Memseg_T program){
    Memseg_unmap(program, registers[c]);
}
/********************************************************************************************/
//Function ouput interprets an IO output instruction in the universal machine
static inline void output(uint32_t *registers, unsigned c){
    printf("%c", registers[c]);
}
/********************************************************************************************/
//Function input interprets an IO input instruction in the universal machine 
static inline void input(uint32_t *registers, unsigned c){
    registers[c] = getc(stdin);
}
/********************************************************************************************/
//Function load_prog interprets a load program instruction in the universal machine
static inline void load_prog(uint32_t *registers, unsigned b, unsigned c, Memseg_T program, unsigned * idx){
    if (registers[b] != 0){
        Memseg_load_prog(program, registers[b]);
    }
    //Set the program counter index
    *idx = registers[c] - 1;
}
/********************************************************************************************/
//Function load_value interprets a load value instruction in the universal machine
static inline void load_value(uint32_t *registers, unsigned a, uint32_t x){
    registers[a] = x;
}