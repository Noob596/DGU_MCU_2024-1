#include <msp430.h> 
void init_motor();
void init_segment();
void init_ADC();
void init_numpad();
void init_timer0();
void init_ultrasonic();
void init_timer1();
void init_encoder_intrpt();

unsigned int number[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int status = 0; // initial condition
unsigned int cycles = 0;
unsigned int timer_gl = 0;
unsigned short LED_flag = 0;
unsigned long ultra_time = 0;
unsigned int distance = 9999; //data initialize 9999
unsigned int ultra_flag = 0;
unsigned int temp_CCR1 = 0;
unsigned int temp_CCR2 = 0;
int delta_enc = 0;
int cycle_counter = 0;
volatile unsigned short star_pressed = 0;
volatile unsigned short hash_pressed = 0;
volatile unsigned short acc_flag = 0;
volatile unsigned short deacc_flag = 0;
unsigned int timer_chattering = 0; // 50 * 1000


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	init_motor();
	init_segment();
	init_ADC();
	init_numpad();
	init_ultrasonic();
	init_encoder_intrpt();

	P2OUT |= BIT1; //p2.1 init
    P2REN |= BIT1;
    P2IE = BIT1; // p2.1 intrp set
    P2IES &= ~BIT1; //rising edge
    P2IFG &= ~BIT1; //flag clear
    init_timer0();
    init_timer1();

	
    /* LED SET */

    P1DIR |= BIT0; // p1.0 led
    P1OUT &= ~BIT0;

    P4DIR |= BIT7; // p4.7 led
    P4OUT &= ~BIT7;


    __bis_SR_register(GIE);

	while(1){
	    switch(status){
            case 0:
                TA2CCR2 = 0;
                TA2CCR1 = 0;
                P4OUT = 0x0F;
                TA0CCR0 = 1000;
                deacc_flag = 0;
                acc_flag = 0;
                P1IE &= ~(BIT2 | BIT3); //too many intrpt -> disable encoder when not using
                break; // initial condition, do nothing, restore everything
            case 1: // use ADC, print segment
                ADC12CTL0 |= ADC12SC;
                cycles = ((ADC12MEM0 - 1100) * 3) >> 2; //for mapping, cycles = (3 x adc - 1100) % 4
                if(cycles >= 2099){
                    cycles = 2000;
                }
                P3OUT = number[cycles / 100 % 10];
                P4OUT = ~(BIT0 | BIT7);
                __delay_cycles(1000);

                P3OUT = number[cycles / 1000 % 10];
                P3OUT |= BIT5; // point.
                P4OUT = ~(BIT1 | BIT7);
                __delay_cycles(1000);

                P3OUT = 0x00; // off
                P4OUT = ~(BIT2 | BIT7);
                __delay_cycles(1000);

                P3OUT = 0x00; //LEFT, 7SEG4
                P4OUT = ~(BIT3 | BIT7);
                __delay_cycles(1000);

                break;

             case 2: // LED toggle
                 TA0CCR0 = 1000; // timer up to 1000 to make 1 sec
                 if(LED_flag == 0){ // p1.0
                     P1OUT |= BIT0;
                     P4OUT &= ~BIT7;
                 }
                 else if(LED_flag == 1){ // p4.7
                     P1OUT &= ~BIT0;
                     P4OUT |= BIT7;
                 }

                 break;


            case 3:
                P1OUT &= ~BIT0;
                P4OUT &= ~BIT7; // LED off

                P2OUT &= ~BIT2;
                P2OUT |= (BIT0 | BIT3); // do nothing except *and #
                if((P6IN & BIT3) == 0){

                }
                else if((P6IN & BIT6) == 0){

                }
                else if((P6IN & BIT5) == 0){

                }
                else if((P6IN & BIT4) == 0){ // *
                    star_pressed = 1;
                    if(TA2CCR2 < TA2CCR1){
                        deacc_flag = 1;
                        acc_flag = 0;
                    }
                    else if(TA2CCR2 >= TA2CCR1){
                        deacc_flag = 0;
                        acc_flag = 1;
                    }
                    /* TA2CCR2++;
                    TA2CCR1 = 0; */
                }
                else { // * not pressed
                    star_pressed = 0;
                }

                // col 2 not need

                // col 3
                P2OUT &= ~BIT3;
                P2OUT |= (BIT0 | BIT2);
                if((P6IN & BIT3) == 0){

                }
                else if((P6IN & BIT6) == 0){

                }
                else if((P6IN & BIT5) == 0){

                }
                else if((P6IN & BIT4) == 0){ // #
                    hash_pressed = 1;
                    if(TA2CCR1 < TA2CCR2){
                        deacc_flag = 1;
                        acc_flag = 0;
                    }
                    else if(TA2CCR1 >= TA2CCR2){
                        deacc_flag = 0;
                        acc_flag = 1;
                    }

                    /* TA2CCR2 = 0;
                    TA2CCR1++; */
                }
                else {
                    hash_pressed = 0;
                }
                temp_CCR1 = TA2CCR1;
                temp_CCR2 = TA2CCR2;

                break;
            case 4: // ultrasonic stopping, digit prints in timer0 intrpt
                TA0CCR0 = 6000; // timer need to be over 5000 to precise ultrasonic
                if (ultra_flag == 0) {
                    P2OUT |= BIT7; // trig on
                    __delay_cycles(10); // 10us delay
                    P2OUT &= ~BIT7; // trig off
                    ultra_flag = 1;
                }

                if (distance < 10){ //motor stop
                    TA2CCR1 = 0;
                    TA2CCR2 = 0;
                }
                else if (distance > 10){
                    TA2CCR1 = temp_CCR1;
                    TA2CCR2 = temp_CCR2;
                }

                break;
            case 5: // digit off
                P1IE |= (BIT2 | BIT3); //enable encoder intrpt
                P4OUT = 0x0F;
                TA0CCR0 = 10000; // low freq timer
                break;


        }
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void global_timer(void) {

    timer_gl++; // global timer

    if ((status == 2) && (timer_gl > ((cycles / 1000 % 10) * 1000 + (cycles / 100 % 10) * 100))){
        timer_gl = 0;
        LED_flag ^= 1; // flag toggle

    }
    /* motor acc*/
    if(status == 3){ // when its motor acc status
        if((star_pressed == 1) && (hash_pressed == 0)){ // when star pressed, prevent both pressed
            if(acc_flag == 1){// 1/10000 increase every 1ms -> 10 sec, skip 3000
                TA2CCR2++;
                TA2CCR1 = 0;
                if(TA2CCR2 < 3000){
                    TA2CCR2 = 3000;
                }
                /* P1OUT |= BIT0;
                P4OUT &= ~BIT7; */
            }
           else if(deacc_flag == 1){
                TA2CCR1--;
                if(TA2CCR1 < 3000){
                    TA2CCR1 = 0;
                }
                /* P1OUT &= ~BIT0;
                P4OUT |= BIT7; */
            }
        }
        else if((star_pressed == 0) && (hash_pressed == 1)){
            if(acc_flag == 1){ // 1/10000 increase every 1ms -> 10 sec, skip 3000
                TA2CCR1++;
                TA2CCR2 = 0;
                if(TA2CCR1 < 3000){
                    TA2CCR1 = 3000;
                }
                /* P1OUT |= BIT0;
                P4OUT &= ~BIT7; */
            }
            else if(deacc_flag == 1){
                TA2CCR2--;
                if(TA2CCR2 < 3000){
                    TA2CCR2 = 0;
                }
                /* P1OUT &= ~BIT0;
                P4OUT |= BIT7; */
            }
        }
    }

    else if((timer_gl < 5) && (status == 4)){ //print ultrasonic

        switch (timer_gl) {
            case 1:
                P3OUT = number[distance % 10];
                P4OUT = ~(BIT0 | BIT7);
                break;
            case 2:
                P3OUT = number[distance / 10 % 10];
                P4OUT = ~(BIT1 | BIT7);
                break;
            case 3:
                P3OUT = number[distance / 100 % 10];
                P4OUT = ~(BIT2 | BIT7);
                break;
            case 4:
                P3OUT = number[distance / 1000 % 10];
                P4OUT = ~(BIT3 | BIT7);
                break;
        }
    }
    else if((timer_gl > 4) && (status == 4))
        timer_gl = 0;


    /* motor stop */
    if(status == 5){
        delta_enc = cycle_counter;
        if(delta_enc < 2 ){ //
            TA2CCR2 = 0;
            TA2CCR1 = 0;
        }
        cycle_counter = 0;
    }

}

#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void){
    if(P2IFG & BIT1){
        // while((P2IN & BIT1) == 0) {} //chattering

        // P2IE &= ~BIT1;
        if(timer_chattering == 0)
            status++;

        timer_chattering = 1000;
        if(status >= 6)
            status = 0;
    }
    if(status == 5){
        TA2CCR1 = temp_CCR1;
        TA2CCR2 = temp_CCR2;
    }
    P2IFG &= ~BIT1;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    ultra_time++;
    if (ultra_flag == 1 && ultra_time > 1000) {
        ultra_flag = 0; // reset flag 50/1mhz x 1000
    }

    if(timer_chattering != 0){
        timer_chattering--;
    }
    /* else if(timer_chattering == 0){
        P2IE |= BIT1;

    } */
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & BIT4) { // ultrasnoic intrpt
        if ((P1IES & BIT4) == 0) { // rising edge
            ultra_time = 0;
            P1IES |= BIT4; // edge sel change
        } else { // falling edge
            if (ultra_time > 3 && ultra_time < 500) { // dd
                distance = ultra_time * 50 / 58;
            } else if (ultra_time >= 760) {
                distance = 9999;
            } else {
                distance = 0;
            }
            P1IES &= ~BIT4; // edge sel rising
            ultra_flag = 0; // clear flag
        }
    }
    P1IFG &= ~BIT4; // flag clear

    if(P1IFG & BIT3) { // encoder intrpt
            if((P1IN & BIT2) != 0) {
                cycle_counter++;
            } else {
                cycle_counter++;
            }
        }

    if(P1IFG & BIT2) {
        if((P1IN & BIT3) == 0) {
            cycle_counter++;
        } else {
            cycle_counter++;
        }
    }

    /* if(cycle_counter > 9999)
        cycle_counter = 0;
    else if(cycle_counter < 0)
        cycle_counter = 9999; */

    P1IFG &= ~BIT3;     //IFG is cleared
    P1IFG &= ~BIT2;     //IFG is cleared
}

