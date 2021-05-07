//Timothy Colaneri
//bitpack.c implementation

/*********************************************************************/
//The bitpack.c module contains functions for inserting and extracting
//bit fields to and from bit words

/*********************************************************************/
#include <stdbool.h> //Boolean functions specified in handout
#include <stdint.h>  //For the (u)nit64_t data types
#include <stdlib.h>  //exit function
#include <stdio.h>   //testing

#include "except.h" //Bitpack overflow exception
#include "bitpack.h"//Files's own header
#include "assert.h" //Assertions

/*************************************************************PROTOYPES*/
// Logical shift right on bit word
static inline uint64_t shift_right_logic(uint64_t word, unsigned shift);
// Arithmatic shift right on bit word
static inline int64_t shift_right_arith(int64_t word, unsigned shift);
// Shift left on bit word
static inline uint64_t shift_left(uint64_t word,unsigned shift);

/***************************************************CONSTANTS+EXCEPTIONS*/
Except_T Bitpack_Overflow = { "Overflow packing bits" };//Specification
static const unsigned MAXWIDTH = 64; //Maximum number of bits in a word

/**************************************************UNSIGNED*BITFIT*CHECK*/
//This function takes in a value n and determiens if it will fit into
//width bits. Function return a stdbool true if it fits.
bool Bitpack_fitsu(uint64_t n, unsigned width){

    //Check if passed in width is greater then max width
    if (width > MAXWIDTH){
        return false;
    }

    //Push a bit into a word 'width' elements in
    uint64_t compVal = 1;
    compVal = (compVal << width);

    //if n < compVal, then if will fit
    //else it will not fit
    if (n < compVal){
        return true;
    }
    return false;
}
/***************************************************SIGNED*BITFIT*CHECK*/
//This function takes in a signed value n and determines if it will fit
//into 'width' bits. Function return stdbool true  if it fits.
bool Bitpack_fitss( int64_t n, unsigned width){

    //Check if passed in width is greater then max width
    if (width > MAXWIDTH){
        return false;
    }

    //We need to handle negative and positive differently here
    //If its negative, we want the complement incremented by 1, 
    //Else we can just transfer the bits over by casting
    uint64_t compVal = 0;

    if (n < 0){
        compVal = ~n;
    }
    else {
        compVal = (uint64_t)n;
    }

    //Process the output via the unsigned bit fit check, width is
    //decremented to compensate for negtive msb in signed representation
    return Bitpack_fitsu(compVal, (--width));
}

/***********************************************UNSIGNED*BIT*RETRIEVAL*/
//This function takes in a bit word, bit width, and least significant
//bit location. The bits starting from the lsb until the (lsb+width)
//are then returned from this function in a unsigned type uint64_t
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){

    //Assert the passed in values are within range
    assert((width + lsb) <= MAXWIDTH);

    // Field initially used as a mask
    //Assemble a mask of just the field that we want to retrieve
    uint64_t field = ~0;
    field = shift_left(field, (MAXWIDTH - width));
    field = shift_right_logic(field, (MAXWIDTH - (width + lsb)));

    //Grab the field from the word with the mask
    field = (word & field);

    //Shift desired field to front of word for retrieval
    field = shift_right_logic(field,lsb);

    return field;
}

