/**
 * \file chacha20.c
 *
 * \brief ChaCha20 cipher.
 *
 * \author Daniel King <damaki.gh@gmail.com>
 *
 *  Copyright (C) 2006-2016, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedcrypto/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_CHACHA20_C)

#include "chacha20.h"
#include "platform_util.h"

#include <stddef.h>
#include <string.h>

#if defined(MBEDTLS_SELF_TEST)
#if defined(MBEDTLS_PLATFORM_C)
#include "platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */
#endif /* MBEDTLS_SELF_TEST */

#if !defined(MBEDTLS_CHACHA20_ALT)

#if ( defined(__ARMCC_VERSION) || defined(_MSC_VER) ) && \
    !defined(inline) && !defined(__cplusplus)
#define inline __inline
#endif

#define BYTES_TO_U32_LE( data, offset )                           \
    ( (uint32_t) data[offset]                                     \
          | (uint32_t) ( (uint32_t) data[( offset ) + 1] << 8 )   \
          | (uint32_t) ( (uint32_t) data[( offset ) + 2] << 16 )  \
          | (uint32_t) ( (uint32_t) data[( offset ) + 3] << 24 )  \
    )

#define ROTL32( value, amount ) \
        ( (uint32_t) ( value << amount ) | ( value >> ( 32 - amount ) ) )

#define CHACHA20_CTR_INDEX ( 12U )

#define CHACHA20_BLOCK_SIZE_BYTES ( 4U * 16U )

/**
 * \brief           ChaCha20 quarter round operation.
 *
 *                  The quarter round is defined as follows (from RFC 7539):
 *                      1.  a += b; d ^= a; d <<<= 16;
 *                      2.  c += d; b ^= c; b <<<= 12;
 *                      3.  a += b; d ^= a; d <<<= 8;
 *                      4.  c += d; b ^= c; b <<<= 7;
 *
 * \param state     ChaCha20 state to modify.
 * \param a         The index of 'a' in the state.
 * \param b         The index of 'b' in the state.
 * \param c         The index of 'c' in the state.
 * \param d         The index of 'd' in the state.
 */
static inline void chacha20_quarter_round( uint32_t state[16],
                                           size_t a,
                                           size_t b,
                                           size_t c,
                                           size_t d )
{
    /* a += b; d ^= a; d <<<= 16; */
    state[a] += state[b];
    state[d] ^= state[a];
    state[d] = ROTL32( state[d], 16 );

    /* c += d; b ^= c; b <<<= 12 */
    state[c] += state[d];
    state[b] ^= state[c];
    state[b] = ROTL32( state[b], 12 );

    /* a += b; d ^= a; d <<<= 8; */
    state[a] += state[b];
    state[d] ^= state[a];
    state[d] = ROTL32( state[d], 8 );

    /* c += d; b ^= c; b <<<= 7; */
    state[c] += state[d];
    state[b] ^= state[c];
    state[b] = ROTL32( state[b], 7 );
}

/**
 * \brief           Perform the ChaCha20 inner block operation.
 *
 *                  This function performs two rounds: the column round and the
 *                  diagonal round.
 *
 * \param state     The ChaCha20 state to update.
 */
static void chacha20_inner_block( uint32_t state[16] )
{
    chacha20_quarter_round( state, 0, 4, 8,  12 );
    chacha20_quarter_round( state, 1, 5, 9,  13 );
    chacha20_quarter_round( state, 2, 6, 10, 14 );
    chacha20_quarter_round( state, 3, 7, 11, 15 );

    chacha20_quarter_round( state, 0, 5, 10, 15 );
    chacha20_quarter_round( state, 1, 6, 11, 12 );
    chacha20_quarter_round( state, 2, 7, 8,  13 );
    chacha20_quarter_round( state, 3, 4, 9,  14 );
}

/**
 * \brief               Generates a keystream block.
 *
 * \param initial_state The initial ChaCha20 state (key, nonce, counter).
 * \param keystream     Generated keystream bytes are written to this buffer.
 */
