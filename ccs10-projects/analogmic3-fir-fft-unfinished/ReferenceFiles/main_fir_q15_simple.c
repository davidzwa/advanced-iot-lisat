//#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//
//#include <stdint.h>
//#include <stdbool.h>
//#include "arm_math.h"
//#include "arm_const_structs.h"
//
//#define TEST_LENGTH_SAMPLES 320
//#define BLOCK_SIZE 320
//#define NUM_TAPS 28
//
//uint32_t blockSize = BLOCK_SIZE;
//uint32_t numBlocks = TEST_LENGTH_SAMPLES/BLOCK_SIZE;
//static q15_t testInput_q15_50Hz_200Hz[TEST_LENGTH_SAMPLES]=
//{0,10000,0,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40,0,40,38,7,0,32,61,45,0,-21,0,21,0,-45,-61,-32,0,-7,-38,-40};
//static q15_t testOutput[TEST_LENGTH_SAMPLES];
//static q15_t firStateF32[BLOCK_SIZE + NUM_TAPS -1];
//q15_t firCoeffs32LP[NUM_TAPS] = {1,0,0,1,1,1,-1,-3,-5,-3,4,10,10,10,0,0,14,4,-3,-5,-3,-1,1,1,1,0,0,0};
//
//static void arm_fir_q15_lp(void)
//{
//    arm_fir_instance_q15 S;
//    arm_fir_init_q15(&S,NUM_TAPS, &firCoeffs32LP, &firStateF32, blockSize);
//
//    arm_fir_q15(&S,&testInput_q15_50Hz_200Hz,&testOutput, blockSize);
//}
//
//void main(void)
//{
//    /* Stop Watchdog */
//    MAP_WDT_A_holdTimer();
//    arm_fir_q15_lp();
//    while(1);
//}