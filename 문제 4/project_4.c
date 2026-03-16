#include <msp430.h> 
void init_segment();
void init_encoder_intrpt();
void init_motor();
void init_timer0();
void init_numpad();
int password_check(int password_input[], int password[]);
void shiftRight(int pwm[3]);
void shiftRightLove(int love[4]);
// void init_timer1();

int position = 0;
unsigned int love[4] = {0x83, 0xDB, 0xD3, 0X8F}; // L O V E
unsigned int number[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int button_pressed = 0;
unsigned int timer_gl = 0;
unsigned int timer_seg = 0;
volatile unsigned int status = 0;
unsigned short keypad_enable = 0;
int password_input[3] = {10, 10, 10}; // to prevent unwanted action
int password[3] = {4, 8, 6};
unsigned short password_count = 0;
unsigned short i = 0;
unsigned int cycle_counter = 0;
int cycle = 9; // 10 cycles including 0
volatile unsigned int pwm[3] = {4, 8, 6};
unsigned short motor_flag = 0;
// unsigned int timer_chattering = 0; // 100 * 500



int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	init_segment();
	init_encoder_intrpt();
	init_motor();
	init_timer0();
	init_numpad();
	// init_timer1();
	
	P1DIR &= ~BIT1;
	P1OUT |= BIT1; //p1.1 init
    P1REN |= BIT1;
    P1IE |= BIT1; // p1.1 interrupt set
    P1IES |= BIT1; // falliing edge
    P1IFG &= ~BIT1; //flag clear

    /* led init */
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;


    __bis_SR_register(GIE);

    while(1){
        switch(status){
        case 0: // initial status, wait for double click
            TA2CCR1 = 0;
            TA2CCR2 = 0;
            P4OUT = 0x0F;
            break;
        case 1: // encoder enable + keypad enable
            switch(position / 100){ // segement position
            case 0:
                P3OUT = BIT3; //LEFT, 7SEG4
                P4OUT = ~(BIT3 | BIT7);
                break;
            case 1:
                P3OUT = BIT3; // 7SEG3
                P4OUT = ~(BIT2 | BIT7);
                break;
            case 2:
                P3OUT = BIT3; // 7SEG2
                P4OUT = ~(BIT1 | BIT7);
                break;
            case 3:
                P3OUT = BIT3; // 7SEG1
                P4OUT = ~(BIT0 | BIT7);
                break;
            case 4:
                P3OUT = BIT4;
                P4OUT = ~(BIT0 | BIT7);
                break;
            case 5:
                P3OUT = BIT6;
                P4OUT = ~(BIT0 | BIT7);
                break;
            case 6:
                P3OUT = BIT0;
                P4OUT = ~(BIT0 | BIT7);
                break;
            case 7:
                P3OUT = BIT0;
                P4OUT = ~(BIT1 | BIT7);
                break;
            case 8:
                P3OUT = BIT0;
                P4OUT = ~(BIT2 | BIT7);
                break;
            case 9:
                P3OUT = BIT0;
                P4OUT = ~(BIT3 | BIT7);
                keypad_enable = 0;
                password_input[0] = 10; //refresh
                password_input[1] = 10;
                password_input[2] = 10;
                password_count = 0;
                break;
            case 10: // required position
                P3OUT = BIT1;
                P4OUT = ~(BIT3 | BIT7);
                keypad_enable = 1;

                break;
            case 11:
                P3OUT = BIT7;
                P4OUT = ~(BIT3 | BIT7);
                keypad_enable = 0;
                password_input[0] = 10; //refresh
                password_input[1] = 10;
                password_input[2] = 10;
                password_count = 0;
                break;
            }

            if(keypad_enable == 1){ // if in required position, numpad enable
                /* input from numpad */
                P2OUT &= ~BIT2;
                P2OUT |= (BIT0 | BIT3);
                if((P6IN & BIT3) == 0){
                    while((P6IN & BIT3) == 0){}
                    password_input[password_count] = 1;
                    password_count++;
                }
                else if((P6IN & BIT6) == 0){
                    while((P6IN & BIT6) == 0){}
                    password_input[password_count] = 4;
                    password_count++;
                }
                else if((P6IN & BIT5) == 0){
                    while((P6IN & BIT5) == 0){}
                    password_input[password_count] = 7;
                    password_count++;
                }
                else if((P6IN & BIT4) == 0){ // * nothing.

                }

                // col 2
                P2OUT &= ~BIT0;
                P2OUT |= (BIT2 | BIT3);
                if((P6IN & BIT3) == 0){
                    while((P6IN & BIT3) == 0){}
                    password_input[password_count] = 2;
                    password_count++;
                }
                else if((P6IN & BIT6) == 0){
                    while((P6IN & BIT6) == 0){}
                    password_input[password_count] = 5;
                    password_count++;
                }
                else if((P6IN & BIT5) == 0){
                    while((P6IN & BIT5) == 0){}
                    password_input[password_count] = 8;
                    password_count++;
                }
                else if((P6IN & BIT4) == 0){
                    while((P6IN & BIT4) == 0){}
                    password_input[password_count] = 0;
                    password_count++;
                }

                // col 3
                P2OUT &= ~BIT3;
                P2OUT |= (BIT0 | BIT2);
                if((P6IN & BIT3) == 0){
                    while((P6IN & BIT3) == 0){}
                    password_input[password_count] = 3;
                    password_count++;
                }
                else if((P6IN & BIT6) == 0){
                    while((P6IN & BIT6) == 0){}
                    password_input[password_count] = 6;
                    password_count++;
                }
                else if((P6IN & BIT5) == 0){
                    while((P6IN & BIT5) == 0){}
                    password_input[password_count] = 9;
                    password_count++;
                }
                else if((P6IN & BIT4) == 0){ // # refresh
                    password_input[0] = 10;
                    password_input[1] = 10;
                    password_input[2] = 10;
                    password_count = 0;
                }

                /* print in segement */
                switch(timer_seg){
                case 1: //7SEG3
                    if(password_input[0] != 10){
                        P3OUT = number[password_input[0]]; // 7SEG3
                        P4OUT = ~(BIT2 | BIT7);

                    }
                    break;
                case 2: //7SEG2
                    if(password_input[1] != 10){
                        P3OUT = number[password_input[1]]; // 7SEG3
                        P4OUT = ~(BIT1 | BIT7);

                    }
                    break;
                case 3: //7SEG1
                    if(password_input[2] != 10){
                        P3OUT = number[password_input[2]]; // 7SEG3
                        P4OUT = ~(BIT0 | BIT7);

                    }
                    break;
                }
            }
            break;
        case 2: // print LOVE

            if(timer_gl < 500) {
                switch(timer_seg){
                case 0: //7SEG3
                    P3OUT = love[1]; // 7SEG3
                    P4OUT = ~(BIT2 | BIT7);

                    break;
                case 1: //7SEG2
                    P3OUT = love[2]; //
                    P4OUT = ~(BIT1 | BIT7);
                    break;
                case 2: //7SEG1
                    P3OUT = love[3]; //
                    P4OUT = ~(BIT0 | BIT7);
                    break;
                case 3: //7SEG4
                    P3OUT = love[0]; //
                    P4OUT = ~(BIT3 | BIT7);
                    break;
                }

                P1OUT &= ~BIT0;
                P4OUT &= ~BIT7;
            }
            else{
                P4OUT = (0x0F | BIT7);
                P1OUT |= BIT0;
            }
            break;

        case 3: // motor function
            P1OUT &= ~BIT0;
            P4OUT &= ~BIT7;
            if(motor_flag == 0){
                TA2CCR1 = (pwm[0] * 100) + (pwm[1]* 10) + (pwm[2]);
                TA2CCR2 = 0;
            }
            else{
                TA2CCR2 = (pwm[0] * 100) + (pwm[1]* 10) + (pwm[2]);
                TA2CCR1 = 0;
            }
            /* segment output */
            switch(timer_gl % 4){
                case 0: //7SEG3
                    P3OUT = number[pwm[0]];
                    P4OUT = ~(BIT2 | BIT7);
                break;
                case 1: //7SEG2
                    P3OUT = number[pwm[1]];
                    P4OUT = ~(BIT1 | BIT7);
                break;
                case 2: //7SEG1
                    P3OUT = number[pwm[2]];
                    P4OUT = ~(BIT0 | BIT7);
                break;
                case 3: //7SEG4 off
                break;
            }
            button_pressed = 0; // for initial condition
            break;
        }
    }
}
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    if((P1IN & BIT1) == 0){
        while((P1IN & BIT1) == 0){}
        // timer_chattering = 500;
        // P1IE &= ~BIT1;
        if(status == 0){ // status = 0 -> double click
            if(button_pressed == 0){ // first click
                timer_gl = 0;
                button_pressed = 1;
            }
            else if (button_pressed == 1){
                status = 1;
                button_pressed = 0;
            }
        }
        else if(status == 1){
            if(password_check(password_input,password)){
                status = 2;
                timer_gl = 0;
            }
        }
        else if(status == 2)
            status = 3;

        else if(status == 3)
            status = 0;
        }



    P1IFG &= ~BIT1;

    if(status != 3){ // position
        if(P1IFG & BIT3) { // encoder intrpt
            if((P1IN & BIT2) != 0) {
                position--;
            } else {
                position++;
            }
        }

        if(P1IFG & BIT2) {
            if((P1IN & BIT3) == 0) {
                position--;
            } else {
                position++;
            }
        }

        if(position > 1200)
            position = 0;
        else if(position < 0)
            position = 1199;


        P1IFG &= ~BIT3;     //flag clear
        P1IFG &= ~BIT2;
    }

    else if(status == 3){ // motor cycle count
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

        if(cycle_counter > 670){
            cycle_counter = 0;
            cycle--; // remain cycle decrease
            if(cycle < 0){
                cycle = 9;
                shiftRight(pwm);
                motor_flag ^= 1;
            }
        }
        P1IFG &= ~BIT3;     //flag clear
        P1IFG &= ~BIT2;
    }

}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void global_timer(void) {

    timer_gl++;


    if(timer_gl > 1000){
        timer_gl = 0;
        if(status == 2){
            shiftRightLove(love);
        }
    }
    else if(timer_gl > 300){
        button_pressed = 0;
    }

    timer_seg++;
    if(timer_seg > 4){
        timer_seg = 0;
    }

    /* motor function */
}
/* #pragma vector=TIMER1_A0_VECTOR
__interrupt void chattering_timer(void) {
    if(timer_chattering != 0){
        timer_chattering--;
    }

    if(timer_chattering == 0){
        P1IE |= BIT1;
    }
} */

