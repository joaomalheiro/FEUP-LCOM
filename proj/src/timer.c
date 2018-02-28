#include <minix/syslib.h>
#include <minix/drivers.h>
#include "timer.h"
#include "i8254.h"
#include "game.h"
#include "robinix.h"
/* For swap_buffers() */
#include "video_gr.h"

unsigned int timer_interrupt_counter = 0;
static int hookID = 0;

int timer_set_frequency(unsigned char timer, unsigned long freq) {


	/* Explanation for the minimum frequency being 19Hz:
	 * 1193182Hz is the frequency of the Clock input
	 * 65535 is the biggest number possible, considering an unsigned 16-bit system;
	 * It is, therefore, the biggest number that we can load into the counter of a timer
	 * Thus, the minimum possible frequency is ≃ 18,2068Hz (clock/div = 1193182/65535)
	 * Since we are working with integers, the limit is at 19Hz, since 18 is lower than 18,2.
	 */
	if(freq < 19){
		printf("timer_set_frequency::Frequency is too low! The minimum possible frequency is 19Hz due to system limitations!\n");
		return 1;
	}

	if(timer < 0 || timer > 2){
		printf("timer_set_frequency::Invalid input, timer index out of range!\n");
		return 2;
	}

	//Reading the current configuration of the selected timer

	//Creating variable to store read configuration
	unsigned char readconfig;

	//Reading timer configuration using timer_get_conf
	if(timer_get_conf(timer, &readconfig) != 0){
		printf("timer_set_frequency::Error in reading current timer configuration!\n");
		return 3;
	}

	//Mask to preserve counting mode and operating mode of selected timer
	unsigned char preserveMask = BIT(3) | BIT(2) | BIT(1) | BIT(0);

	//Calculating the control word to load into the timer control register
	/* Control word format in this case:
	 * Counter selection based on timer variable, LSB followed by MSB,
	 * and the previous operating and counting mode gotten by reading
	 * the previous configuration and using a bit mask on it
	 */
	unsigned char controlWord = TIMER_LSB_MSB | (readconfig & preserveMask);

	//Using a switch to input the timer selection into the control word
	switch (timer) {
		case 0:
			controlWord |= TIMER_SEL0;
			break;
		case 1:
			controlWord |= TIMER_SEL1;
			break;
		case 2:
			controlWord |= TIMER_SEL2;
			break;
	}

	//Loading the control word into the timer control register
	if(sys_outb(TIMER_CTRL, controlWord) != 0){
		printf("timer_set_frequency::Error writing control word to timer control register!\n");
		return 4;
	}

	//Calculating the value to load into the counter
	/* From lab2's handout's chapter 3, frequency = clock/div
	 * Thus, div = clock/frequency
	 * (Where frequency is the desired timer output frequency,
 	 *  clock is the frequency of the Clock input for the timers and
 	 *  div is the value to load into the timer)
	 */
	unsigned long initialCounterVal = (unsigned long) TIMER_FREQ / freq;

	//Splitting the value to load into the counter into LSB and MSB
	//(This needs to happen because despite having 16-bit counters, the i8254 only has 8 data lines)

	unsigned char initialCounterVal_LSB = (unsigned char) initialCounterVal;
	unsigned char initialCounterVal_MSB = (unsigned char) (initialCounterVal >> 8);

	//Calculating the address to load into based on timer input variable
	//Since the addresses are sequential, then this is possible
	unsigned char timerCounterAddress = TIMER_0 + timer;

	//Loading the LSB into the counter
	if(sys_outb(timerCounterAddress, initialCounterVal_LSB) != 0){
		printf("timer_set_frequency::Error writing LSB into the counter\n");
		return 5;
	}

	//Loading the MSB into the counter
	if(sys_outb(timerCounterAddress, initialCounterVal_MSB) != 0){
		printf("timer_set_frequency::Error writing MSB into the counter\n");
		return 6;
	}

	//Everything went as expected
	return 0;
}

int timer_subscribe_int() {

	//Variable used for preserving hookID value that will later be used
	int temp = hookID;

	if(sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &hookID) != OK) {
		printf("timer_subscribe_int::Error setting IRQ policy!\n");
		return -1;
	}

	if(sys_irqenable(&hookID)!= OK){
		printf("timer_subscribe_int::Error enabling interrupts on the IRQ line!\n");
		return -2;
	}

	//Everything went as expected
	return BIT(temp);
}

