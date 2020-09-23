/*
 **************************  SOUND LOCALIZATOR V.3 *************************
 * LINK:   http://coolarduino.wordpress.com/2012/10/01/sound-localization/ ‎
 *
 * Created for  Arduino LEONARDO board: Anatoly Kuzmenko 16 October 2012 
 *                                 k_anatoly@hotmail.com
 *
 * SOFTWARE COMPILES USING Arduino 1.0.1 IDE (Tested on Linux OS only).
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * Copyright (C) 2012 Anatoly Kuzmenko.
 * All Rights Reserved.
 ********************************************************************************************
 */

#include <avr/pgmspace.h>

#define FFT_SIZE 128   // Number of samples; FFT Size
#define SMPLTIME 39    // 50 ksps; 25 ksps per channel; 12.5 kHz Audio.
#define SRVOTIME 20000 // :2 = 20.000 msec, or 50 Hz
#define CHANNELS 2

#define LOG_2_FFT 7 /* log2 FFT_SIZE */
#define MIRROR FFT_SIZE / 2
#define NWAVE 256 /* full length of Sinewave[] */

volatile int16_t x_r[CHANNELS][FFT_SIZE]; // MEM: 512 Bytes

int16_t f_r[FFT_SIZE]; // MEM: 256 Bytes
int16_t f_i[FFT_SIZE]; // MEM: 256 Bytes

int16_t faza[2][MIRROR]; // DEBUG

volatile uint8_t flagSamp = 0;
uint8_t directin = 0; // X - Y Dimension.
uint8_t indxUpdt = 0;
const uint8_t srvoAccl[2] = {2, 1};     // Filter's responsivnes adjustment ( 1 is slowest).
int16_t lokaVect[2][MIRROR] = {{0, 0}}; // "Rolling Filter", Extremely efficient "Median" LPF.
int16_t srvoPosn[2] = {1350, 1800};     // Keep current Position.
const uint16_t updtRate = 8;            // 1 Update servo position per 8 frames (4 per dimension).

const int16_t Hamming[128] PROGMEM = {
    // elements 1 & 128 "trimmed" to "0"
    +0, +20, +21, +21, +22, +24, +25, +27, +29, +31, +34, +37, +40, +44, +47, +51,
    +55, +59, +64, +68, +73, +78, +83, +88, +94, +99, +105, +110, +116, +122, +128, +133,
    +139, +145, +151, +157, +162, +168, +174, +179, +184, +190, +195, +200, +205, +210, +214, +218,
    +223, +226, +230, +234, +237, +240, +243, +245, +247, +249, +251, +253, +254, +255, +255, +255,
    +255, +255, +255, +254, +253, +251, +249, +247, +245, +243, +240, +237, +234, +230, +226, +223,
    +218, +214, +210, +205, +200, +195, +190, +184, +179, +174, +168, +162, +157, +151, +145, +139,
    +133, +128, +122, +116, +110, +105, +99, +94, +88, +83, +78, +73, +68, +64, +59, +55,
    +51, +47, +44, +40, +37, +34, +31, +29, +27, +25, +24, +22, +21, +21, +20, +0};