void init_segment(){
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3; //dynamic outputs
    P3DIR |= 0xFF; //segment outputs
    P4OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P3OUT &= 0x00;
}

void init_encoder_intrpt(){
    P1IE |= BIT3; //p1.3 intrpt enable
    P1IES &= ~BIT3; //p1.3 edge select to rising edge
    P1IFG &= ~BIT3; //intrpt flag clear
    P1IE |= BIT2; //p.12 intrpt enable
    P1IES &= ~BIT2; //p1.2 edge select to rising edge
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

void init_timer0(){
    TA0CCTL0 = CCIE; //#pragma vector=TIMER0_A0_VECTOR
    TA0CCR0 = 1000;  //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // 1Mhz, to ccr0
}

void init_numpad(){
    P2DIR |= (BIT0 | BIT2 | BIT3); // output (col, 147* p2.2/ 2580 p2.0/ 369# p2.3)
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input (row, 123 p6.3/ 456 p6.6 789 p6.5/ *0# p6.4)
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up, active low

}

/* void init_timer1(){
    TA1CCTL0 = CCIE; // #pragma vector=TIMER1_A0_VECTOR
    TA1CCR0 = 100; //0.1ms, chattering 50ms -> 1ms x 50
    TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK : 1Mhz / Up mode to CCR0

} */
int password_check(int password_input[], int password[]) {
    for (i = 0; i < 3; ++i) {
        if (password_input[i] != password[i]) {
            return 0; // wrong
        }
    }
    return 1; // right
}

void shiftRight(int pwm[3]) {
    int temp = pwm[2];  //

    pwm[2] = pwm[1];
    pwm[1] = pwm[0];
    pwm[0] = temp;
}

void shiftRightLove(int love[4]){
    int temp = love[3];

    love[3] = love[2];
    love[2] = love[1];
    love[1] = love[0];
    love[0] = temp;
}