static void chacha20_block( const uint32_t initial_state[16],
                            unsigned char keystream[64] )
{
    uint32_t working_state[16];
    size_t i;

    memcpy( working_state,
            initial_state,
            CHACHA20_BLOCK_SIZE_BYTES );

    for( i = 0U; i < 10U; i++ )
        chacha20_inner_block( working_state );

    working_state[ 0] += initial_state[ 0];
    working_state[ 1] += initial_state[ 1];
    working_state[ 2] += initial_state[ 2];
    working_state[ 3] += initial_state[ 3];
    working_state[ 4] += initial_state[ 4];
    working_state[ 5] += initial_state[ 5];
    working_state[ 6] += initial_state[ 6];
    working_state[ 7] += initial_state[ 7];
    working_state[ 8] += initial_state[ 8];
    working_state[ 9] += initial_state[ 9];
    working_state[10] += initial_state[10];
    working_state[11] += initial_state[11];
    working_state[12] += initial_state[12];
    working_state[13] += initial_state[13];
    working_state[14] += initial_state[14];
    working_state[15] += initial_state[15];

    for( i = 0U; i < 16; i++ )
    {
        size_t offset = i * 4U;

        keystream[offset     ] = (unsigned char)( working_state[i]       );
        keystream[offset + 1U] = (unsigned char)( working_state[i] >>  8 );
        keystream[offset + 2U] = (unsigned char)( working_state[i] >> 16 );
        keystream[offset + 3U] = (unsigned char)( working_state[i] >> 24 );
    }

    mbedtls_platform_zeroize( working_state, sizeof( working_state ) );
}

void mbedtls_chacha20_init( mbedtls_chacha20_context *ctx )
{
    if( ctx != NULL )
    {
        mbedtls_platform_zeroize( ctx->state, sizeof( ctx->state ) );
        mbedtls_platform_zeroize( ctx->keystream8, sizeof( ctx->keystream8 ) );

        /* Initially, there's no keystream bytes available */
        ctx->keystream_bytes_used = CHACHA20_BLOCK_SIZE_BYTES;
    }
}

void mbedtls_chacha20_free( mbedtls_chacha20_context *ctx )
{
    if( ctx != NULL )
    {
        mbedtls_platform_zeroize( ctx, sizeof( mbedtls_chacha20_context ) );
    }
}

int mbedtls_chacha20_setkey( mbedtls_chacha20_context *ctx,
                            const unsigned char key[32] )
{
    if( ( ctx == NULL ) || ( key == NULL ) )
    {
        return( MBEDTLS_ERR_CHACHA20_BAD_INPUT_DATA );
    }

    /* ChaCha20 constants - the string "expand 32-byte k" */
    ctx->state[0] = 0x61707865;
    ctx->state[1] = 0x3320646e;
    ctx->state[2] = 0x79622d32;
    ctx->state[3] = 0x6b206574;

    /* Set key */
    ctx->state[4]  = BYTES_TO_U32_LE( key, 0 );
    ctx->state[5]  = BYTES_TO_U32_LE( key, 4 );
    ctx->state[6]  = BYTES_TO_U32_LE( key, 8 );
    ctx->state[7]  = BYTES_TO_U32_LE( key, 12 );
    ctx->state[8]  = BYTES_TO_U32_LE( key, 16 );
    ctx->state[9]  = BYTES_TO_U32_LE( key, 20 );
    ctx->state[10] = BYTES_TO_U32_LE( key, 24 );
    ctx->state[11] = BYTES_TO_U32_LE( key, 28 );

    return( 0 );
}

int mbedtls_chacha20_starts( mbedtls_chacha20_context* ctx,
                             const unsigned char nonce[12],
                             uint32_t counter )
{
    if( ( ctx == NULL ) || ( nonce == NULL ) )
    {
        return( MBEDTLS_ERR_CHACHA20_BAD_INPUT_DATA );
    }

    /* Counter */
    ctx->state[12] = counter;

    /* Nonce */
    ctx->state[13] = BYTES_TO_U32_LE( nonce, 0 );
    ctx->state[14] = BYTES_TO_U32_LE( nonce, 4 );
    ctx->state[15] = BYTES_TO_U32_LE( nonce, 8 );

    mbedtls_platform_zeroize( ctx->keystream8, sizeof( ctx->keystream8 ) );

    /* Initially, there's no keystream bytes available */
    ctx->keystream_bytes_used = CHACHA20_BLOCK_SIZE_BYTES;

    return( 0 );
}