const int16_t aTangentc[32][32] PROGMEM = {

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {512, 256, 151, 105, 80, 64, 54, 46, 41, 36, 32, 30, 27, 25, 23, 22, 20, 19, 18, 17, 16, 16, 15, 14, 14, 13, 13, 12, 12, 11, 11, 11},
    {512, 361, 256, 192, 151, 124, 105, 91, 80, 71, 64, 59, 54, 50, 46, 43, 41, 38, 36, 34, 32, 31, 30, 28, 27, 26, 25, 24, 23, 22, 22, 21},
    {512, 407, 320, 256, 210, 176, 151, 132, 117, 105, 95, 87, 80, 74, 69, 64, 60, 57, 54, 51, 49, 46, 44, 42, 41, 39, 37, 36, 35, 34, 32, 31},
    {512, 432, 361, 302, 256, 220, 192, 169, 151, 136, 124, 114, 105, 97, 91, 85, 80, 75, 71, 68, 64, 61, 59, 56, 54, 52, 50, 48, 46, 45, 43, 42},
    {512, 448, 388, 336, 292, 256, 226, 202, 182, 165, 151, 139, 129, 120, 112, 105, 99, 93, 88, 84, 80, 76, 73, 70, 67, 64, 62, 60, 58, 56, 54, 52},
    {512, 458, 407, 361, 320, 286, 256, 231, 210, 192, 176, 163, 151, 141, 132, 124, 117, 111, 105, 100, 95, 91, 87, 83, 80, 77, 74, 71, 69, 66, 64, 62},
    {512, 466, 421, 380, 343, 310, 281, 256, 234, 215, 199, 185, 172, 161, 151, 142, 134, 127, 121, 115, 110, 105, 100, 96, 93, 89, 86, 83, 80, 77, 75, 72},
    {512, 471, 432, 395, 361, 330, 302, 278, 256, 237, 220, 205, 192, 180, 169, 160, 151, 143, 136, 130, 124, 119, 114, 109, 105, 101, 97, 94, 91, 88, 85, 82},
    {512, 476, 441, 407, 376, 347, 320, 297, 275, 256, 239, 224, 210, 197, 186, 176, 167, 159, 151, 144, 138, 132, 127, 122, 117, 113, 109, 105, 101, 98, 95, 92},
    {512, 480, 448, 417, 388, 361, 336, 313, 292, 273, 256, 240, 226, 214, 202, 192, 182, 173, 165, 158, 151, 145, 139, 134, 129, 124, 120, 116, 112, 108, 105, 102},
    {512, 482, 453, 425, 398, 373, 349, 327, 307, 288, 272, 256, 242, 229, 217, 206, 196, 187, 179, 171, 164, 157, 151, 145, 140, 135, 130, 126, 122, 118, 115, 111},
    {512, 485, 458, 432, 407, 383, 361, 340, 320, 302, 286, 270, 256, 243, 231, 220, 210, 200, 192, 184, 176, 169, 163, 157, 151, 146, 141, 136, 132, 128, 124, 120},
    {512, 487, 462, 438, 415, 392, 371, 351, 332, 315, 298, 283, 269, 256, 244, 233, 222, 213, 204, 196, 188, 181, 174, 168, 162, 156, 151, 146, 142, 137, 133, 129},
    {512, 489, 466, 443, 421, 400, 380, 361, 343, 326, 310, 295, 281, 268, 256, 245, 234, 225, 215, 207, 199, 192, 185, 178, 172, 166, 161, 156, 151, 147, 142, 138},
    {512, 490, 469, 448, 427, 407, 388, 370, 352, 336, 320, 306, 292, 279, 267, 256, 245, 236, 226, 218, 210, 202, 195, 188, 182, 176, 171, 165, 160, 156, 151, 147},
    {512, 492, 471, 452, 432, 413, 395, 378, 361, 345, 330, 316, 302, 290, 278, 267, 256, 246, 237, 228, 220, 212, 205, 198, 192, 186, 180, 174, 169, 164, 160, 155},
    {512, 493, 474, 455, 437, 419, 401, 385, 369, 353, 339, 325, 312, 299, 287, 276, 266, 256, 247, 238, 230, 222, 214, 207, 201, 195, 189, 183, 178, 173, 168, 163},
    {512, 494, 476, 458, 441, 424, 407, 391, 376, 361, 347, 333, 320, 308, 297, 286, 275, 265, 256, 247, 239, 231, 224, 216, 210, 203, 197, 192, 186, 181, 176, 171},
    {512, 495, 478, 461, 444, 428, 412, 397, 382, 368, 354, 341, 328, 316, 305, 294, 284, 274, 265, 256, 248, 240, 232, 225, 218, 212, 206, 200, 194, 189, 184, 179},
    {512, 496, 480, 463, 448, 432, 417, 402, 388, 374, 361, 348, 336, 324, 313, 302, 292, 282, 273, 264, 256, 248, 240, 233, 226, 220, 214, 208, 202, 197, 192, 187},
    {512, 496, 481, 466, 451, 436, 421, 407, 393, 380, 367, 355, 343, 331, 320, 310, 300, 290, 281, 272, 264, 256, 248, 241, 234, 228, 221, 215, 210, 204, 199, 194},
    {512, 497, 482, 468, 453, 439, 425, 412, 398, 385, 373, 361, 349, 338, 327, 317, 307, 298, 288, 280, 272, 264, 256, 249, 242, 235, 229, 223, 217, 212, 206, 201},
    {512, 498, 484, 470, 456, 442, 429, 416, 403, 390, 378, 367, 355, 344, 334, 324, 314, 305, 296, 287, 279, 271, 263, 256, 249, 242, 236, 230, 224, 219, 213, 208},
    {512, 498, 485, 471, 458, 445, 432, 419, 407, 395, 383, 372, 361, 350, 340, 330, 320, 311, 302, 294, 286, 278, 270, 263, 256, 249, 243, 237, 231, 225, 220, 215},
    {512, 499, 486, 473, 460, 448, 435, 423, 411, 399, 388, 377, 366, 356, 346, 336, 326, 317, 309, 300, 292, 284, 277, 270, 263, 256, 250, 243, 238, 232, 226, 221},
    {512, 499, 487, 475, 462, 450, 438, 426, 415, 403, 392, 382, 371, 361, 351, 341, 332, 323, 315, 306, 298, 291, 283, 276, 269, 262, 256, 250, 244, 238, 233, 227},
    {512, 500, 488, 476, 464, 452, 441, 429, 418, 407, 396, 386, 376, 366, 356, 347, 338, 329, 320, 312, 304, 297, 289, 282, 275, 269, 262, 256, 250, 244, 239, 234},
    {512, 500, 489, 477, 466, 454, 443, 432, 421, 411, 400, 390, 380, 370, 361, 352, 343, 334, 326, 318, 310, 302, 295, 288, 281, 274, 268, 262, 256, 250, 245, 239},
    {512, 501, 490, 478, 467, 456, 446, 435, 424, 414, 404, 394, 384, 375, 365, 356, 348, 339, 331, 323, 315, 308, 300, 293, 287, 280, 274, 268, 262, 256, 250, 245},
    {512, 501, 490, 480, 469, 458, 448, 437, 427, 417, 407, 397, 388, 379, 370, 361, 352, 344, 336, 328, 320, 313, 306, 299, 292, 286, 279, 273, 267, 262, 256, 251},
    {512, 501, 491, 481, 470, 460, 450, 440, 430, 420, 410, 401, 392, 383, 374, 365, 357, 349, 341, 333, 325, 318, 311, 304, 297, 291, 285, 278, 273, 267, 261, 256}};

