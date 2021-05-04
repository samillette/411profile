//Timothy Colaneri
//Universal Machine interpreter interface

/*****************************************************************/
#include"um_mem.h"
/*****************************************************************/
extern void Interp_prog(Memseg_T program, unsigned *registers);
//Function Interpret_prog acts as the main interpretation driver
//for a universal machine program. This function takes in a 
//Memseg_t representing the program in memory and a pointer
//to a integer array representing the machines registers
//and then interprets the passed in universal machine program.
/*****************************************************************/