int timer_unsubscribe_int() {

	if(sys_irqdisable(&hookID) != OK){
		printf("timer_unsubscribe_int::Error disabling interrupts on the IRQ line\n");
		return 1;
	}

	if(sys_irqrmpolicy(&hookID) != OK){
		printf("timer_unsubscribe_int::Error unsubscribing the previous subscription of the interrupt notification on the IRQ line!\n");
		return 2;
	}

	//Everything went as expected
	return 0;
}

void timer_IH() {
	Robinix * rob = get_rob();
	//The int handler, when using the game object, serves to call drawing, updating and event handling functions
	if(rob == NULL) {
		//If the game object is NULL, we are not using Events, so we just increment the timer counter
		timer_interrupt_counter++;
	} else {
		//Otherwise call respective update functions
		//For each timer interrupt
		//Drawing the game based on its state
		game_draw(rob);
		//Swapping the buffers since we are drawing in the back buffer (using double buffering)
		swap_buffers();
		//Updating game state
		game_update(rob);
		//Updating game events (to decide if done for every timer interrupt or less often)
		game_process_events(rob);
	}
}

int timer_get_conf(unsigned char timer, unsigned char *st) {

	if(timer < 0 || timer > 2){
		printf("timer_get_conf::Invalid input, timer index out of range!\n");
		return 1;
	}

	//Writing the control word to the control register, to read the timer indicated by 'timer'
	if(sys_outb(TIMER_CTRL, TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)) != 0){
		printf("timer_get_conf::Error writing control word to timer control register!\n");
		return 2;
	}

	//Reading timer status
	//Temporary variable because sys_inb requires an unsigned long pointer as its second argument, despite the output desired only being an unsigned char
	unsigned long temp;

	if(sys_inb(TIMER_0 + timer, &temp) != 0){
		printf("timer_get_conf::Error reading output from timer!\n");
		return 3;
	}

	//Putting the result back in st with the correct typecasting
	*st = (unsigned char) temp;

	//Everything went as expected
	return 0;
}

int timer_display_conf(unsigned char conf) {

	//For bit 0 : counting mode
	if((conf & 0x01) == TIMER_BIN){
		printf("The counting mode is set to binary\n");
	} else if((conf & 0x01) == TIMER_BCD){
		printf("The counting mode is set to BCD\n");
	}

	//For the bits 1, 2 and 3 : operating mode

	//NOTE: Tested and works - However, if the compiler sends a warning, add typecast to int
	int operatingMode = ((conf & (BIT(3) | BIT(2) | BIT(1))) >> 1);

	switch (operatingMode) {
		case 0:
			printf("The timer is set to mode 0: Interrupt on terminal count\n");
			break;
		case 1:
			printf("The timer is set to mode 1: Hardware Retriggerable One-Shot\n");
			break;
		case 2:	case 6:
			printf("The timer is set to mode 2: Rate Generator\n");
			break;
		case 3:	case 7:
			printf("The timer is set to mode 3: Square Wave Mode\n");
			break;
		case 4:
			printf("The timer is set to mode 4: Software Triggered Strobe\n");
			break;
		case 5:
			printf("The timer is set to mode 5: Hardware Triggered Strobe\n");
			break;
		default:
			printf("timer_display_conf::Invalid number received for the operating mode!\n");
			return 1;
			break;
	}

	//For bits 4 and 5 : Type of Access

	unsigned char typeOfAccess = conf & (BIT(4) | BIT(5));

	if(typeOfAccess == TIMER_LSB){
		printf("The type of access is set to LSB\n");
	} else if(typeOfAccess == TIMER_MSB){
		printf("The type of access is set to MSB\n");
	} else if(typeOfAccess == TIMER_LSB_MSB){
		printf("The type of access is set to LSB followed by MSB\n");
	} else if(typeOfAccess == 0x00){
		printf("The type of access is not set yet\n");
	}

	//For bit 6 : Null Count
	if((conf & BIT(6)) == BIT(6)){
		printf("The timer's counter is being loaded with a new value\n");
	}else{
		printf("The timer's counter has been loaded with a new value\n");
	}

	//For bit 7 : Current value of OUT line
	if((conf & BIT(7)) == BIT(7)){
		printf("The timer's OUT line is currently HIGH\n");
	}else{
		printf("The timer's OUT line is currently LOW\n");
	}

	//Everything went as expected
	return 0;
}
