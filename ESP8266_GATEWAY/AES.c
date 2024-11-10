//
// Created by abdelrhman mattar on 2/24/2024.
//

#include "AES.h"
#include <stdio.h>
//Rcon
const BYTE rcon[11] = {0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
//shift row and inv shift row
const BYTE  shiftRow[16]={0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
const BYTE  invShift[16] = {0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3};

//const BYTE  mix_col[16]={2,3,1,1,1,2,3,1,1,1,2,3,3,1,1,2};
//mix column write in column major order
const BYTE  mix_col[16]={2,1,1,3,
                            3,2,1,1,
                            1,3,2,1,
                            1,1,3,2};
//invMix[16] = {14,11,13,9,9,14,11,13,13,9,14,11,11,13,9,14};
const BYTE invMix[16]  ={ 14,9,13,11,
                          11,14,9,13,
                          13,11,14,9,
                          9,13,11,14};
//sbox and inv_sbox
const BYTE sBox[256] =
{0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};

const BYTE inv_sBox[256] =
        {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
        0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
        0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
        0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
        0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
        0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
        0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
        0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
        0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
        0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
        0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D};


//print hex
/* print_hex
 * print the array in hex format
 * arr: the array to be printed
 * size: the size of the array
 */
void print_hex(BYTE *arr, BYTE size) {
    for (BYTE i = 0; i < size; i++) {
        printf("%02x ", arr[i]);
    }
    printf("\n");
}

//key expansion
//key format is 16 bytes K0 K1 K2 K3 K4 K5 K6 K7 K8 K9 K10 K11 K12 K13 K14 K15
/* W0 = K0 K1 K2 K3
 * W1 = K4 K5 K6 K7
 * W2 = K8 K9 K10 K11
 * W3 = K12 K13 K14 K15
*/
//Used in KeyExpansion, substitute word(4byte) by byte
//sub_word function takes a word and substitute each byte in the word by the sbox
WORD sub_word(WORD word) {
    WORD res = 0;
    for (BYTE i = 0; i < 4; i++) {
        res = res << 8;
        res = res | sBox[(word >> (24 - i * 8)) & 0xff];
    }
    return res;
}
//key expansion
void key_expansion(WORD * res , const BYTE * key) {
    WORD W[4];
    W[0] = (key[0] << 24) | (key[1] << 16) | (key[2] << 8) | key[3];
    W[1] = (key[4] << 24) | (key[5] << 16) | (key[6] << 8) | key[7];
    W[2] = (key[8] << 24) | (key[9] << 16) | (key[10] << 8) | key[11];
    W[3] = (key[12] << 24) | (key[13] << 16) | (key[14] << 8) | key[15];

    for(BYTE i=0 ; i<4 ; i++){
        res[i] = W[i];
    }
    for(int i=4;i<44;i++)
    {
        WORD temp = res[i-1];
        if(i%4==0)
        {
            //Rotation for KeyExpansion
            WORD temp2 = temp>>24;
            temp = (temp<<8) | temp2;
            //Substitution for KeyExpansion
            temp = sub_word(temp);
            //Rcon for KeyExpansion
            temp = temp ^ ((WORD)rcon[i/4]<<24);
        }
        res[i] = res[i-4] ^ temp;
        //printf("W[%d] = %08x\n",i,res[i]);
    }
    //swap all bytes in word from 0,1,2,3 -> 3,2,1,0
    for(BYTE i=0;i<44;i++)
    {
        res[i]=((res[i]&0xFF)<<24)|((res[i]&0xFF00)<< 8)|((res[i]>>8)&0xFF00) | ((res[i] >> 24) & 0xFF);
    }
}

//add round key function takes the data and the key and add them together
/* add_round_key
 * data: the data to be added
 * key: the key to be added
 */
void add_round_key(BYTE * data , const BYTE * key)
{
    for(BYTE i=0;i<16;i++)
    {
        data[i] = data[i] ^ key[i];
    }
}
// general SUB_BYTES function takes the data and the mode and substitute the bytes
/* sub_bytes
 * data: the data to be substituted
 * mode: the mode of the substitution
 */
void sub_bytes(BYTE *data , AES_MODE mode)
{
    BYTE *sbox_ptr = NULL;
    if(mode == AES_ENCRYPT)
    {
        sbox_ptr = (BYTE*)sBox;
    }
    else
    {
        sbox_ptr = (BYTE*)inv_sBox;
    }
    for(BYTE i=0;i<16;i++)
    {
        data[i] = sbox_ptr[data[i]];
    }
}
// general shift row function takes the data and the mode and shift the rows
/* shift_rows
 * data: the data to be shifted
 * mode: the mode of the shift
 */
void shift_rows(BYTE *data , AES_MODE mode)
{
    BYTE temp[16];
    BYTE *shift_ptr = NULL;
    for(BYTE i=0;i<16;i++)
    {
        temp[i] = data[i];
    }
    if(mode == AES_ENCRYPT)
    {
        shift_ptr = (BYTE*)shiftRow;
    }
    else
    {
        shift_ptr = (BYTE*)invShift;
    }
    for(BYTE i=0;i<16;i++)
    {
        data[i] = temp[shift_ptr[i]];
    }
}
//galios multiplication
/* galoisMul
 * num: the number to be multiplied
 * by: the number to multiply by
 * return: the result of the multiplication
 */
BYTE galoisMul(BYTE num , BYTE by)
{
    BYTE res = 0;
    for(BYTE i=0;i<8;i++)
    {
        if(by &0x01)
        {
            res = res ^ num;
        }
        BYTE last_bit = (num & 0x80)>>7;
        num = num << 1;
        if(last_bit)
        {
            num = num ^ 0x1b;
        }
        by = by >> 1;
        if(by == 0)
        {
            break;
        }
    }
    return res;
}

//mix column
/* mix_column
 * data: the data to be mixed
 * mode: the mode of the mix column
 */
void mix_column(BYTE *data, AES_MODE mode)
{
    BYTE *mix_col_ptr = NULL;
    if(mode == AES_ENCRYPT)
    {
        mix_col_ptr = (BYTE*)mix_col;
    }
    else
    {
        mix_col_ptr = (BYTE*)invMix;
    }
    for (BYTE i = 0; i < 4; i++) {
        BYTE temp[4] = {0, 0, 0, 0};
        for (BYTE j = 0; j < 4; j++) {
            for (BYTE k = 0; k < 4; k++) {
                //col = i * 4 + k for data
                //row = k * 4 + j for mix_col
                temp[j] = temp[j] ^ galoisMul(data[i * 4 + k], mix_col_ptr[k * 4 + j]);
            }
        }
        for (BYTE j = 0; j < 4; j++) {
            data[i * 4 + j] = temp[j];
        }
    }
}

//aes128 Encryption
void aes128_enc(BYTE * res , const BYTE * data , WORD * key_rounds)
{
    //Initial Round
    for(BYTE i=0;i<16;i++)
    {
        res[i]=data[i];
    }
    add_round_key((BYTE*)res,(BYTE*)key_rounds);
    //Rounds
    for(BYTE i=0;i<9;i++)
    {
        //SUB_BYTES
        sub_bytes(res,AES_ENCRYPT);
        //SHIFT_ROWS
        shift_rows(res,AES_ENCRYPT);
        //MIX_COLUMNS
        mix_column(res,AES_ENCRYPT);
        //ADD_ROUND_KEY
        add_round_key((BYTE*)res,(BYTE*)(key_rounds+(i+1)*4));
    }
    //Final Round
    sub_bytes(res,AES_ENCRYPT);
    shift_rows(res,AES_ENCRYPT);
    add_round_key((BYTE*)res,(BYTE*)(key_rounds+40));
}

//aes128 Decryption
void aes128_dec(BYTE * res , const BYTE * data , WORD * key_rounds)
{
    //Initial Round
    for(BYTE i=0;i<16;i++)
    {
        res[i]=data[i];
    }
    add_round_key((BYTE*)res,(BYTE*)(key_rounds+40));
    //Rounds
    for(BYTE i=0;i<9;i++)
    {
        //SHIFT_ROWS
        shift_rows(res,AES_DECRYPT);
        //SUB_BYTES
        sub_bytes(res,AES_DECRYPT);
        //ADD_ROUND_KEY
        add_round_key((BYTE*)res,(BYTE*)(key_rounds+(9-i)*4));
        //MIX_COLUMNS
        mix_column(res,AES_DECRYPT);
    }
    //Final Round
    shift_rows(res,AES_DECRYPT);
    sub_bytes(res,AES_DECRYPT);
    add_round_key((BYTE*)res,(BYTE*)key_rounds);
}
//make general function for decryption and encryption and add the mode as a parameter
void AES128(BYTE * res , const BYTE * data , WORD * key_rounds , AES_MODE mode)
{
    BYTE start = 0;
    BYTE end = 10;
    if(mode == AES_DECRYPT)
    {
        start = 10;
        end = 0;
    }
    for(BYTE i=0;i<16;i++)
    {
        res[i]=data[i];
    }
    add_round_key((BYTE*)res,(BYTE*)(key_rounds+(start*4)));
    start = start + mode;
    for(BYTE i=start;i!=end;i+=mode)
    {
        sub_bytes(res,mode);
        shift_rows(res, mode);
        mix_column(res, mode);
        add_round_key((BYTE*)res,(BYTE*)(key_rounds+(i*4)));
    }
    sub_bytes(res,mode);
    shift_rows(res, mode);
    add_round_key((BYTE*)res,(BYTE*)(key_rounds+(end*4)));
}