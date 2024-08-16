/*
 *  FIPS-46-3 compliant 3DES implementation
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <string.h>
#include "stades.h"

/* the eight DES S-boxes */

uint32 SB1[64] =
{
    0x01010400, 0x00000000, 0x00010000, 0x01010404,
    0x01010004, 0x00010404, 0x00000004, 0x00010000,
    0x00000400, 0x01010400, 0x01010404, 0x00000400,
    0x01000404, 0x01010004, 0x01000000, 0x00000004,
    0x00000404, 0x01000400, 0x01000400, 0x00010400,
    0x00010400, 0x01010000, 0x01010000, 0x01000404,
    0x00010004, 0x01000004, 0x01000004, 0x00010004,
    0x00000000, 0x00000404, 0x00010404, 0x01000000,
    0x00010000, 0x01010404, 0x00000004, 0x01010000,
    0x01010400, 0x01000000, 0x01000000, 0x00000400,
    0x01010004, 0x00010000, 0x00010400, 0x01000004,
    0x00000400, 0x00000004, 0x01000404, 0x00010404,
    0x01010404, 0x00010004, 0x01010000, 0x01000404,
    0x01000004, 0x00000404, 0x00010404, 0x01010400,
    0x00000404, 0x01000400, 0x01000400, 0x00000000,
    0x00010004, 0x00010400, 0x00000000, 0x01010004
};

static uint32 SB2[64] =
{
    0x80108020, 0x80008000, 0x00008000, 0x00108020,
    0x00100000, 0x00000020, 0x80100020, 0x80008020,
    0x80000020, 0x80108020, 0x80108000, 0x80000000,
    0x80008000, 0x00100000, 0x00000020, 0x80100020,
    0x00108000, 0x00100020, 0x80008020, 0x00000000,
    0x80000000, 0x00008000, 0x00108020, 0x80100000,
    0x00100020, 0x80000020, 0x00000000, 0x00108000,
    0x00008020, 0x80108000, 0x80100000, 0x00008020,
    0x00000000, 0x00108020, 0x80100020, 0x00100000,
    0x80008020, 0x80100000, 0x80108000, 0x00008000,
    0x80100000, 0x80008000, 0x00000020, 0x80108020,
    0x00108020, 0x00000020, 0x00008000, 0x80000000,
    0x00008020, 0x80108000, 0x00100000, 0x80000020,
    0x00100020, 0x80008020, 0x80000020, 0x00100020,
    0x00108000, 0x00000000, 0x80008000, 0x00008020,
    0x80000000, 0x80100020, 0x80108020, 0x00108000
};

static uint32 SB3[64] =
{
    0x00000208, 0x08020200, 0x00000000, 0x08020008,
    0x08000200, 0x00000000, 0x00020208, 0x08000200,
    0x00020008, 0x08000008, 0x08000008, 0x00020000,
    0x08020208, 0x00020008, 0x08020000, 0x00000208,
    0x08000000, 0x00000008, 0x08020200, 0x00000200,
    0x00020200, 0x08020000, 0x08020008, 0x00020208,
    0x08000208, 0x00020200, 0x00020000, 0x08000208,
    0x00000008, 0x08020208, 0x00000200, 0x08000000,
    0x08020200, 0x08000000, 0x00020008, 0x00000208,
    0x00020000, 0x08020200, 0x08000200, 0x00000000,
    0x00000200, 0x00020008, 0x08020208, 0x08000200,
    0x08000008, 0x00000200, 0x00000000, 0x08020008,
    0x08000208, 0x00020000, 0x08000000, 0x08020208,
    0x00000008, 0x00020208, 0x00020200, 0x08000008,
    0x08020000, 0x08000208, 0x00000208, 0x08020000,
    0x00020208, 0x00000008, 0x08020008, 0x00020200
};

static uint32 SB4[64] =
{
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802080, 0x00800081, 0x00800001, 0x00002001,
    0x00000000, 0x00802000, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00800080, 0x00800001,
    0x00000001, 0x00002000, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002001, 0x00002080,
    0x00800081, 0x00000001, 0x00002080, 0x00800080,
    0x00002000, 0x00802080, 0x00802081, 0x00000081,
    0x00800080, 0x00800001, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00000000, 0x00802000,
    0x00002080, 0x00800080, 0x00800081, 0x00000001,
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802081, 0x00000081, 0x00000001, 0x00002000,
    0x00800001, 0x00002001, 0x00802080, 0x00800081,
    0x00002001, 0x00002080, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002000, 0x00802080
};