const int16_t Sinewave[NWAVE] PROGMEM = {

    +0, +6, +13, +19, +25, +31, +37, +44, +50, +56, +62, +68, +74, +80, +86, +92,
    +98, +103, +109, +115, +120, +126, +131, +136, +142, +147, +152, +157, +162, +167, +171, +176,
    +180, +185, +189, +193, +197, +201, +205, +208, +212, +215, +219, +222, +225, +228, +231, +233,
    +236, +238, +240, +242, +244, +246, +247, +249, +250, +251, +252, +253, +254, +254, +255, +255,
    +255, +255, +255, +254, +254, +253, +252, +251, +250, +249, +247, +246, +244, +242, +240, +238,
    +236, +233, +231, +228, +225, +222, +219, +215, +212, +208, +205, +201, +197, +193, +189, +185,
    +180, +176, +171, +167, +162, +157, +152, +147, +142, +136, +131, +126, +120, +115, +109, +103,
    +98, +92, +86, +80, +74, +68, +62, +56, +50, +44, +37, +31, +25, +19, +13, +6,
    +0, -6, -13, -19, -25, -31, -38, -44, -50, -56, -62, -68, -74, -80, -86, -92,
    -98, -104, -109, -115, -121, -126, -132, -137, -142, -147, -152, -157, -162, -167, -172, -177,
    -181, -185, -190, -194, -198, -202, -206, -209, -213, -216, -220, -223, -226, -229, -231, -234,
    -237, -239, -241, -243, -245, -247, -248, -250, -251, -252, -253, -254, -255, -255, -256, -256,
    -256, -256, -256, -255, -255, -254, -253, -252, -251, -250, -248, -247, -245, -243, -241, -239,
    -237, -234, -231, -229, -226, -223, -220, -216, -213, -209, -206, -202, -198, -194, -190, -185,
    -181, -177, -172, -167, -162, -157, -152, -147, -142, -137, -132, -126, -121, -115, -109, -104,
    -98, -92, -86, -80, -74, -68, -62, -56, -50, -44, -38, -31, -25, -19, -13, -6};

