//Timothy Colaneri
//Universal Machine Memory Segment Implementation

/**********************************************************/
#include<stdlib.h>
#include<stdio.h>   //Output/input instructions
#include"stack.h"   //Used to handle unmapped segments
#include<seq.h>     //Used to represent memory segment
#include<array.h>   //
#include<assert.h>  //Assertions
#include"um_mem.h"  //Own header
/**********************************************************/
//A Memseg_T structure is a representation of a universal
//machine memory segment. The sequence segemnts holds each
//segmented piece of the memory. The Queue unmapped will
//contain a list of each memory space which has been 
//released and is availible for reuse.
//At any point in execution of a program, the Memseg_T's
//segments sequence will contain all memory segments
//currently stored in the program
//At any point in execution of a program, the unmapped
//stack will contain a list of all of the memory spaces
//which have been unmapped and not reused.
#define T Memseg_T
struct T {
    Seq_T segments;
    Stack_T unmapped;
};
/**********************************************************/
//Memseg_init creates a new Memseg_T memory segment,
//initializes all of its values to empty, and returns the
//new memory segment.
extern T Memseg_init(){

    //Create the new memory space
    T memSpace = malloc(sizeof(*memSpace));
    assert(memSpace);

    //Initialize the memory structs members  
    memSpace->segments = Seq_new(16);
    memSpace->unmapped = Stack_new();

    //Assert that the memory space was availible
    assert(memSpace->segments);
    assert(memSpace->unmapped);    
    return memSpace;
}
/**********************************************************/
//Memseg_store stores a new value 'elem' into the memory segment
//located at 'seg'. It is placed into this word(memory segment)
// at offset 'offset' 
extern void Memseg_store(T memSpace,uint32_t elem,int seg, int offset){
    Array_T memSeg = Seq_get(memSpace->segments,seg);
    uint32_t * val = (uint32_t*)Array_get(memSeg,offset);
    *val = elem;
}
/**********************************************************/
//Memseg_load loads a value from the memory space 'memSpace'
//found in the segment 'seg' at offset 'offset'. This 
//function then returns that value
extern uint32_t Memseg_load(T memSpace,int seg,int offset){
    Array_T memSeg = Seq_get(memSpace->segments,seg);
    return *(uint32_t*)Array_get(memSeg,offset);
}
/**********************************************************/
//Memseg_map creates a new segment with a number of words
//equal to 'size'. A pointer to this new segment is then
//returned from the function
extern uint32_t Memseg_map(T memSpace, int size){

    Array_T newSeg = Array_new(size,sizeof(uint32_t));

    assert(newSeg);

    //Determine if any memory spaces have been previously
    //freed, if so use the freed up memory space
    //else, add a new memory space
    if (Stack_empty(memSpace->unmapped)){

        Seq_addhi(memSpace->segments, (void*)newSeg);
        return (uint32_t)((Seq_length(memSpace->segments)-1));
    }
    else{

        uint64_t index = (uint64_t)(uint32_t*)Stack_pop(memSpace->unmapped);
        Seq_put(memSpace->segments, index, (void*)newSeg);
        return (uint32_t)index;
    }
}
/**********************************************************/
//Memseg_unmap unmaps the memery segment 'seg' found in the
//memory space memSpace.
extern void Memseg_unmap(T memSpace, uint32_t seg){
    Array_T oldSeg = Seq_put(memSpace->segments,seg,NULL);
    Array_free(&oldSeg);
    Stack_push(memSpace->unmapped,(void*)(uint64_t)seg);
}
/**********************************************************/
//Memseg_load_prog duplicates the memory segment found in
//'memSpace' at 'seg'. This segment is then loaded into 
//'memSpace' at postion 0, and the former code at postion 0
//is abandoned.
extern void Memseg_load_prog(T memSpace,int seg){
    Array_T segment = Seq_get(memSpace->segments, seg);
    Array_T newSeg = Array_copy(segment,Array_length(segment));
    Array_T oldSeg = Seq_put(memSpace->segments,0,newSeg);
    Array_free(&oldSeg);
}
/**********************************************************/
//Memspace_free frees the memory space used up by the passed
//in 'memSpace' struct.
extern void Memseg_free(T memSpace){

    int totalSpace = Seq_length(memSpace->segments);
    Array_T segment;

    //Iterate through all of the segment in the memory space
    //and free them.
    for (int i = 0;i < totalSpace;++i){
        segment = Seq_remhi(memSpace->segments);
        if (segment != NULL){
            Array_free(&segment);
        }
    }

    //Free the actual memory space stucture
    Seq_free(&(memSpace->segments));
    Stack_free(&(memSpace->unmapped));
    free(memSpace);
}
/**********************************************************/