int mbedtls_chacha20_update( mbedtls_chacha20_context *ctx,
                              size_t size,
                              const unsigned char *input,
                              unsigned char *output )
{
    size_t offset = 0U;
    size_t i;

    if( ctx == NULL )
    {
        return( MBEDTLS_ERR_CHACHA20_BAD_INPUT_DATA );
    }
    else if( ( size > 0U ) && ( ( input == NULL ) || ( output == NULL ) ) )
    {
        /* input and output pointers are allowed to be NULL only if size == 0 */
        return( MBEDTLS_ERR_CHACHA20_BAD_INPUT_DATA );
    }

    /* Use leftover keystream bytes, if available */
    while( size > 0U && ctx->keystream_bytes_used < CHACHA20_BLOCK_SIZE_BYTES )
    {
        output[offset] = input[offset]
                       ^ ctx->keystream8[ctx->keystream_bytes_used];

        ctx->keystream_bytes_used++;
        offset++;
        size--;
    }

    /* Process full blocks */
    while( size >= CHACHA20_BLOCK_SIZE_BYTES )
    {
        /* Generate new keystream block and increment counter */
        chacha20_block( ctx->state, ctx->keystream8 );
        ctx->state[CHACHA20_CTR_INDEX]++;

        for( i = 0U; i < 64U; i += 8U )
        {
            output[offset + i  ] = input[offset + i  ] ^ ctx->keystream8[i  ];
            output[offset + i+1] = input[offset + i+1] ^ ctx->keystream8[i+1];
            output[offset + i+2] = input[offset + i+2] ^ ctx->keystream8[i+2];
            output[offset + i+3] = input[offset + i+3] ^ ctx->keystream8[i+3];
            output[offset + i+4] = input[offset + i+4] ^ ctx->keystream8[i+4];
            output[offset + i+5] = input[offset + i+5] ^ ctx->keystream8[i+5];
            output[offset + i+6] = input[offset + i+6] ^ ctx->keystream8[i+6];
            output[offset + i+7] = input[offset + i+7] ^ ctx->keystream8[i+7];
        }

        offset += CHACHA20_BLOCK_SIZE_BYTES;
        size   -= CHACHA20_BLOCK_SIZE_BYTES;
    }

    /* Last (partial) block */
    if( size > 0U )
    {
        /* Generate new keystream block and increment counter */
        chacha20_block( ctx->state, ctx->keystream8 );
        ctx->state[CHACHA20_CTR_INDEX]++;

        for( i = 0U; i < size; i++)
        {
            output[offset + i] = input[offset + i] ^ ctx->keystream8[i];
        }

        ctx->keystream_bytes_used = size;

    }

    return( 0 );
}

int mbedtls_chacha20_crypt( const unsigned char key[32],
                            const unsigned char nonce[12],
                            uint32_t counter,
                            size_t data_len,
                            const unsigned char* input,
                            unsigned char* output )
{
    mbedtls_chacha20_context ctx;
    int ret;

    mbedtls_chacha20_init( &ctx );

    ret = mbedtls_chacha20_setkey( &ctx, key );
    if( ret != 0 )
        goto cleanup;

    ret = mbedtls_chacha20_starts( &ctx, nonce, counter );
    if( ret != 0 )
        goto cleanup;

    ret = mbedtls_chacha20_update( &ctx, data_len, input, output );

cleanup:
    mbedtls_chacha20_free( &ctx );
    return( ret );
}

#endif /* !MBEDTLS_CHACHA20_ALT */

#if defined(MBEDTLS_SELF_TEST)

static const unsigned char test_keys[2][32] =
{
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    }
};

static const unsigned char test_nonces[2][12] =
{
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02
    }
};