/***************************************************SIGNED*BIT*RETRIEVAL*/
//This function takes in a bit word, bit width, and least significant
//bit location. The bits starting from the lsb until the (lsb+width)
//are then returned from this function in a signed type int64_t
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){

    //Assert the passed in values are within range
    assert((width + lsb) <= MAXWIDTH);

    uint64_t mask = ~0;
    int64_t field;

    //Assemble the mask of the field we want to retrieve
    mask = shift_left(mask, (MAXWIDTH - width));
    mask = shift_right_logic(mask, (MAXWIDTH - (width + lsb)));

    //Grab the field and shift it into place for retrieval
    mask = (word & mask);
    mask = shift_left(mask, (MAXWIDTH - (width + lsb)));
    field = shift_right_arith(mask,(MAXWIDTH - width));

    return field;
}
/*****************************************************UNSIGNED*BIT*UPDATE*/
//This function takes in a bit word, bit width, least significant bit
//location, and a unsigned value. This function then attempts to update
//the location in the bit word from the lsb to the (width + lsb) with
//the unsigned value. If the value does not fit a Bitpack_Overflow 
//exception is raised. The updated word is then returned to the caller.
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value){
    
    //Assert the passed in values are within range
    assert((width + lsb) <= MAXWIDTH);

    uint64_t mask;

    //If the value we want to to place into bit fits into the number of bits,
    //then process it
    //Else raise exception
    if (Bitpack_fitsu(value, width) == true){

        //Assemble the mask of the field we want to update
        mask = ~0;
        mask = shift_right_logic(mask,MAXWIDTH - width);
        mask = shift_left(mask, lsb);

        //Get the complement of the mask so we can grab the rest of the word
        //That is not changed
        mask = ~mask;

        //(word & mask) The unchanged part of the word
        //(value << lsb) the update value shifted into place
        return ((word & mask) | (value << lsb));
    }
    else{
        RAISE(Bitpack_Overflow);
        exit(1);
    }
}

/*********************************************************SIGNED*BIT*UPDATE*/
//This function takes in a bit word, bit width, least significant bit
//location, and a signed value. This function then attempts to update
//the location in the bit word from the lsb to the (width + lsb) with
//the signed value. If the value does not fit a Bitpack_Overflow 
//exception is raised. The updated word is then returned to the caller.
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  int64_t value){

    //Assert the passed in values are within range
    assert((width + lsb) <= MAXWIDTH);

    uint64_t mask;

    //If the value we want to to place into bit fits into the number of bits,
    //then process it
    //Else raise exception
    if (Bitpack_fitss(value, width) == true){

        //Assemble a mask of the field we want to update
        mask = ~0;
        mask = shift_right_logic(mask,MAXWIDTH - width);
        mask = shift_left(mask, lsb);

        //Get the complement of the mask so we can grab the rest of the word
        //That is not changed
        mask = ~mask;

        //Clean the value up to make sure it fits in range.
        //Ex. -1 in uint64_t = 0xfffffffffffffff
        //Suppose we are only adding recording 2-bits into the word, this check
        //esures we dont copy the whole thing.
        value = shift_left(value,(MAXWIDTH - (width)));
        value = shift_right_logic(value,(MAXWIDTH - width));

        //(word & mask) The unchanged part of the word
        //(value << lsb) the update value shifted into place
        return ((word & mask) | (value << lsb));
        //(void)word;
    }
    else{
        RAISE(Bitpack_Overflow);
        exit(1);
    }
}
/************************************************************************/
// Bit Shifting Helpers
/*************************************************************LEFT*SHIFT*/
//Shift left has only one version, Shifts the word left by "shift" spaces
static inline uint64_t shift_left(uint64_t word,unsigned shift){

    //Assert that we are not asked to shift more bits then exist
    assert(!(shift > 64));

    //If we are shifting all of bits off the word, just return 0
    //Else we preform the shift.
    if (shift == 64){
        return 0;
    }
    else{
        return word << shift;
    }
}
/******************************************************ARITH*RIGHT*SHIFT*/
//Arithmatic version of shift right, handles signed values
//Shifts the word "shift" spaces to the right
static inline int64_t shift_right_arith(int64_t word, unsigned shift){

    //Assert that we are not asked to shift more bits then exist
    assert(!(shift > MAXWIDTH));

    //If we are shifting all the bits off
    if (shift == MAXWIDTH){

        //If the input value was negative, return all 1's
        //Else return all 0's
        if (word < 0){
            return ~0;
        }
        else{
            return 0;
        }
    }
    else{
        //Else we just preform the right shift
        return word >> shift;
    }
}
/*******************************************************LOGIC*RIGHT*SHIFT*/
//Logical version of shift right, handles unsigned values
//Shifts the word "shift" spaces to the right
static inline uint64_t shift_right_logic(uint64_t word, unsigned shift){

    //Assert that we are not asked to shift more bits then exist
    assert(!(shift > MAXWIDTH));

    //If we are shifting all of bits off the word, just return 0
    //Else we preform the shift.
    if (shift == MAXWIDTH){
        return 0;
    }
    else{
        return word >> shift;
    }
}
/************************************************************************/