#define mult_shf_s16x16(a, b)                         \
    ({                                                \
        int prod, val1 = a, val2 = b;                 \
        __asm__ __volatile__(\ 
"muls %B1, %B2	\n\t"                       \
                             "mov %B0, r0    \n\t"    \
                             \ 
"mul %A1, %A2   \n\t"                       \
                             \ 
"mov %A0, r1    \n\t"                       \
                             \ 
"mulsu %B1, %A2	\n\t"                       \
                             \ 
"add %A0, r0    \n\t"                       \
                             \ 
"adc %B0, r1    \n\t"                       \
                             \ 
"mulsu %B2, %A1	\n\t"                       \
                             \ 
"add %A0, r0    \n\t"                       \
                             \ 
"adc %B0, r1    \n\t"                       \
                             \ 
"clr r1         \n\t"                       \
                             \ 
: "=&d"(prod)                                         \
                             : "a"(val1), "a"(val2)); \
        prod;                                         \
    })

static inline void mult_shf_I(int c, int s, int x, int y, int &u, int &v) __attribute__((always_inline));
static inline void mult_shf_I(int c, int s, int x, int y, int &u, int &v)
{
    u = (mult_shf_s16x16(x, c) - mult_shf_s16x16(y, s));
    v = (mult_shf_s16x16(y, c) + mult_shf_s16x16(x, s)); // Hardcoded >>8 bits, use with 8-bits Sinewave ONLY.
}

static inline void sum_dif_I(int a, int b, int &s, int &d) __attribute__((always_inline));
static inline void sum_dif_I(int a, int b, int &s, int &d)
{
    s = (a + b);
    d = (a - b);
}

void rev_bin(int *fr, int fft_n)
{
    int m, mr, nn, l;
    int tr;

    mr = 0;
    nn = fft_n - 1;

    for (m = 1; m <= nn; ++m)
    {
        l = fft_n;
        do
        {
            l >>= 1;
        } while (mr + l > nn);

        mr = (mr & (l - 1)) + l;

        if (mr <= m)
            continue;
        tr = fr[m];
        fr[m] = fr[mr];
        fr[mr] = tr;
    }
}

