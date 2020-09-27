//#include <arm_math.h>
//
//// https://github.com/ARM-software/CMSIS/blob/master/CMSIS/DSP_Lib/Examples/arm_fir_example/ARM/arm_fir_example_f32.c
//
//#ifndef SIGNAL_LINK__H
//
////#define TEST_LENGTH_SAMPLES  320
//#define BLOCK_SIZE            32
//#define NUM_TAPS              29
//
//q15_t FILTER_COEFFS_EX1[16] = {
//    +0.001962f,
//    -0.001728, _Q15(-0.012558), _Q15(-0.023557),
//    _Q15(-0.004239), _Q15(+0.072656), _Q15(+0.189060), _Q15(+0.278404),
//    _Q15(+0.278404), _Q15(+0.189060), _Q15(+0.072656), _Q15(-0.004239),
//    _Q15(-0.023557), _Q15(-0.012558), _Q15(-0.001728), _Q15(+0.001962)
//};
//
//static q15_t firStateQ15[BLOCK_SIZE + NUM_TAPS - 1];
//
//uint32_t i;
//arm_fir_instance_q15 S;
//arm_status status;
//int16_t  *outputQ15;
//
//#endif // SIGNAL_LINK__H
