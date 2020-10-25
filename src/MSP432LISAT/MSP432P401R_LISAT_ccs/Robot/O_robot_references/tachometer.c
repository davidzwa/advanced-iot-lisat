///*
// * Date  : 25 Jul 2020
// * Author: Amjad Yousef Majid
// * Email : amjad.y.majid@gmail.com
// */
//
//#include <interruptHandler.h>
//#include <stdint.h>
//#include "msp.h"
//
//void(*taskLeft)(void);
//void(*taskRight)(void);
//
//void tachometer_init(void(*userTaskLeft)(void), void(*userTaskRight)(void) ){
//
//    P5->DIR |=BIT4+BIT5;
//    P5->OUT &=~(BIT4+BIT5);
//
//	//setup the hooks for the tachometers ISRs
//	taskLeft =  userTaskLeft;
//	taskRight =  userTaskRight;
//	// // pin 9.2 and 10.5 are used to figure out the spinning direction (which is normally known)
//	// P9->SEL0 &= ~BIT2;
//	// P9->SEL1 &= ~BIT2;               // configure P9.2 as GPIO
//	// P9->DIR &= ~BIT2;                // make P9.2 in
//
//	// P10->SEL0 &= ~BIT5;
//	// P10->SEL1 &= ~BIT5;              // configure P10.5 as GPIO
//	// P10->DIR &= ~BIT5;               // make P10.5 in
//
//	// configure pin 6.1 as input and enable interrupt
//	P6->SEL0 &= ~BIT1;
//	P6->SEL1 &= ~BIT1;
//	P6->DIR &= ~BIT1;
//
//	P6->IFG &= ~BIT1; // ensure that the interrupt flag is initially clear
//	P6->IES &= ~BIT1;  // rising edge
//	P6->IE |= BIT1;  // arm (enable) interrupt on P6.1 and P6.6
//
//	NVIC->IP[10] &= (NVIC->IP[10] &0xFFFFFF00 )|0x00000020;
//	NVIC->ISER[1] =  1 << (PORT6_IRQn & 31); /* enable interrupt 40 in NVICC
//                           This is a friendly operation as 0's does not clear the bits.
//                           To disarm and interrupt in NVIC, you must use ICER register. */
//
//    // configure pin 5.0 as input and enable interrupt
//    P5->SEL0 &= ~(BIT0);
//    P5->SEL1 &= ~(BIT0);
//    P5->DIR &= ~(BIT0);
//
//    P5->IFG &= ~(BIT0);
//    P5->IES &= ~(BIT0);
//    P5->IE |= (BIT0);
//
//    NVIC->IP[9] &= (NVIC->IP[9] &0x00FFFFFF)|0x20000000;
//    NVIC->ISER[1] =  1 << (PORT5_IRQn & 31);
//
//}
//
//
//void PORT5_IRQHandler(void){
//    if(P5->IFG & BIT0){
//        P5->IFG &= ~BIT0;
//        // execute user hook
//        (*taskLeft)();
////        P5->OUT |=BIT4;
////        P5->OUT &=~BIT4;
//    }
//}
//
//void PORT6_IRQHandler(void){
//	if(P6->IFG & BIT1){
//	    P6->IFG &= ~BIT1;
//		// execute user hook
//		(*taskRight)();
////        P5->OUT |=BIT5;
////        P5->OUT &=~BIT5;
//	}
//}
//
//