static uint32 SB5[64] =
{
    0x00000100, 0x02080100, 0x02080000, 0x42000100,
    0x00080000, 0x00000100, 0x40000000, 0x02080000,
    0x40080100, 0x00080000, 0x02000100, 0x40080100,
    0x42000100, 0x42080000, 0x00080100, 0x40000000,
    0x02000000, 0x40080000, 0x40080000, 0x00000000,
    0x40000100, 0x42080100, 0x42080100, 0x02000100,
    0x42080000, 0x40000100, 0x00000000, 0x42000000,
    0x02080100, 0x02000000, 0x42000000, 0x00080100,
    0x00080000, 0x42000100, 0x00000100, 0x02000000,
    0x40000000, 0x02080000, 0x42000100, 0x40080100,
    0x02000100, 0x40000000, 0x42080000, 0x02080100,
    0x40080100, 0x00000100, 0x02000000, 0x42080000,
    0x42080100, 0x00080100, 0x42000000, 0x42080100,
    0x02080000, 0x00000000, 0x40080000, 0x42000000,
    0x00080100, 0x02000100, 0x40000100, 0x00080000,
    0x00000000, 0x40080000, 0x02080100, 0x40000100
};

static uint32 SB6[64] =
{
    0x20000010, 0x20400000, 0x00004000, 0x20404010,
    0x20400000, 0x00000010, 0x20404010, 0x00400000,
    0x20004000, 0x00404010, 0x00400000, 0x20000010,
    0x00400010, 0x20004000, 0x20000000, 0x00004010,
    0x00000000, 0x00400010, 0x20004010, 0x00004000,
    0x00404000, 0x20004010, 0x00000010, 0x20400010,
    0x20400010, 0x00000000, 0x00404010, 0x20404000,
    0x00004010, 0x00404000, 0x20404000, 0x20000000,
    0x20004000, 0x00000010, 0x20400010, 0x00404000,
    0x20404010, 0x00400000, 0x00004010, 0x20000010,
    0x00400000, 0x20004000, 0x20000000, 0x00004010,
    0x20000010, 0x20404010, 0x00404000, 0x20400000,
    0x00404010, 0x20404000, 0x00000000, 0x20400010,
    0x00000010, 0x00004000, 0x20400000, 0x00404010,
    0x00004000, 0x00400010, 0x20004010, 0x00000000,
    0x20404000, 0x20000000, 0x00400010, 0x20004010
};

static uint32 SB7[64] =
{
    0x00200000, 0x04200002, 0x04000802, 0x00000000,
    0x00000800, 0x04000802, 0x00200802, 0x04200800,
    0x04200802, 0x00200000, 0x00000000, 0x04000002,
    0x00000002, 0x04000000, 0x04200002, 0x00000802,
    0x04000800, 0x00200802, 0x00200002, 0x04000800,
    0x04000002, 0x04200000, 0x04200800, 0x00200002,
    0x04200000, 0x00000800, 0x00000802, 0x04200802,
    0x00200800, 0x00000002, 0x04000000, 0x00200800,
    0x04000000, 0x00200800, 0x00200000, 0x04000802,
    0x04000802, 0x04200002, 0x04200002, 0x00000002,
    0x00200002, 0x04000000, 0x04000800, 0x00200000,
    0x04200800, 0x00000802, 0x00200802, 0x04200800,
    0x00000802, 0x04000002, 0x04200802, 0x04200000,
    0x00200800, 0x00000000, 0x00000002, 0x04200802,
    0x00000000, 0x00200802, 0x04200000, 0x00000800,
    0x04000002, 0x04000800, 0x00000800, 0x00200002
};

static uint32 SB8[64] =
{
    0x10001040, 0x00001000, 0x00040000, 0x10041040,
    0x10000000, 0x10001040, 0x00000040, 0x10000000,
    0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040,
    0x10040000, 0x10000040, 0x10001000, 0x00001040,
    0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040,
    0x10000040, 0x10001000, 0x00041040, 0x00040000,
    0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040,
    0x10001000, 0x00000040, 0x10000040, 0x10040000,
    0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040,
    0x10040000, 0x10001000, 0x10001040, 0x00000000,
    0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000
};

/* PC1: left and right halves bit-swap */

