#include "bitpack.h"
#include "except.h"
#include <stdio.h>
#include <inttypes.h>

Except_T Bitpack_Overflow = { "Overflow packing bits\n" };

uint64_t maxWidth = 64;

static inline uint64_t lshift( uint64_t word, unsigned n ){
    if( n >= maxWidth ){
        return 0;
    } else {
        return word << n;
    }
}

static inline uint64_t rshift_logical( uint64_t word, unsigned n ){
    if( n >= maxWidth ){
        return 0;
    } else {
        return word >> n;
    }
}

static inline int64_t rshift_arith( int64_t word, unsigned n ){
    if( n >= maxWidth ){
        return 0;
    } else {
        return word >> n;
    }
}

bool Bitpack_fitsu(uint64_t n, unsigned width){
    uint64_t upperBound = lshift( (uint64_t)1, width )-1;
//    printf("Upper: %" PRIu64 "\n", upperBound);
    if( n <= upperBound ){
        return true;
    } else {
        return false;
    }
}

bool Bitpack_fitss( int64_t n, unsigned width){
    int64_t lowerBound = lshift( (int64_t)-1, (width - 1) );
//    printf("lower: %" PRId64 "\n", lowerBound);
    int64_t upperBound = rshift_logical( ~(uint64_t)0, (maxWidth - width + 1) );
//    printf("Upper: %" PRIu64 "\n", upperBound);
    if( n >= lowerBound && n<= upperBound ){
        return true;
    } else {
        return false;
    }
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){
    if( width + lsb > maxWidth ){
        fprintf(stderr, "Width invalid in getu\n");
        return word;
    }
    // removes bits to the left of the field
    word = lshift(word, maxWidth-width-lsb);
    //moves field to end of word
    word = rshift_logical(word, maxWidth-width);
    return word;
}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){
    if( width > maxWidth || width + lsb > maxWidth ){
        fprintf(stderr, "Width invalid in gets\n");
        return word;
    }
    //removes data to the left of the field
    word = lshift(word, maxWidth-width-lsb);

    //maintaining the sign bit, pushes the field to the end of the word
    int64_t wordCpy = rshift_arith(word, maxWidth-width);
    return wordCpy;
}

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value){
    if( width > maxWidth || width+lsb>maxWidth ){
        fprintf(stderr, "Input invalid in newu\n");
        return word;
    } else if ( !Bitpack_fitsu(value, width) ) {
        RAISE( Bitpack_Overflow );
    } else {
        //everything is good, update the field

        //field full of zeros
        uint64_t fieldMask = 0;
        //field full of ones
        fieldMask = ~fieldMask;
        //shift those ones to be in the position of the field
        fieldMask = lshift(fieldMask, maxWidth-width);
        fieldMask = rshift_logical(fieldMask, maxWidth-width);
        fieldMask = lshift(fieldMask, lsb);
        fieldMask = ~fieldMask;
        //zero out field
        word = word & fieldMask;

        //center and insert field
        value = lshift(value, lsb);
        word = word | value;
        return word;
    }
    return word;
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  int64_t value){
    if( width > maxWidth || width+lsb>maxWidth ){
        fprintf(stderr, "Input invalid in news\n");
        return word;
    } else if ( !Bitpack_fitss(value, width) ) {
        RAISE( Bitpack_Overflow );
    } else {

        //field full of zeros
        uint64_t fieldMask = 0;
        //fill with ones
        fieldMask = ~fieldMask;

        //center ones over field location
        fieldMask = lshift(fieldMask, maxWidth-width);
        fieldMask = rshift_logical(fieldMask, maxWidth-width);
        fieldMask = lshift(fieldMask, lsb);

        //zero out field location
        fieldMask = ~fieldMask;
        word = word & fieldMask;

        //remove leading sign bits
        value = lshift(value, maxWidth-width);
        value = rshift_logical(value, maxWidth-width-lsb);

        //insert field into word
        word = word | value;
        return word;
    }
    return word;
}