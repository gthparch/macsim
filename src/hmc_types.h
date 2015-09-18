#ifndef HMC_TYPES_H
#define HMC_TYPES_H

typedef enum HMC_Type_enum
{
    HMC_NONE=0,
    HMC_CAS_equal_16B=1,
    HMC_CAS_zero_16B=2,
    HMC_CAS_greater_16B=3,
    HMC_CAS_less_16B=4,
    HMC_ADD_16B=5,
    HMC_ADD_8B=6,
    HMC_ADD_DUAL=7,
    HMC_SWAP=8,
    HMC_BIT_WR=9,
    HMC_AND=10,
    HMC_NAND=11,
    HMC_OR=12,
    HMC_XOR=13,
    NUM_HMC_TYPES=14
} HMC_Type;

#endif