static uint32 LHs[16] =
{
    0x00000000, 0x00000001, 0x00000100, 0x00000101,
    0x00010000, 0x00010001, 0x00010100, 0x00010101,
    0x01000000, 0x01000001, 0x01000100, 0x01000101,
    0x01010000, 0x01010001, 0x01010100, 0x01010101
};

static uint32 RHs[16] =
{
    0x00000000, 0x01000000, 0x00010000, 0x01010000,
    0x00000100, 0x01000100, 0x00010100, 0x01010100,
    0x00000001, 0x01000001, 0x00010001, 0x01010001,
    0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

/* platform-independant 32-bit integer manipulation macros */

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (uint32) (b)[(i)    ] << 24 )       \
        | ( (uint32) (b)[(i) + 1] << 16 )       \
        | ( (uint32) (b)[(i) + 2] <<  8 )       \
        | ( (uint32) (b)[(i) + 3]       );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8) ( (n)       );       \
}

/* Initial Permutation macro */

#define DES_IP(X,Y)                                             \
{                                                               \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    Y = ((Y << 1) | (Y >> 31)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; Y ^= T; X ^= T;                   \
    X = ((X << 1) | (X >> 31)) & 0xFFFFFFFF;                    \
}

/* Final Permutation macro */

#define DES_FP(X,Y)                                             \
{                                                               \
    X = ((X << 31) | (X >> 1)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; X ^= T; Y ^= T;                   \
    Y = ((Y << 31) | (Y >> 1)) & 0xFFFFFFFF;                    \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
}

/* DES round macro */

#define DES_ROUND(X,Y)                          \
{                                               \
    T = *SK++ ^ X;                              \
    Y ^= SB8[ (T      ) & 0x3F ] ^              \
         SB6[ (T >>  8) & 0x3F ] ^              \
         SB4[ (T >> 16) & 0x3F ] ^              \
         SB2[ (T >> 24) & 0x3F ];               \
                                                \
    T = *SK++ ^ ((X << 28) | (X >> 4));         \
    Y ^= SB7[ (T      ) & 0x3F ] ^              \
         SB5[ (T >>  8) & 0x3F ] ^              \
         SB3[ (T >> 16) & 0x3F ] ^              \
         SB1[ (T >> 24) & 0x3F ];               \
}

/* DES key schedule */

static int des_main_ks( uint32 SK[32], uint8 key[8] )
{
    int i;
    uint32 X, Y, T;

    GET_UINT32( X, key, 0 );
    GET_UINT32( Y, key, 4 );

    /* Permuted Choice 1 */

    T =  ((Y >>  4) ^ X) & 0x0F0F0F0F;  X ^= T; Y ^= (T <<  4);
    T =  ((Y      ) ^ X) & 0x10101010;  X ^= T; Y ^= (T      );

    X =   (LHs[ (X      ) & 0xF] << 3) | (LHs[ (X >>  8) & 0xF ] << 2)
        | (LHs[ (X >> 16) & 0xF] << 1) | (LHs[ (X >> 24) & 0xF ]     )
        | (LHs[ (X >>  5) & 0xF] << 7) | (LHs[ (X >> 13) & 0xF ] << 6)
        | (LHs[ (X >> 21) & 0xF] << 5) | (LHs[ (X >> 29) & 0xF ] << 4);

    Y =   (RHs[ (Y >>  1) & 0xF] << 3) | (RHs[ (Y >>  9) & 0xF ] << 2)
        | (RHs[ (Y >> 17) & 0xF] << 1) | (RHs[ (Y >> 25) & 0xF ]     )
        | (RHs[ (Y >>  4) & 0xF] << 7) | (RHs[ (Y >> 12) & 0xF ] << 6)
        | (RHs[ (Y >> 20) & 0xF] << 5) | (RHs[ (Y >> 28) & 0xF ] << 4);

    X &= 0x0FFFFFFF;
    Y &= 0x0FFFFFFF;

    /* calculate subkeys */

    for( i = 0; i < 16; i++ )
    {
        if( i < 2 || i == 8 || i == 15 )
        {
            X = ((X <<  1) | (X >> 27)) & 0x0FFFFFFF;
            Y = ((Y <<  1) | (Y >> 27)) & 0x0FFFFFFF;
        }
        else
        {
            X = ((X <<  2) | (X >> 26)) & 0x0FFFFFFF;
            Y = ((Y <<  2) | (Y >> 26)) & 0x0FFFFFFF;
        }

        *SK++ =   ((X <<  4) & 0x24000000) | ((X << 28) & 0x10000000)
                | ((X << 14) & 0x08000000) | ((X << 18) & 0x02080000)
                | ((X <<  6) & 0x01000000) | ((X <<  9) & 0x00200000)
                | ((X >>  1) & 0x00100000) | ((X << 10) & 0x00040000)
                | ((X <<  2) & 0x00020000) | ((X >> 10) & 0x00010000)
                | ((Y >> 13) & 0x00002000) | ((Y >>  4) & 0x00001000)
                | ((Y <<  6) & 0x00000800) | ((Y >>  1) & 0x00000400)
                | ((Y >> 14) & 0x00000200) | ((Y      ) & 0x00000100)
                | ((Y >>  5) & 0x00000020) | ((Y >> 10) & 0x00000010)
                | ((Y >>  3) & 0x00000008) | ((Y >> 18) & 0x00000004)
                | ((Y >> 26) & 0x00000002) | ((Y >> 24) & 0x00000001);

        *SK++ =   ((X << 15) & 0x20000000) | ((X << 17) & 0x10000000)
                | ((X << 10) & 0x08000000) | ((X << 22) & 0x04000000)
                | ((X >>  2) & 0x02000000) | ((X <<  1) & 0x01000000)
                | ((X << 16) & 0x00200000) | ((X << 11) & 0x00100000)
                | ((X <<  3) & 0x00080000) | ((X >>  6) & 0x00040000)
                | ((X << 15) & 0x00020000) | ((X >>  4) & 0x00010000)
                | ((Y >>  2) & 0x00002000) | ((Y <<  8) & 0x00001000)
                | ((Y >> 14) & 0x00000808) | ((Y >>  9) & 0x00000400)
                | ((Y      ) & 0x00000200) | ((Y <<  7) & 0x00000100)
                | ((Y >>  7) & 0x00000020) | ((Y >>  3) & 0x00000011)
                | ((Y <<  2) & 0x00000004) | ((Y >> 21) & 0x00000002);
    }

    return( 0 );
}

int des_set_key( des_context *ctx, uint8 key[8] )
{
    int i;

    /* setup encryption subkeys */

    des_main_ks( ctx->esk, key );

    /* setup decryption subkeys */

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i    ] = ctx->esk[30 - i];
        ctx->dsk[i + 1] = ctx->esk[31 - i];
    }

    return( 0 );
}