void fft8_dit_core_p1(int *fr, int *fi)
{
    int plus1a, plus2a, plus3a, plus4a, plus1b, plus2b;
    int mins1a, mins2a, mins3a, mins4a, mins1b, mins2b, mM1a, mM2a;

    sum_dif_I(fr[0], fr[1], plus1a, mins1a);
    sum_dif_I(fr[2], fr[3], plus2a, mins2a);
    sum_dif_I(fr[4], fr[5], plus3a, mins3a);
    sum_dif_I(fr[6], fr[7], plus4a, mins4a);

    sum_dif_I(plus1a, plus2a, plus1b, mins1b);
    sum_dif_I(plus3a, plus4a, plus2b, mins2b);

    sum_dif_I(plus1b, plus2b, fr[0], fr[4]);
    sum_dif_I(mins3a, mins4a, mM1a, mM2a);

    int prib1a, prib2a, prib3a, prib4a, prib1b, prib2b;
    int otnt1a, otnt2a, otnt3a, otnt4a, otnt1b, otnt2b, oT1a, oT2a;

    sum_dif_I(fi[0], fi[1], prib1a, otnt1a);
    sum_dif_I(fi[2], fi[3], prib2a, otnt2a);
    sum_dif_I(fi[4], fi[5], prib3a, otnt3a);
    sum_dif_I(fi[6], fi[7], prib4a, otnt4a);

    sum_dif_I(prib1a, prib2a, prib1b, otnt1b);
    sum_dif_I(prib3a, prib4a, prib2b, otnt2b);

    sum_dif_I(prib1b, prib2b, fi[0], fi[4]);
    sum_dif_I(otnt3a, otnt4a, oT1a, oT2a);

    mM2a = mult_shf_s16x16(mM2a, 181);
    sum_dif_I(mins1a, mM2a, plus1a, plus2a);

    prib2b = mult_shf_s16x16(oT1a, 181);
    sum_dif_I(otnt2a, prib2b, mins3a, plus3a);

    sum_dif_I(plus1a, mins3a, fr[7], fr[1]);
    sum_dif_I(mins1b, otnt2b, fr[6], fr[2]);
    sum_dif_I(plus2a, plus3a, fr[3], fr[5]);

    oT2a = mult_shf_s16x16(oT2a, 181);
    sum_dif_I(otnt1a, oT2a, plus1a, plus2a);

    plus2b = mult_shf_s16x16(mM1a, 181);
    sum_dif_I(-mins2a, plus2b, plus3a, mins3a);

    sum_dif_I(plus1a, mins3a, fi[7], fi[1]);
    sum_dif_I(otnt1b, -mins2b, fi[6], fi[2]);
    sum_dif_I(plus2a, plus3a, fi[3], fi[5]);
}

void fft_radix4_I(int *fr, int *fi, int ldn)
{
    const int n = (1UL << ldn);
    int ldm = 0, rdx = 2;

    ldm = (ldn & 1);
    if (ldm != 0)
    {
        for (int i0 = 0; i0 < n; i0 += 8)
        {
            fft8_dit_core_p1(fr + i0, fi + i0);
        }
    }
    else
    {
        for (int i0 = 0; i0 < n; i0 += 4)
        {
            int xr, yr, ur, vr, xi, yi, ui, vi;

            int i1 = i0 + 1;
            int i2 = i1 + 1;
            int i3 = i2 + 1;

            sum_dif_I(fr[i0], fr[i1], xr, ur);
            sum_dif_I(fr[i2], fr[i3], yr, vi);
            sum_dif_I(fi[i0], fi[i1], xi, ui);
            sum_dif_I(fi[i3], fi[i2], yi, vr);

            sum_dif_I(ui, vi, fi[i1], fi[i3]);
            sum_dif_I(xi, yi, fi[i0], fi[i2]);
            sum_dif_I(ur, vr, fr[i1], fr[i3]);
            sum_dif_I(xr, yr, fr[i0], fr[i2]);
        }
    }
    for (ldm += 2 * rdx; ldm <= ldn; ldm += rdx)
    {
        int m = (1UL << ldm);
        int m4 = (m >> rdx);

        int phI0 = NWAVE / m;
        int phI = 0;

        for (int j = 0; j < m4; j++)
        {
            int c, s, c2, s2, c3, s3;

            s = pgm_read_word(&Sinewave[phI]);
            s2 = pgm_read_word(&Sinewave[2 * phI]);
            s3 = pgm_read_word(&Sinewave[3 * phI]);

            c = pgm_read_word(&Sinewave[phI + NWAVE / 4]);
            c2 = pgm_read_word(&Sinewave[2 * phI + NWAVE / 4]);
            c3 = pgm_read_word(&Sinewave[3 * phI + NWAVE / 4]);

            for (int r = 0; r < n; r += m)
            {
                int i0 = j + r;
                int i1 = i0 + m4;
                int i2 = i1 + m4;
                int i3 = i2 + m4;

                int xr, yr, ur, vr, xi, yi, ui, vi;

                mult_shf_I(c2, s2, fr[i1], fi[i1], xr, xi);
                mult_shf_I(c, s, fr[i2], fi[i2], yr, vr);
                mult_shf_I(c3, s3, fr[i3], fi[i3], vi, yi);

                int t = yi - vr;
                yi += vr;
                vr = t;

                ur = fr[i0] - xr;
                xr += fr[i0];

                sum_dif_I(ur, vr, fr[i1], fr[i3]);

                t = yr - vi;
                yr += vi;
                vi = t;

                ui = fi[i0] - xi;
                xi += fi[i0];

                sum_dif_I(ui, vi, fi[i1], fi[i3]);
                sum_dif_I(xr, yr, fr[i0], fr[i2]);
                sum_dif_I(xi, yi, fi[i0], fi[i2]);
            }
            phI += phI0;
        }
    }
}

