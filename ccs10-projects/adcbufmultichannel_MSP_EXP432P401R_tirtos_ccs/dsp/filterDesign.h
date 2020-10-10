#ifndef DSP_FILTERDESIGN_H_
#define DSP_FILTERDESIGN_H_

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 10000 Hz

fixed point precision: 16 bits

* 0 Hz - 2000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 2100 Hz - 3000 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 3200 Hz - 5000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM 58

static q15_t filter_taps[FILTER_TAP_NUM] = {
                                            1103,
                                            2089,
                                            1127,
                                            -811,
                                            -749,
                                            449,
                                            -74,
                                            -887,
                                            -73,
                                            74,
                                            -944,
                                            -544,
                                            78,
                                            -962,
                                            -1061,
                                            -15,
                                            -864,
                                            -1619,
                                            -245,
                                            -573,
                                            -2170,
                                            -699,
                                            30,
                                            -2638,
                                            -1628,
                                            1322,
                                            -2953,
                                            -4677,
                                            8481,
                                            18778,
                                            8481,
                                            -4677,
                                            -2953,
                                            1322,
                                            -1628,
                                            -2638,
                                            30,
                                            -699,
                                            -2170,
                                            -573,
                                            -245,
                                            -1619,
                                            -864,
                                            -15,
                                            -1061,
                                            -962,
                                            78,
                                            -544,
                                            -944,
                                            74,
                                            -73,
                                            -887,
                                            -74,
                                            449,
                                            -749,
                                            -811,
                                            1127,
                                            1103+2089 //,
//                                            1103
                                          };





#endif /* DSP_FILTERDESIGN_H_ */