/* DES 64-bit block encryption/decryption */

void des_crypt( uint32 SK[32], uint8 input[8], uint8 output[8] )
{
    uint32 X, Y, T;

    GET_UINT32( X, input, 0 );
    GET_UINT32( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32( Y, output, 0 );
    PUT_UINT32( X, output, 4 );
}

void des_encrypt( des_context *ctx, uint8 input[8], uint8 output[8] )
{
    des_crypt( ctx->esk, input, output );
}

void des_decrypt( des_context *ctx, uint8 input[8], uint8 output[8] )
{
    des_crypt( ctx->dsk, input, output );
}

/* Triple-DES key schedule */

int des3_set_2keys( des3_context *ctx, uint8 key1[8], uint8 key2[8] )
{
    int i;

    des_main_ks( ctx->esk     , key1 );
    des_main_ks( ctx->dsk + 32, key2 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[30 - i];
        ctx->dsk[i +  1] = ctx->esk[31 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->esk[i + 64] = ctx->esk[     i];
        ctx->esk[i + 65] = ctx->esk[ 1 + i];

        ctx->dsk[i + 64] = ctx->dsk[     i];
        ctx->dsk[i + 65] = ctx->dsk[ 1 + i];
    }

    return( 0 );
}

int des3_set_3keys( des3_context *ctx, uint8 key1[8], uint8 key2[8],
                                       uint8 key3[8] )
{
    int i;

    des_main_ks( ctx->esk     , key1 );
    des_main_ks( ctx->dsk + 32, key2 );
    des_main_ks( ctx->esk + 64, key3 );

    for( i = 0; i < 32; i += 2 )
    {
        ctx->dsk[i     ] = ctx->esk[94 - i];
        ctx->dsk[i +  1] = ctx->esk[95 - i];

        ctx->esk[i + 32] = ctx->dsk[62 - i];
        ctx->esk[i + 33] = ctx->dsk[63 - i];

        ctx->dsk[i + 64] = ctx->esk[30 - i];
        ctx->dsk[i + 65] = ctx->esk[31 - i];
    }

    return( 0 );
}

/* Triple-DES 64-bit block encryption/decryption */

static void des3_crypt( uint32 SK[96], uint8 input[8], uint8 output[8] )
{
    uint32 X, Y, T;

    GET_UINT32( X, input, 0 );
    GET_UINT32( Y, input, 4 );

    DES_IP( X, Y );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );
    DES_ROUND( X, Y );  DES_ROUND( Y, X );

    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );
    DES_ROUND( Y, X );  DES_ROUND( X, Y );

    DES_FP( Y, X );

    PUT_UINT32( Y, output, 0 );
    PUT_UINT32( X, output, 4 );
}