static const uint32_t test_counters[2] =
{
    0U,
    1U
};

static const unsigned char test_input[2][375] =
{
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    },
    {
        0x41, 0x6e, 0x79, 0x20, 0x73, 0x75, 0x62, 0x6d,
        0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x74,
        0x6f, 0x20, 0x74, 0x68, 0x65, 0x20, 0x49, 0x45,
        0x54, 0x46, 0x20, 0x69, 0x6e, 0x74, 0x65, 0x6e,
        0x64, 0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x74,
        0x68, 0x65, 0x20, 0x43, 0x6f, 0x6e, 0x74, 0x72,
        0x69, 0x62, 0x75, 0x74, 0x6f, 0x72, 0x20, 0x66,
        0x6f, 0x72, 0x20, 0x70, 0x75, 0x62, 0x6c, 0x69,
        0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x61,
        0x73, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6f, 0x72,
        0x20, 0x70, 0x61, 0x72, 0x74, 0x20, 0x6f, 0x66,
        0x20, 0x61, 0x6e, 0x20, 0x49, 0x45, 0x54, 0x46,
        0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65,
        0x74, 0x2d, 0x44, 0x72, 0x61, 0x66, 0x74, 0x20,
        0x6f, 0x72, 0x20, 0x52, 0x46, 0x43, 0x20, 0x61,
        0x6e, 0x64, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x73,
        0x74, 0x61, 0x74, 0x65, 0x6d, 0x65, 0x6e, 0x74,
        0x20, 0x6d, 0x61, 0x64, 0x65, 0x20, 0x77, 0x69,
        0x74, 0x68, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65,
        0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x78, 0x74,
        0x20, 0x6f, 0x66, 0x20, 0x61, 0x6e, 0x20, 0x49,
        0x45, 0x54, 0x46, 0x20, 0x61, 0x63, 0x74, 0x69,
        0x76, 0x69, 0x74, 0x79, 0x20, 0x69, 0x73, 0x20,
        0x63, 0x6f, 0x6e, 0x73, 0x69, 0x64, 0x65, 0x72,
        0x65, 0x64, 0x20, 0x61, 0x6e, 0x20, 0x22, 0x49,
        0x45, 0x54, 0x46, 0x20, 0x43, 0x6f, 0x6e, 0x74,
        0x72, 0x69, 0x62, 0x75, 0x74, 0x69, 0x6f, 0x6e,
        0x22, 0x2e, 0x20, 0x53, 0x75, 0x63, 0x68, 0x20,
        0x73, 0x74, 0x61, 0x74, 0x65, 0x6d, 0x65, 0x6e,
        0x74, 0x73, 0x20, 0x69, 0x6e, 0x63, 0x6c, 0x75,
        0x64, 0x65, 0x20, 0x6f, 0x72, 0x61, 0x6c, 0x20,
        0x73, 0x74, 0x61, 0x74, 0x65, 0x6d, 0x65, 0x6e,
        0x74, 0x73, 0x20, 0x69, 0x6e, 0x20, 0x49, 0x45,
        0x54, 0x46, 0x20, 0x73, 0x65, 0x73, 0x73, 0x69,
        0x6f, 0x6e, 0x73, 0x2c, 0x20, 0x61, 0x73, 0x20,
        0x77, 0x65, 0x6c, 0x6c, 0x20, 0x61, 0x73, 0x20,
        0x77, 0x72, 0x69, 0x74, 0x74, 0x65, 0x6e, 0x20,
        0x61, 0x6e, 0x64, 0x20, 0x65, 0x6c, 0x65, 0x63,
        0x74, 0x72, 0x6f, 0x6e, 0x69, 0x63, 0x20, 0x63,
        0x6f, 0x6d, 0x6d, 0x75, 0x6e, 0x69, 0x63, 0x61,
        0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20, 0x6d, 0x61,
        0x64, 0x65, 0x20, 0x61, 0x74, 0x20, 0x61, 0x6e,
        0x79, 0x20, 0x74, 0x69, 0x6d, 0x65, 0x20, 0x6f,
        0x72, 0x20, 0x70, 0x6c, 0x61, 0x63, 0x65, 0x2c,
        0x20, 0x77, 0x68, 0x69, 0x63, 0x68, 0x20, 0x61,
        0x72, 0x65, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65,
        0x73, 0x73, 0x65, 0x64, 0x20, 0x74, 0x6f
    }
};