void take_it(uint8_t direct)
{
    ADMUX &= 0xFC;
    ADMUX |= (direct << 1); // 0 & 1 - Horizontal. 2 & 3 - Vertical

    OCR0A = SMPLTIME;
    TCNT0 = 0;
    TIFR0 |= (1 << OCF0A);
    TIMSK0 |= (1 << OCIE0A);

    flagSamp = 0;
    while (!flagSamp)
        ;

    for (uint8_t y = 0; y < CHANNELS; y++)
    {
        for (uint16_t i = 0; i < FFT_SIZE; i++)
        {
            if (x_r[y][i] & 0x0200)
                x_r[y][i] += 0xFC00;                                            // Convert to negative 16-bit word (2's comp)
            x_r[y][i] = mult_shf_s16x16(x_r[y][i], pgm_read_word(&Hamming[i])); // Windowing - Hamming
        }
    }
}

ISR(TIMER0_COMPA_vect)
{
    static uint8_t n_sampl = 0;
    static uint8_t n_chanl = 0;

    x_r[n_chanl][n_sampl] = ADC;

    if (n_chanl)
        n_sampl++;
    n_chanl = !n_chanl;

    ADMUX &= 0xFE;
    ADMUX |= (n_chanl & 0x01);

    if (n_sampl >= FFT_SIZE)
    {
        flagSamp = 1;
        n_sampl = 0;
        n_chanl = 0;
        TIMSK0 &= ~(1 << OCIE0A);
    }
}