void des3_encrypt( des3_context *ctx, uint8 input[8], uint8 output[8] )
{
    des3_crypt( ctx->esk, input, output );
}

void des3_decrypt( des3_context *ctx, uint8 input[8], uint8 output[8] )
{
    des3_crypt( ctx->dsk, input, output );
}


static void DES(des_context *ctx,char *input,char *output,char type)
{
  char In[8],Out[8];

  memcpy(In,input,8);
  memcpy(Out,output,8);

  if(type == ENCRYPT)
	{
	  des_crypt(ctx->esk, In,Out);
	}
  else
	{
	  des_crypt( ctx->dsk, In,Out);
	}
    memcpy(output,Out,8);
}


static void DES3(des3_context *ctx,char *input,char *output,char type)
{
  char In[8],Out[8];

  memcpy(In,input,8);
  memcpy(Out,output,8);

  if(type == ENCRYPT)
	{
	  des3_crypt(ctx->esk, In,Out);
	}
  else
	{
	  des3_crypt( ctx->dsk, In,Out);
	}
   memcpy(output,Out,8);
}


/*******************************************************************/
/*
  函 数 名 称:	RunDes
  功 能 描 述：	执行DES算法对文本加解密
  参 数 说 明：	bType	:类型：加密ENCRYPT，解密DECRYPT
				bMode	:模式：ECB,CBC
				In		:待加密串指针
				Out		:待输出串指针
				datalen	:待加密串的长度，同时Out的缓冲区大小应大于或者等于datalen
				Key		:密钥(可为8位,16位,24位)支持3密钥
				keylen	:密钥长度，多出24位部分将被自动裁减

  返回值 说明：	char	:是否加密成功
*/
/*******************************************************************/
char RunDes(char bType,char bMode,char* In,char* Out,unsigned int datalen,const char* Key,const unsigned char keylen)
{
	unsigned int i,j;
	des_context  ctx;
	des3_context ctx3;
	unsigned  char nKey;
	unsigned  char dkey1[8];
	unsigned  char dkey2[8];
	unsigned  char dkey3[8];

	  //判断输入合法性
	  if(!(In && Out && Key && datalen && keylen>=8))
		  return false;
	  //只处理8的整数倍，不足长度自己填充
	  if(datalen & 0x00000007)
		  return false;

      nKey	=	(keylen>>3)>3 ? 3: (keylen>>3);
	  switch(nKey)
	  	{
         case 1:
		 	 memcpy(dkey1,Key,8);
		 	 des_set_key(&ctx,dkey1);
		 	 break;
		 case 2:
			memcpy(dkey1,Key,8);
			memcpy(dkey2,Key+8,8);
			des3_set_2keys(&ctx3,dkey1,dkey2);
			break;
		 case 3:
			memcpy(dkey1,Key,8);
			memcpy(dkey2,Key+8,8);
			memcpy(dkey3,Key+16,8);
			des3_set_3keys(&ctx3,dkey1,dkey2,dkey3);
			 break;
		  default:
		  	return false;
			break;
	    }



	if(bMode == ECB)	//ECB模式
	{
		if(nKey	==	1)	//单Key
		{
			for(i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
			{
				//DES(Out,In,&m_SubKey[0],bType);

                 DES(&ctx,In,Out,bType);
			}
		}
		else			//3DES 3Key
		{
			for( i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
			{
                DES3(&ctx3,In,Out,bType);
			}
		}
	}
/*
	else				//CBC模式
	{
		char	cvec[8]	=	"";	//扭转向量
		char	cvin[8]	=	""; //中间变量

		if(nKey == 1)	//单Key
		{
			for(int i=0,j=datalen>>3; i<j; ++i,Out+=8,In+=8)
			{
				if(bType	==	ENCRYPT)
				{
					for(int j=0;j<8;++j)		//将输入与扭转变量异或
					{
						cvin[j]	=	In[j] ^ cvec[j];
					}
				};
				else
				{
					memcpy(cvin,In,8);
				}

				DES(Out,cvin,&m_SubKey[0],bType);

				if(bType	==	ENCRYPT)
				{
					memcpy(cvec,Out,8);			//将输出设定为扭转变量
				}
				else
				{
					for(int j=0;j<8;++j)		//将输出与扭转变量异或
					{
						Out[j]	=	Out[j] ^ cvec[j];
					}
					memcpy(cvec,cvin,8);			//将输入设定为扭转变量
				}
			}
		}
		else
		if(nKey == 2)	//3DES CBC 2Key
		{
			for(int i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
			{
				if(bType	==	ENCRYPT)
				{
					for(int j=0;j<8;++j)		//将输入与扭转变量异或
					{
						cvin[j]	=	In[j] ^ cvec[j];
					}
				}
				else
				{
					memcpy(cvin,In,8);
				}

				DES(Out,cvin,&m_SubKey[0],bType);
				DES(Out,Out,&m_SubKey[1],!bType);
				DES(Out,Out,&m_SubKey[0],bType);

				if(bType	==	ENCRYPT)
				{
					memcpy(cvec,Out,8);			//将输出设定为扭转变量
				}
				else
				{
					for(int j=0;j<8;++j)		//将输出与扭转变量异或
					{
						Out[j]	=	Out[j] ^ cvec[j];
					}
					memcpy(cvec,cvin,8);			//将输入设定为扭转变量
				}
			}
		}
		else			//3DES CBC 3Key
		{
			for(int i=0,j=datalen>>3;i<j;++i,Out+=8,In+=8)
			{
				if(bType	==	ENCRYPT)
				{
					for(int j=0;j<8;++j)		//将输入与扭转变量异或
					{
						cvin[j]	=	In[j] ^ cvec[j];
					}
				}
				else
				{
					memcpy(cvin,In,8);
				}

				DES(Out,cvin,&m_SubKey[bType ? 2 : 0],bType);
				DES(Out,Out,&m_SubKey[1],!bType);
				DES(Out,Out,&m_SubKey[bType ? 0 : 2],bType);

				if(bType	==	ENCRYPT)
				{
					memcpy(cvec,Out,8);			//将输出设定为扭转变量
				}
				else
				{
					for(int j=0;j<8;++j)		//将输出与扭转变量异或
					{
						Out[j]	=	Out[j] ^ cvec[j];
					}
					memcpy(cvec,cvin,8);			//将输入设定为扭转变量
				}
			}
		}
	}
*/
	return true;
}




/*******************************************************************/
/*
  函 数 名 称:	RunPad
  功 能 描 述：	根据协议对加密前的数据进行填充
  参 数 说 明：	bType	:类型：PAD类型
				In		:数据串指针
				Out		:填充输出串指针
				datalen	:数据的长度
				padlen	:(in,out)输出buffer的长度，填充后的长度

  返回值 说明：	char	:是否填充成功
*/
/*******************************************************************/
char RunPad(char nType,const char* In,unsigned int datalen,char* Out,unsigned int* padlen)
{
	unsigned int res = (datalen & 0x00000007);

	if(*padlen< (datalen+8-res))
	{
		return false;
	}
	else
	{
		*padlen	= (datalen+8-res);
		memcpy(Out,In,datalen);
	}


	if(nType == PAD_ISO_1)
	{
		memset(Out+datalen,0x00,(8-res));
	}
	else if(nType == PAD_ISO_2)
	{
		memset(Out+datalen,0x80,1);
		memset(Out+datalen,0x00,(7-res));
	}
	else if(nType == PAD_PKCS_7)
	{
		memset(Out+datalen,(8-res),(8-res));
	}
	else if(nType == PAD_PBOC)
	{
		memset(Out+datalen,0x20,(8-res));
	}
	else
	{
		return false;
	}

	return true;
}

//按位取反操作
void Not(char *Out, const char *In, int len)
{
	int i;

	 for(i=0; i<len; ++i)
        Out[i]= ~In[i];
}

//按位异或操作
void XOR(char *Out, const char *In, int len)
{
	int i;
	for(i=0;i<len;i++)
		Out[i]^=In[i];
}