static const unsigned char test_output[2][375] =
{
    {
        0x76, 0xb8, 0xe0, 0xad, 0xa0, 0xf1, 0x3d, 0x90,
        0x40, 0x5d, 0x6a, 0xe5, 0x53, 0x86, 0xbd, 0x28,
        0xbd, 0xd2, 0x19, 0xb8, 0xa0, 0x8d, 0xed, 0x1a,
        0xa8, 0x36, 0xef, 0xcc, 0x8b, 0x77, 0x0d, 0xc7,
        0xda, 0x41, 0x59, 0x7c, 0x51, 0x57, 0x48, 0x8d,
        0x77, 0x24, 0xe0, 0x3f, 0xb8, 0xd8, 0x4a, 0x37,
        0x6a, 0x43, 0xb8, 0xf4, 0x15, 0x18, 0xa1, 0x1c,
        0xc3, 0x87, 0xb6, 0x69, 0xb2, 0xee, 0x65, 0x86
    },
    {
        0xa3, 0xfb, 0xf0, 0x7d, 0xf3, 0xfa, 0x2f, 0xde,
        0x4f, 0x37, 0x6c, 0xa2, 0x3e, 0x82, 0x73, 0x70,
        0x41, 0x60, 0x5d, 0x9f, 0x4f, 0x4f, 0x57, 0xbd,
        0x8c, 0xff, 0x2c, 0x1d, 0x4b, 0x79, 0x55, 0xec,
        0x2a, 0x97, 0x94, 0x8b, 0xd3, 0x72, 0x29, 0x15,
        0xc8, 0xf3, 0xd3, 0x37, 0xf7, 0xd3, 0x70, 0x05,
        0x0e, 0x9e, 0x96, 0xd6, 0x47, 0xb7, 0xc3, 0x9f,
        0x56, 0xe0, 0x31, 0xca, 0x5e, 0xb6, 0x25, 0x0d,
        0x40, 0x42, 0xe0, 0x27, 0x85, 0xec, 0xec, 0xfa,
        0x4b, 0x4b, 0xb5, 0xe8, 0xea, 0xd0, 0x44, 0x0e,
        0x20, 0xb6, 0xe8, 0xdb, 0x09, 0xd8, 0x81, 0xa7,
        0xc6, 0x13, 0x2f, 0x42, 0x0e, 0x52, 0x79, 0x50,
        0x42, 0xbd, 0xfa, 0x77, 0x73, 0xd8, 0xa9, 0x05,
        0x14, 0x47, 0xb3, 0x29, 0x1c, 0xe1, 0x41, 0x1c,
        0x68, 0x04, 0x65, 0x55, 0x2a, 0xa6, 0xc4, 0x05,
        0xb7, 0x76, 0x4d, 0x5e, 0x87, 0xbe, 0xa8, 0x5a,
        0xd0, 0x0f, 0x84, 0x49, 0xed, 0x8f, 0x72, 0xd0,
        0xd6, 0x62, 0xab, 0x05, 0x26, 0x91, 0xca, 0x66,
        0x42, 0x4b, 0xc8, 0x6d, 0x2d, 0xf8, 0x0e, 0xa4,
        0x1f, 0x43, 0xab, 0xf9, 0x37, 0xd3, 0x25, 0x9d,
        0xc4, 0xb2, 0xd0, 0xdf, 0xb4, 0x8a, 0x6c, 0x91,
        0x39, 0xdd, 0xd7, 0xf7, 0x69, 0x66, 0xe9, 0x28,
        0xe6, 0x35, 0x55, 0x3b, 0xa7, 0x6c, 0x5c, 0x87,
        0x9d, 0x7b, 0x35, 0xd4, 0x9e, 0xb2, 0xe6, 0x2b,
        0x08, 0x71, 0xcd, 0xac, 0x63, 0x89, 0x39, 0xe2,
        0x5e, 0x8a, 0x1e, 0x0e, 0xf9, 0xd5, 0x28, 0x0f,
        0xa8, 0xca, 0x32, 0x8b, 0x35, 0x1c, 0x3c, 0x76,
        0x59, 0x89, 0xcb, 0xcf, 0x3d, 0xaa, 0x8b, 0x6c,
        0xcc, 0x3a, 0xaf, 0x9f, 0x39, 0x79, 0xc9, 0x2b,
        0x37, 0x20, 0xfc, 0x88, 0xdc, 0x95, 0xed, 0x84,
        0xa1, 0xbe, 0x05, 0x9c, 0x64, 0x99, 0xb9, 0xfd,
        0xa2, 0x36, 0xe7, 0xe8, 0x18, 0xb0, 0x4b, 0x0b,
        0xc3, 0x9c, 0x1e, 0x87, 0x6b, 0x19, 0x3b, 0xfe,
        0x55, 0x69, 0x75, 0x3f, 0x88, 0x12, 0x8c, 0xc0,
        0x8a, 0xaa, 0x9b, 0x63, 0xd1, 0xa1, 0x6f, 0x80,
        0xef, 0x25, 0x54, 0xd7, 0x18, 0x9c, 0x41, 0x1f,
        0x58, 0x69, 0xca, 0x52, 0xc5, 0xb8, 0x3f, 0xa3,
        0x6f, 0xf2, 0x16, 0xb9, 0xc1, 0xd3, 0x00, 0x62,
        0xbe, 0xbc, 0xfd, 0x2d, 0xc5, 0xbc, 0xe0, 0x91,
        0x19, 0x34, 0xfd, 0xa7, 0x9a, 0x86, 0xf6, 0xe6,
        0x98, 0xce, 0xd7, 0x59, 0xc3, 0xff, 0x9b, 0x64,
        0x77, 0x33, 0x8f, 0x3d, 0xa4, 0xf9, 0xcd, 0x85,
        0x14, 0xea, 0x99, 0x82, 0xcc, 0xaf, 0xb3, 0x41,
        0xb2, 0x38, 0x4d, 0xd9, 0x02, 0xf3, 0xd1, 0xab,
        0x7a, 0xc6, 0x1d, 0xd2, 0x9c, 0x6f, 0x21, 0xba,
        0x5b, 0x86, 0x2f, 0x37, 0x30, 0xe3, 0x7c, 0xfd,
        0xc4, 0xfd, 0x80, 0x6c, 0x22, 0xf2, 0x21
    }
};

