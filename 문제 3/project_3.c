#include <msp430.h> 
void init_timer0();
void init_encoder_intrpt();
void init_segment();
void init_numpad();
int password_check(int get_password[4], int password[4]);

volatile int timer_gl = 0;
volatile int cycle_counter = 5000;
unsigned int number[10] = { 0xDB, 0x50, 0x1F, 0x5D, 0xD4, 0xCD, 0xCF, 0xD8, 0xDF, 0xDD };
unsigned int password[4] = {10, 10, 10, 10}; // initial value > 9999 to prevent error
unsigned int get_password[4]= {0};
volatile unsigned int status = 0;
unsigned int numpad_input = 0;
int i = 0;
volatile unsigned int button_pressed = 0;
unsigned int timer_chattering = 0; // 1000 * 50

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	P2OUT |= BIT1; //p2.1 init
    P2REN |= BIT1;
    P2IE = BIT1; // p2.1 intrp set
    P2IES |= BIT1; //falling edge
    P2IFG &= ~BIT1; //flag clear

    P1OUT |= BIT1; //p1.1 init
    P1REN |= BIT1;
    P1IE |= BIT1; // p1.1 interrupt set
    P1IES &= ~BIT1; // rising edge
    P1IFG &= ~BIT1; //flag clear


    P1DIR |= BIT0; // p1.0 led
    P1OUT &= ~BIT0;


    init_encoder_intrpt();
    init_segment();
    init_numpad();
	init_timer0(); //timer0 initalize

	__bis_SR_register(GIE);

	while(1){
	    switch(status){
	        case 0: //setting password
	            P3OUT = number[cycle_counter % 10];
	            P4OUT = ~BIT0;
	            __delay_cycles(1000);

	            P3OUT = number[cycle_counter / 10 % 10];
	            P4OUT = ~BIT1;
	            __delay_cycles(1000);

	            P3OUT = number[cycle_counter / 100 % 10];
	            P4OUT = ~BIT2;
	            __delay_cycles(1000);

	            P3OUT = number[cycle_counter / 1000 % 10];
	            P4OUT = ~BIT3;
	            __delay_cycles(1000);
	            break;
	        case 1: // enter password
	            P3OUT = number[get_password[3]];
	            P4OUT = ~BIT0;
	            __delay_cycles(1000);

	            P3OUT = number[get_password[2]];
	            P4OUT = ~BIT1;
	            __delay_cycles(1000);

	            P3OUT = number[get_password[1]];
	            P4OUT = ~BIT2;
	            __delay_cycles(1000);

	            P3OUT = number[get_password[0]];
	            P4OUT = ~BIT3;
	            __delay_cycles(1000);
	            break;
	        case 2: // status2, print ----
	            P3OUT = BIT2;
                P4OUT = ~BIT0;
                __delay_cycles(1000);

                P3OUT = BIT2;
                P4OUT = ~BIT1;
                __delay_cycles(1000);

                P3OUT = BIT2;
                P4OUT = ~BIT2;
                __delay_cycles(1000);

                P3OUT = BIT2;
                P4OUT = ~BIT3;
                __delay_cycles(1000);
                break;

	        }


	    }


	
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    if(P1IFG & BIT3) { // encoder intrpt
        if((P1IN & BIT2) != 0) {
            cycle_counter--;
        } else {
            cycle_counter++;
        }
    }

    if(P1IFG & BIT2) {
        if((P1IN & BIT3) == 0) {
            cycle_counter--;
        } else {
            cycle_counter++;
        }
    }

    if(cycle_counter > 9999)
        cycle_counter = 0;
    else if(cycle_counter < 0)
        cycle_counter = 9999;

    P1IFG &= ~BIT3;     //IFG is cleared
    P1IFG &= ~BIT2;     //IFG is cleared

    if(P1IFG & BIT1){
        //while((P1IN & BIT1) == 0){} // chattering
        P1IE &= ~BIT1;
        timer_chattering = 50;
        if(status == 0){
            password[3] = cycle_counter % 10;
            password[2] = cycle_counter / 10 % 10;
            password[1] = cycle_counter / 100 % 10;
            password[0] = cycle_counter / 1000 % 10;
            status = 1;
        }
        P1IFG &= ~BIT1; //flag clear
    }


// chattering
// main - intrpt



}

