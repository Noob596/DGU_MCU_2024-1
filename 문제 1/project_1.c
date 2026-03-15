#include <msp430.h>
void init_segment();
void init_timer0();
void init_timer1();
void init_encoder_intrpt();
void init_motor();

volatile int random_value = 0;
unsigned int number[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
volatile unsigned int status = 0;
unsigned int timer_seg = 0;
unsigned int timer_gl = 0;
volatile unsigned short led_flag_one = 0;
volatile unsigned short led_flag_two = 0;
unsigned int timer_1sec = 0;
unsigned int time_took = 0;
unsigned short time_took_enable = 0;
int position = 0;
int position_cnt = 0;
signed int cycle_counter_short = 0;
signed long cycle_counter_precise = 0; // 1,000,000 for 1 cycle
// unsigned int timer_chattering = 0; // 1000 * 90

/* 1 cycle = 670 encoder count
    1 encoder count approx 0.001492 cycle
    -> 1 encoder count = 1492 count -> 1,000,000 for 1 cycle
     */

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    /* p1.1 init */
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;
    P1IE |= BIT1;
    P1IES |= BIT1; // fall edge
    P1IFG &= ~BIT1;

    /* led init */
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    init_encoder_intrpt();
    init_timer0();
    init_segment();
    init_timer1();
    init_motor();

    __bis_SR_register(GIE);

    while (1) {
        /* led flag */
        if(led_flag_one == 1){
            P1OUT |= BIT0;
        }
        else if(led_flag_one == 0){
            P1OUT &= ~BIT0;
        }

        if(led_flag_two == 1){
            P4OUT |= BIT7;
        }
        else if(led_flag_two == 0){
            P4OUT &= ~BIT7;
        }
        ///////////////////////

        switch(status){
        case 0: // initial status -> do nothing and refresh
            time_took_enable = 0;
            P4OUT = 0x0F;
            TA2CCR2 = 0;
            TA2CCR1 = 0;
            position = 0;
            position_cnt = 0;
            cycle_counter_short = 0;
            cycle_counter_precise = 0;
            break;

        case 1: // after random time, p1.0 led 1sec on

            break;
        case 2: // p4.7 led on, how much time took
            switch(timer_seg){
            case 0: //7SEG3
                P3OUT = number[time_took / 100 % 10]; // 7SEG3
                if((position ) == 1){
                    P3OUT |= BIT5;
                }
                P4OUT = ~(BIT2 | BIT7);

                break;
            case 1: //7SEG2
                P3OUT = number[time_took / 10 % 10]; //
                if((position) == 2){
                    P3OUT |= BIT5;
                }
                P4OUT = ~(BIT1 | BIT7);

                break;
            case 2: //7SEG1
                P3OUT = number[time_took % 10]; //
                if((position ) == 3){
                    P3OUT |= BIT5;
                }
                P4OUT = ~(BIT0 | BIT7);

                break;
            case 3: //7SEG4
                P3OUT = number[time_took / 1000 % 10]; //
                if((position) == 0){
                    P3OUT |= BIT5;
                }
                P4OUT = ~(BIT3 | BIT7);

                break;
            }
            break;
            case 3:
                switch(position){
                case 0: // x.xxx cycle, precise count
                    switch(timer_seg){
                    case 0: //7SEG3
                        P3OUT = number[(cycle_counter_precise / 100000) % 10]; // 7SEG3
                        if((position ) == 1){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT2 | BIT7);

                        break;
                    case 1: //7SEG2
                        P3OUT = number[(cycle_counter_precise / 10000) % 10]; //
                        if((position) == 2){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT1 | BIT7);

                        break;
                    case 2: //7SEG1
                        P3OUT = number[(cycle_counter_precise / 1000) % 10]; //
                        if((position ) == 3){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT0 | BIT7);

                        break;
                    case 3: //7SEG4
                        P3OUT = number[cycle_counter_precise / 1000000]; //
                        if((position) == 0){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT3 | BIT7);

                        break;
                    }
                    break;

                case 1:
                    switch(timer_seg){
                    case 0: //7SEG3
                        P3OUT = number[(cycle_counter_precise / 1000000) % 10]; // 7SEG3
                        if((position ) == 1){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT2 | BIT7);

                        break;
                    case 1: //7SEG2
                        P3OUT = number[(cycle_counter_precise / 100000) % 10]; //
                        if((position) == 2){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT1 | BIT7);

                        break;
                    case 2: //7SEG1
                        P3OUT = number[(cycle_counter_precise / 10000) % 10]; //
                        if((position ) == 3){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT0 | BIT7);

                        break;
                    case 3: //7SEG4
                        P3OUT = number[cycle_counter_precise / 10000000]; //
                        if((position) == 0){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT3 | BIT7);

                        break;
                    }
                    break;
                case 2:
                    switch(timer_seg){
                    case 0: //7SEG3
                        P3OUT = number[cycle_counter_short / 100 % 10]; // 7SEG3
                        if((position ) == 1){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT2 | BIT7);

                        break;
                    case 1: //7SEG2
                        P3OUT = number[cycle_counter_short / 10 % 10]; //
                        if((position) == 2){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT1 | BIT7);

                        break;
                    case 2: //7SEG1
                        P3OUT = number[cycle_counter_short % 10]; //
                        if((position ) == 3){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT0 | BIT7);

                        break;
                    case 3: //7SEG4
                        P3OUT = number[cycle_counter_short / 1000 % 10]; //
                        if((position) == 0){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT3 | BIT7);

                        break;
                    }
                    break;
                case 3:
                    switch(timer_seg){
                    case 0: //7SEG3
                        P3OUT = number[cycle_counter_short / 100 % 10]; // 7SEG3
                        if((position ) == 1){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT2 | BIT7);

                        break;
                    case 1: //7SEG2
                        P3OUT = number[cycle_counter_short / 10 % 10]; //
                        if((position) == 2){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT1 | BIT7);

                        break;
                    case 2: //7SEG1
                        P3OUT = number[cycle_counter_short % 10]; //
                        if((position ) == 3){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT0 | BIT7);

                        break;
                    case 3: //7SEG4
                        P3OUT = number[cycle_counter_short / 1000 % 10]; //
                        if((position) == 0){
                            P3OUT |= BIT5;
                        }
                        P4OUT = ~(BIT3 | BIT7);

                        break;
                    }
                    break;
                }


                break;


        }

        /* P3OUT = number[timer_gl % 10];
        P4OUT = ~BIT0;
        __delay_cycles(1000);

        P3OUT = number[timer_gl / 10 % 10];
        P4OUT = ~BIT1;
        __delay_cycles(1000);

        P3OUT = number[timer_gl / 100 % 10];
        P4OUT = ~BIT2;
        __delay_cycles(1000);

        P3OUT = number[timer_gl / 1000 % 10];
        P4OUT = ~BIT3;
        __delay_cycles(1000); */

    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer_global(void) {
    timer_gl++;
    timer_1sec++;
    /* if(timer_chattering != 0){
        timer_chattering--;
    }

    if(timer_chattering == 0){
        P1IE |= BIT1;
    } */
    if(status == 1){
        if(timer_gl < random_value){
            led_flag_one = 0; // led 1.0 off for random value time (rand * 1000ms = s)
        }
        else if(timer_gl == random_value){
            led_flag_one = 1;
            time_took_enable = 1;
        }
    }

    if(timer_gl == (random_value + 1000)){ // 1sec
        led_flag_one = 0;
        random_value = -1; // random value refresh
    }

    if(timer_1sec > 1000){
        led_flag_two = 0;
    }

    /* time took */
    if(time_took_enable){
        time_took++;
        if(time_took > 9000){ //maximum 9sec
            time_took = 0;
            time_took_enable = 0;
        }
    }
    /////////////////

    if(timer_gl > 4000){
        timer_gl = 0;
    }
    /* segment timer */
    timer_seg++;
    if(timer_seg > 4){
        timer_seg = 0;
    }
    ///////////////////
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
    if ((P1IN & BIT1) == 0) {
        while((P1IN & BIT1) == 0){}
        // P1IE &= ~BIT1;
        // timer_chattering = 90;
        if (status == 3){
            status = 0;
        }
        else if (status == 2){
            status = 3;
            switch(position){
            case 0:
                cycle_counter_precise = (unsigned long)time_took * 1000; // x.xxx cycle
                break;
            case 1:
                cycle_counter_precise = (unsigned long)time_took * 10000; // xx.xx cycle
                break;
            case 2:
                cycle_counter_short = time_took; // xxx.x cycle
                break;
            case 3:
                cycle_counter_short = time_took; // xxxx cycle
                break;
            }
            position_cnt = 0;
            TA2CCR2 = 0;
            TA2CCR1 = 500;
        }
        else if (status == 1){
            status = 2;
            time_took_enable = 0;
            timer_1sec = 0;
            led_flag_two = 1;
        }
        else if(status == 0){
            random_value = (TA0R % 1000) + 1000;   // timer value to random value, 1000 to 2000
            status = 1;
            timer_gl = 0; // for time check
        }
    }
    P1IFG &= ~BIT1;       // flag clear

    /* encoder function */
    if(status == 2){
        if(P1IFG & BIT3) { // encoder intrpt
                if((P1IN & BIT2) != 0) {
                    position_cnt--;
                } else {
                    position_cnt++;
                }
            }

        if(P1IFG & BIT2) {
            if((P1IN & BIT3) == 0) {
                position_cnt--;
            } else {
                position_cnt++;
            }
        }

        if(position_cnt > 400)
            position_cnt = 0;
        else if(position_cnt < 0)
            position_cnt = 399;

        position = position_cnt /100;

        P1IFG &= ~BIT3;     //IFG is cleared
        P1IFG &= ~BIT2;     //IFG is cleared
    }

    else if((status == 3) && ((position == 0) || (position == 1))){ // precise count
        if(P1IFG & BIT3) { // encoder intrpt
                if((P1IN & BIT2) != 0) {
                    cycle_counter_precise = cycle_counter_precise - 1492;
                } else {
                    cycle_counter_precise = cycle_counter_precise - 1492;
                }
            }

        if(P1IFG & BIT2) {
            if((P1IN & BIT3) == 0) {
                cycle_counter_precise = cycle_counter_precise - 1492;
            } else {
                cycle_counter_precise = cycle_counter_precise - 1492;
            }
        }

        if(cycle_counter_precise < 0){
            cycle_counter_precise = 0;
            TA2CCR2 = 0;
            TA2CCR1 = 0;
            time_took = 0; // refresh and wait for status 0

        }
        P1IFG &= ~BIT3;     //IFG is cleared
        P1IFG &= ~BIT2;     //IFG is cleared
    }
    else if((status == 3) && (position == 2)){ // short count, xxx.x cycle count
        if(P1IFG & BIT3) { // encoder intrpt
                if((P1IN & BIT2) != 0) {
                    position_cnt++;
                } else {
                    position_cnt++;
                }
            }

        if(P1IFG & BIT2) {
            if((P1IN & BIT3) == 0) {
                position_cnt++;
            } else {
                position_cnt++;
            }
        }

        if(position_cnt >= 67){
            position_cnt = 0;
            cycle_counter_short--;
        }

        if(cycle_counter_short <= 0){
            TA2CCR2 = 0;
            TA2CCR1 = 0;
            cycle_counter_short = 0;
            time_took = 0; // refresh and wait for status 0
        }
        P1IFG &= ~BIT3;     //IFG is cleared
        P1IFG &= ~BIT2;     //IFG is cleared
    }

    else if((status == 3) && (position == 3)){ // short count, xxxx cycle count
        if(P1IFG & BIT3) { // encoder intrpt
                if((P1IN & BIT2) != 0) {
                    position_cnt++;
                } else {
                    position_cnt++;
                }
            }

        if(P1IFG & BIT2) {
            if((P1IN & BIT3) == 0) {
                position_cnt++;
            } else {
                position_cnt++;
            }
        }

        if(position_cnt >= 670){
            position_cnt = 0;
            cycle_counter_short--;
        }

        if(cycle_counter_short <= 0){
            TA2CCR2 = 0;
            TA2CCR1 = 0;
            cycle_counter_short = 0;
            time_took = 0; // refresh and wait for status 0
        }
        P1IFG &= ~BIT3;     //IFG is cleared
        P1IFG &= ~BIT2;     //IFG is cleared
    }
    /* cycle count */

}

void init_segment(){
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3; //dynamic outputs
    P3DIR |= 0xFF; //segment outputs
    P4OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P3OUT &= 0x00;
}

void init_timer0(){
    // TA0CCTL0 = CCIE; //#pragma vector=TIMER0_A0_VECTOR
    // TA0CCR0 = 0xFF;  //1000;
    TA0CTL = TASSEL_2 + MC_2 + TACLR; // 1Mhz, up to 0xffff(MC2)
}

void init_timer1(){
    TA1CCTL0 = CCIE; // #pragma vector=TIMER1_A0_VECTOR
    TA1CCR0 = 1000; //1000, main timer ISR
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCR0

}

void init_encoder_intrpt(){
    P1IE |= BIT3; //p1.3 intrpt enable
    P1IES &= ~BIT3; //p1.3 edge select to falling edge
    P1IFG &= ~BIT3; //intrpt flag clear
    P1IE |= BIT2; //p.12 intrpt enable
    P1IES &= ~BIT2; //p1.2 edge select to falling edge
    P1IFG &= ~BIT2; //flag clear
}

void init_motor(){
    P2DIR |= (BIT5 | BIT4); // output select
    P2SEL |= (BIT5 | BIT4); // alt mode select

    P2DIR |= (BIT0 | BIT2);
    P2OUT &= ~(BIT0 | BIT2);

    TA2CTL = TASSEL_2 + MC_1;
    TA2CCR0 = 1000; // x/1000 speed
    TA2CCTL2 = OUTMOD_6;
    TA2CCR2 = 0;
    TA2CCTL1 = OUTMOD_6;
    TA2CCR1 = 0;

}