static const size_t test_lengths[2] =
{
    64U,
    375U
};

#define ASSERT( cond, args )            \
    do                                  \
    {                                   \
        if( ! ( cond ) )                \
        {                               \
            if( verbose != 0 )          \
                mbedtls_printf args;    \
                                        \
            return( -1 );               \
        }                               \
    }                                   \
    while( 0 )

int mbedtls_chacha20_self_test( int verbose )
{
    unsigned char output[381];
    unsigned i;
    int ret;

    for( i = 0U; i < 2U; i++ )
    {
        if( verbose != 0 )
            mbedtls_printf( "  ChaCha20 test %u ", i );

        ret = mbedtls_chacha20_crypt( test_keys[i],
                                      test_nonces[i],
                                      test_counters[i],
                                      test_lengths[i],
                                      test_input[i],
                                      output );

        ASSERT( 0 == ret, ( "error code: %i\n", ret ) );

        ASSERT( 0 == memcmp( output, test_output[i], test_lengths[i] ),
                ( "failed (output)\n" ) );

        if( verbose != 0 )
            mbedtls_printf( "passed\n" );
    }

    if( verbose != 0 )
        mbedtls_printf( "\n" );

    return( 0 );
}

#endif /* MBEDTLS_SELF_TEST */

#endif /* !MBEDTLS_CHACHA20_C */