void init_motor(){
    P2DIR |= (BIT5 | BIT4); // output select
    P2SEL |= (BIT5 | BIT4); // alt mode select

    P2DIR |= (BIT0 | BIT2);
    P2OUT &= ~(BIT0 | BIT2);

    TA2CTL = TASSEL_2 + MC_1;
    TA2CCR0 = 10000; // x/10000 speed
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;

}

void init_segment(){
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3; //dynamic outputs
    P3DIR |= 0xFF; //segment outputs
    P4OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P3OUT &= 0x00;
}

void init_ADC(){
    P6SEL |= BIT0; // alternative mod
    ADC12CTL0 = ADC12SHT02 + ADC12ON; //c&p
    ADC12CTL1 = ADC12SHP; //c&p
    ADC12MCTL0 = ADC12INCH_0; //c&p
    ADC12CTL0 |= ADC12ENC; //c&p */
    // ADC12CTL0 |= ADC12SC; means starts convert, it must need
    // read ADC12MEM0
    // need more info
}

void init_numpad(){
    P2DIR |= (BIT0 | BIT2 | BIT3); // output (col, 147* p2.2/ 2580 p2.0/ 369# p2.3)
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input (row, 123 p6.3/ 456 p6.6 789 p6.5/ *0# p6.4)
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up, active low

}

void init_timer0(){
    TA0CCTL0 = CCIE; //#pragma vector=TIMER0_A0_VECTOR
    TA0CCR0 = 1000;  //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // 1Mhz, to ccr0
}

void init_timer1(){ //use for ultrasonic & chattering
    TA1CCTL0 = CCIE; // #pragma vector=TIMER1_A0_VECTOR
    TA1CCR0 = 50; //50us
    TA1CTL = TASSEL_2 + MC_1 + TACLR; //

}

void init_ultrasonic(){
    P2DIR |= BIT7; //trig is output
    P2OUT &= ~BIT7; // trig
    P1IE |= BIT4; // p1.4 echo interrupt enable
    P1IES &= ~BIT4; // rising edge  //P1IES |= BIT4;  // falling edge
    P1IFG &= ~BIT4; // flag clear
}

void init_encoder_intrpt(){
    P1IE |= BIT3; //p1.3 intrpt enable
    P1IES &= ~BIT3; //p1.3 edge select to falling edge
    P1IFG &= ~BIT3; //intrpt flag clear
    P1IE |= BIT2; //p.12 intrpt enable
    P1IES &= ~BIT2; //p1.2 edge select to falling edge
    P1IFG &= ~BIT2; //flag clear
}
