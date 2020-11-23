/*
 * debug_prints.cpp
 *
 *  Created on: Nov 23, 2020
 *      Author: d.zwart
 */

//        arm_min_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &minValue, &minIndex);
//        Display_printf(display, 0, 0, "Mi.%d", minValue);
//        arm_max_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &maxValue, &maxIndex);
//        Display_printf(display, 0, 0, "Ma.%d", maxValue);
//        Send RMS value of ADCBuf for either Valin or CTP or algorithm
//        arm_rms_q15(outputBuffer_filtered, ADCBUFFERSIZE_SHORT, &rms);
//        Display_printf(display, 0, 0, "R.%d", rms);
//         Decide to send ADC buffer over the line
//        for (int i = 0; i < ADCBUFFERSIZE; i++) {
//            Display_printf(display, 0, 0, "v.%d", outputBuffer[i]);
//        }
//                Display_printf(display, 0, 0, "S.%d", ADCBUFFERSIZE_SHORT);
//                Display_printf(display, 0, 0, "F.%d", SAMPLE_FREQUENCY);
//        Send mic time differences
//        Display_printf(display, 0, 0, "M1.%ld", lastTriggerMic1L);
//        Display_printf(display, 0, 0, "M2.%ld", lastTriggerMic2M);
//        Display_printf(display, 0, 0, "M3.%ld", lastTriggerMic3R);
//        Send DOA values for either Valin or CTP or algorithm
//        Display_printf(display, 0, 0, "Dv1.%f", outputDirVector2D_valin[0]);
//        Display_printf(display, 0, 0, "Dv2.%f", outputDirVector2D_valin[1]);
//        Display_printf(display, 0, 0, "Dp1.%f", outputDirVector2D_plane_cutting[0]);
//        Display_printf(display, 0, 0, "Dp2.%f", outputDirVector2D_plane_cutting[1]);
//        Display_printf(display, 0, 0, "--Done");