void setup()
{
    Serial.begin(115200);

    /* Setup ADC */
    ADMUX = 0xDC; // 0xD4 = x40.  0xCC = x10.  0xDC = Gain x200, (1) 11100.
    DIDR0 = 0x00; // Turn Off Digital Input Buffer.

    ADCSRA = ((1 << ADEN) |  // 1 = ADC Enable
              (0 << ADSC) |  // ADC Start Conversion
              (1 << ADATE) | // 1 = ADC Auto Trigger Enable
              (0 << ADIF) |  // ADC Interrupt Flag
              (0 << ADIE) |  // ADC Interrupt Enable
              (1 << ADPS2) |
              (0 << ADPS1) | // ADC Prescaler Selects 1 MHz.
              (0 << ADPS0));

    ADCSRB = (//(1 << ADHSM) | // High Speed mode select
//              (1 << MUX5) |  // 1 (10100) ADC1<->ADC4 Gain = 200x.
//              (0 << ADTS3) |
              (0 << ADTS2) | // Sets Auto Trigger source - TIMER0 OCA
              (1 << ADTS1) |
              (1 << ADTS0));

    /* Set up TIMER 0 - ADC sampler */
    TCCR0A = 0x02; // CTC, MODE-2, TOP-OCRA
    TCCR0B = 0x02; // (1<<CS01); DIV:8 = 16 / 8 = 2 MHz; T = 0.5 mikrosec.
    OCR0A = SMPLTIME;

    /* Set up TIMER 1 - Servo Driver */
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;

    TCCR1B |= (1 << WGM13);  // Mode 9
    TCCR1B &= ~(1 << WGM12); // Mode 9
    TCCR1A |= (1 << WGM10);  // Mode 9
    TCCR1A &= ~(1 << WGM11); // Mode 9

    TCCR1B |= (1 << CS11);                  // clk/8 prescaling.
    TCCR1B &= ~((1 << CS12) | (1 << CS10)); // prescaler = 8, T = 0.5 mikrosec.

//    TCCR1A |= ((1 << COM1B1) | (1 << COM1C1));  // PWM
//    TCCR1A &= ~((1 << COM1B0) | (1 << COM1C0)); // PWM

    OCR1B = 1350;
//    OCR1C = 1800;
    OCR1A = SRVOTIME;

    TIMSK0 = 0x00;
    TIMSK1 = 0x00;
//    TIMSK3 = 0x00;
//    TIMSK4 = 0x00;

    pinMode(10, OUTPUT); //Servo Horizontal
    pinMode(11, OUTPUT); //Servo Vertical

    pinMode(13, OUTPUT); // Gain x200 Indicator
    digitalWrite(13, HIGH);

    pinMode(9, INPUT); // Connect to Ground to change Gain to x40 - DEBUG
    digitalWrite(9, HIGH);
    if (!digitalRead(9))
    {
        ADMUX = 0xD4; // 0xD4 = x40.
        digitalWrite(13, LOW);
    }

    pinMode(12, OUTPUT); // LASER Indicator
    digitalWrite(12, HIGH);
}