#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void){ // p2.1 intrpt
    if((P2IN & BIT1) == 0){ // 2.1 double click, first click
        while((P2IN & BIT1) == 0){} // chattering
        // P2IE &= ~BIT1;
        timer_chattering = 50;
        if(button_pressed == 0) { // first click
            timer_gl = 0;
            button_pressed = 1;
            P1OUT |= BIT0;
        }
        else if(button_pressed && (status == 2)){
           button_pressed = 0;
           status = 1;

        }

    }
    P2IFG &= ~BIT1;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void global_timer(void) {
    if(timer_gl > 300){ //global timer, 1ms x 1000 = 1sec
        timer_gl = 0;
        P1OUT &= ~BIT0;
        button_pressed = 0;
    }
    else
        timer_gl++;


    if(timer_chattering != 0){
        timer_chattering--;
    }
    else if(timer_chattering == 0){
        // P2IE |= BIT1;
        P1IE |= BIT1;
    }
    if(numpad_input<4){ // get password via numpad
        P2OUT &= ~BIT2;
        P2OUT |= (BIT0 | BIT3);
        if((P6IN & BIT3) == 0){
            get_password[numpad_input] = 1;
        }
        else if((P6IN & BIT6) == 0){
            get_password[numpad_input] = 4;
        }
        else if((P6IN & BIT5) == 0){
            get_password[numpad_input] = 7;
        }
        else if((P6IN & BIT4) == 0){ // *
            while((P6IN & BIT4) == 0){}

            numpad_input++;
            if (password_check(get_password, password) && (numpad_input == 4)){
                status = 2;
            }
        }

        // columns 2
        P2OUT &= ~BIT0;
        P2OUT |= (BIT2 | BIT3);
        if((P6IN & BIT3) == 0){
            get_password[numpad_input] = 2;
        }
        else if((P6IN & BIT6) == 0){
            get_password[numpad_input] = 5;
        }
        else if((P6IN & BIT5) == 0){
            get_password[numpad_input] = 8;
        }
        else if((P6IN & BIT4) == 0){
            get_password[numpad_input] = 0;
        }

        // columns 3
        P2OUT &= ~BIT3;
        P2OUT |= (BIT0 | BIT2);
        if((P6IN & BIT3) == 0){
            get_password[numpad_input] = 3;
        }
        else if((P6IN & BIT6) == 0){
            get_password[numpad_input] = 6;
        }
        else if((P6IN & BIT5) == 0){
            get_password[numpad_input] = 9;
        }
        else if((P6IN & BIT4) == 0){ // # -> reset
            while((P6IN & BIT4) == 0){}
            get_password[0] = 0;
            get_password[1] = 0;
            get_password[2] = 0;
            get_password[3] = 0;
            numpad_input = 0;
            }


    else if (numpad_input == 4){
        numpad_input = 0;
        get_password[0] = 0;
        get_password[1] = 0;
        get_password[2] = 0;
        get_password[3] = 0;
    }
    }

}




void init_timer0(){
    TA0CCTL0 = CCIE; //#pragma vector=TIMER0_A0_VECTOR
    TA0CCR0 = 1000;  //1000;
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // 1Mhz, up to ccr0
}

void init_segment(){
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3; //dynamic outputs
    P3DIR |= 0xFF; //segment outputs
    P4OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P3OUT &= 0x00;
}

void init_encoder_intrpt(){
    P1IE |= BIT3; //p1.3 intrpt enable
    P1IES &= ~BIT3; //p1.3 edge select to falling edge
    P1IFG &= ~BIT3; //intrpt flag clear
    P1IE |= BIT2; //p.12 intrpt enable
    P1IES &= ~BIT2; //p1.2 edge select to falling edge
    P1IFG &= ~BIT2; //flag clear
}
void init_numpad(){
    P2DIR |= (BIT0 | BIT2 | BIT3); // output (col, 147* p2.2/ 2580 p2.0/ 369# p2.3)
    P2OUT |= (BIT0 | BIT2 | BIT3); // all high

    P6REN |= (BIT3 | BIT4 | BIT5 | BIT6); //input (row, 123 p6.3/ 456 p6.6 789 p6.5/ *0# p6.4)
    P6OUT |= (BIT3 | BIT4 | BIT5 | BIT6); // pull up, active low

}

int password_check(int get_password[4], int password[4]) {
    for (i = 0; i < 4; ++i) {
        if (get_password[i] != password[i]) {
            return 0; // wrong
        }
    }
    return 1; // right
}

