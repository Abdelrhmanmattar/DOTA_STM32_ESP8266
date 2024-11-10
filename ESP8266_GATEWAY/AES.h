//
// Created by abdelrhman mattar on 2/24/2024.
//

#ifndef UNTITLED34_AES_H
#define UNTITLED34_AES_H

#ifdef __cplusplus
extern "C"{
#endif

//byte
typedef unsigned char BYTE;
//word 32 bit
typedef unsigned long int WORD;

//enum for mode encryption or decryption
typedef enum {
    AES_ENCRYPT =  1,
    AES_DECRYPT = -1
} AES_MODE;

//prototypes for the functions
void print_hex(BYTE *arr, BYTE size);
void key_expansion(WORD * res , const BYTE * key);
void aes128_enc(BYTE * res , const BYTE * data , WORD * key_rounds);
void aes128_dec(BYTE * res , const BYTE * data , WORD * key_rounds);
void AES128(BYTE * res , const BYTE * data , WORD * key_rounds , AES_MODE mode);



#ifdef __cplusplus
}
#endif

#endif //UNTITLED34_AES_H