void loop()
{
    char incomingByte;
    int16_t vremn = 0;

    if (directin)
        directin = 0; // X koordinate;
    else
        directin = 1; // Y koordinate;

    take_it(directin); // SAMPLING

    for (uint8_t y = 0; y < CHANNELS; y++)
    {
        for (uint8_t i = 0; i < FFT_SIZE; i++)
        {
            f_r[i] = x_r[y][i];
            f_i[i] = 0;
        }

        rev_bin(f_r, FFT_SIZE);
        fft_radix4_I(f_r, f_i, LOG_2_FFT);

        for (uint8_t i = 0; i < MIRROR; i++)
        {
            uint16_t sina, cosina;
            int16_t qr, qi, phase = 0, znak = 1;

            qr = f_r[i] >> 5; // DEFAULT gain Reset with FFT_SIZE = 128 ( 128/2 = 64 or 6 bits).
            qi = f_i[i] >> 5; // Only 5 for now, there is 3.5 bit Limiter in RAINBOW NOICE CANCELER

            cosina = abs(qr);
            sina = abs(qi);

            while ((sina > 31) || (cosina > 31)) // Scaling for aTangent LUT.
            {
                sina >>= 1;
                cosina >>= 1;
            }

            if (((sina * sina) + (cosina * cosina)) < 256)
                phase = -1; // RAINBOW NOICE CANCELER
            else
            {
                phase = pgm_read_word(&aTangentc[sina][cosina]);

                if ((qr < 0) && (qi > 0))
                    znak = -1;
                if ((qr > 0) && (qi < 0))
                    znak = -1;

                phase *= znak;

                if ((qr < 0) && (qi < 0))
                    phase -= 1024;
                if ((qr < 0) && (qi >= 0))
                    phase += 1024;
            }
            x_r[y][i] = phase;
        }
    }

    for (uint8_t i = 2; i < MIRROR; i++) // LIMITS: 150 HZ
    {                                    // Electrical grid interference, Motor Vibration.
        vremn = 0;
        if ((x_r[0][i] != -1) && (x_r[1][i] != -1))
        {
            vremn = x_r[0][i] - x_r[1][i];
            if (vremn > 1024)
                vremn -= 2048;
            if (vremn < -1024)
                vremn += 2048;

            vremn = (32 * vremn) / i;
            vremn -= 256; // MIC Time Offset Correction (1 ADC, 25 usec delay).

            if (lokaVect[directin][i] < vremn)
                lokaVect[directin][i] += srvoAccl[directin];
            if (lokaVect[directin][i] > vremn)
                lokaVect[directin][i] -= srvoAccl[directin];
        }
    }

    indxUpdt++;
    if (indxUpdt > updtRate) // Servo running at 50 Hz, don't have to be updated often ~1/8
    {
        indxUpdt = 0;
        for (uint8_t y = 0; y < 2; y++)
        {
            int16_t count = 0,
                    summa = 0;
            for (uint8_t i = 2; i < MIRROR; i++)
            {
                faza[y][i] = lokaVect[y][i];

                if (abs(lokaVect[y][i]) > 8) //  2048 / 8 ~= 360 / 256 ~= 1.5 DEGREE
                {
                    summa += lokaVect[y][i];
                    count++;

                    if (lokaVect[y][i] < 0)
                        lokaVect[y][i] = -8;
                    if (lokaVect[y][i] > 0)
                        lokaVect[y][i] = 8;
                }
                if (lokaVect[y][i] < 0)
                    lokaVect[y][i]++;
                if (lokaVect[y][i] > 0)
                    lokaVect[y][i]--;
            }

            if (count)
                summa /= count;

            srvoPosn[y] -= summa;
            if (y)
            {
                srvoPosn[y] = constrain(srvoPosn[y], 1500, 2100);
//                OCR1C = srvoPosn[y]; // Pin 11
            }
            else
            {
                srvoPosn[y] = constrain(srvoPosn[y], 800, 1900);
                OCR1B = srvoPosn[y]; // Pin 10
            }
        }
    }

    if (Serial.available() > 0)
    {
        incomingByte = Serial.read();
        // "i" command - print a table "raw" phases. "ii" print X and Y consequently.
        if (incomingByte == 'i')
        {
            for (uint8_t y = 0; y < CHANNELS; y++)
            {
                Serial.print("\n\t");
                for (uint16_t i = 0; i < FFT_SIZE; i++)
                {
                    Serial.print(x_r[y][i]);
                    Serial.print("\t");
                    if ((i + 1) % 16 == 0)
                        Serial.print("\n\t");
                }
                Serial.println("\n\t");
            }
            Serial.print("\t 0-X / 1-Y: ");
            Serial.print(directin);
        }

        if (incomingByte == 'd') // PHAZA Difference <<< DEBUG >>>
        {
            Serial.print("\n\t");
            for (uint8_t i = 0; i < MIRROR / 2; i++)
            {
                Serial.print("\n\t");
                Serial.print(i);
                Serial.print("\t");
                for (uint8_t ind = 0; ind < 2; ind++)
                {
                    if (faza[ind][i])
                        Serial.print(faza[ind][i]);
                    else
                        Serial.print("n");
                    Serial.print("\t");
                }
                Serial.print(i + MIRROR / 2);
                Serial.print("\t");
                for (uint8_t ind = 0; ind < 2; ind++)
                {
                    if (faza[ind][i + MIRROR / 2])
                        Serial.print(faza[ind][i + MIRROR / 2]);
                    else
                        Serial.print("n");
                    Serial.print("\t");
                }
            }
        }
    }
}
