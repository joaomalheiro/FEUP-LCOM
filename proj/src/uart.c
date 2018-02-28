#include "uart.h"
#include "utilities.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <minix/syslib.h>
#include <minix/drivers.h>
//
#include "game.h"
#include "robinix.h"

uart_queue * create_uart_queue() {
	uart_queue * uq = malloc(sizeof *uq);
	if(uq == NULL) {
		return NULL;
	}

	//Setting starting values
	uq->buffer = NULL;
	uq->allocated_size = 0;
	uq->n_elems = 0;
	uq->seq_received = 0;

  return uq;
}

void uart_queue_push(uart_queue * uq, char val) {
	if(uq == NULL) {
		return;
	}

	if(uq->n_elems + 1 > uq->allocated_size) {
		char * new_uq_buf = NULL;
		if(uq->allocated_size == 0) {
			//If size was 0, the requested size would also be 0 which would make no sense so in that case we request 1 element
			new_uq_buf = malloc(1 * sizeof *(uq->buffer));
		} else {
			//No more space in the queue, reallocate to double of previously allocated size (idea taken from C++ STL vectors)
			new_uq_buf = realloc(uq->buffer, uq->allocated_size * 2 * sizeof *(uq->buffer));
		}

		if(new_uq_buf != NULL) {
			//Successfully reallocated, changing values
			uq->buffer = new_uq_buf;
			//Since 2*0 would still be 0
			if(uq->allocated_size == 0) {
				uq->allocated_size = 1;
			} else {
				uq->allocated_size *= 2;
			}

		} else {
			printf("Debug: Error when reallocating uart queue for element insertion!\n");
			return;
		}
	}

	//printf("DBG: inserting val %c in the queue\n", val);

	//Inserting the element
	uq->buffer[uq->n_elems] = val;
  uq->n_elems++;
}

void uart_queue_push_string(uart_queue * uq, char * string) {
  if(uq == NULL || string == NULL || strlen(string) == 0) {
    return;
  }

  uart_queue_push(uq, UART_SEQ_START);
  int i;
  for(i = 0; i < strlen(string); i++) {
    uart_queue_push(uq, string[i]);
  }
  uart_queue_push(uq, UART_SEQ_END);
}

char uart_queue_top(uart_queue * uq) {
  if(uq == NULL || uq->n_elems == 0) {
    return -1;
  }

  return uq->buffer[0];
}

void uart_queue_pop(uart_queue * uq) {
  if(uq == NULL || uq->n_elems == 0) {
    return;
  }

  //Popping an element is shifting every other back by 1
  int i;
  for(i = 0; i < uq->n_elems - 1; i++) {
    SWAP(char, uq->buffer[i], uq->buffer[i+1]);
  }

  //Changing number of elements after shift
  uq->n_elems--;
}

bool is_uart_queue_empty(uart_queue * uq) {
  if(uq == NULL) {
    return true;
  }

  return uq->n_elems == 0;
}

void print_uart_queue(uart_queue * uq) {
  if(uq == NULL) {
    printf("print_uart_queue::queue is NULL!\n");
    return;
  }

  if(uq->n_elems == 0) {
    printf("print_uart_queue::queue is empty!\n");
    return;
  }

  //+1 for \0
  char * result = calloc((uq->n_elems + 1), sizeof *result);
  if(result == NULL) {
    printf("print_uart_queue::Could not allocate result string\n");
    return;
  }

  int i;
  for(i = 0; i < uq->n_elems; i++) {
		//printf("DBG: Element %d : %c\n", i, uq->buffer[i]);
    //Since strcat needs a null terminated string as an argument we create a temporary one to concatenate each character
		strcat(result, (char[2]) {uq->buffer[i], '\0'});
  }

  //There is no problem with having no \0 because when using strcat we are always appending it and we also used calloc to be safe
  printf("print_uart_queue::uart queue contents: %s\n", result);
  free(result);
}

bool uart_queue_has_sequence(uart_queue * uq) {
	if(uq == NULL || uq->n_elems == 0) {
		return false;
	}

	return uq->seq_received > 0;
}

char * uart_queue_get_top_sequence(uart_queue * uq) {
	if(uq == NULL || uq->n_elems == 0 || uq->seq_received == 0) {
		return NULL;
	}

	//At most the result is the size of the queue (+1 for \0)
	//Using calloc to ensure \0s
	//The string is not allocated for a very long time and the queues are emptied quickly so there is not that much unnecessary memory being allocated unnecessarily
	char * result = calloc((uq->n_elems + 1), sizeof *result);
	if(result == NULL) {
		printf("Debug: uart_queue_get_top_sequence could not allocate result string\n");
		return NULL;
	}

	//Getting the sequence
	char topped_char = 0;
	//While we don't get the last character
	while((topped_char = uart_queue_top(uq)) != UART_SEQ_END) {
		//printf("DBG: Topped_char in getting sequence: %c\n", topped_char);
		if(topped_char != UART_SEQ_START) {
			//If it is not the sequence start character then concatenate it to the result
			//Since strcat must receive a null terminated string to concatenate, we generate a temporary one
			strcat(result, (char[2]){topped_char, '\0'});
		}
		//printf("DBG: current result: %s\n\n", result);
		uart_queue_pop(uq);
		if(is_uart_queue_empty(uq)) {
			printf("Debug: UART queue is empty! Cannot be popped anymore for creating sequence string!\n");
			free(result);
			return NULL;
		}
	}

	//Popping the final character so that the sequence end is removed as well (otherwise we would get stuck here)
	uart_queue_pop(uq);

	//One sequence was successfully extracted and thus we can decrement the sequence received variable
	uq->seq_received--;

	//The elements inserted into the result string were already popped from the queue

	return result;
}

void destroy_uart_queue(uart_queue ** uq) {
  if(*uq == NULL) {
    return;
  }

	/*
	//Freeing each element of the buffer
	int i;
	for(i = 0; i < (*uq)->n_elems; i++) {
		free((*uq)->buffer[i]);
	}
	*/

  ////////Freeing the buffer itself (char ** was used so this must be done too)
  free((*uq)->buffer);
  //Freeing the object
  free(*uq);
  *uq = NULL;
}

static int uart_hookID = 4;
static uart_queue * send_queue = NULL;
static uart_queue * receive_queue = NULL;
//If the UART rceived an interrupt but did not have data to send at the time, this bool is set so that when adding data to the buffer this can be operated on
static bool can_send = false;

int uart_subscribe_int() {

  //Deallocating previous queues if they exist
  if(send_queue != NULL) {
    destroy_uart_queue(&send_queue);
  }
  if(receive_queue != NULL) {
    destroy_uart_queue(&receive_queue);
  }

  //Variable used for preserving uart_hookID value that will later be used
	int temp = uart_hookID;

	if(sys_irqsetpolicy(UART_COM1_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &uart_hookID) != OK) {
		printf("uart_subscribe_int::Error setting IRQ policy!\n");
		return -1;
	}

	if(sys_irqenable(&uart_hookID)!= OK){
		printf("uart_subscribe_int::Error enabling interrupts on the IRQ line!\n");
		return -2;
	}

  if(uart_set_desired_interrupts() != 0) {
    printf("uart_subscribe_int::Error setting desired interrupts in the IER\n");
    return -3;
  }

  if(uart_enable_FIFO() != 0) {
    printf("uart_subscribe_int::Error enabling FIFO\n");
    return -4;
  }

  //Allocating queues
  send_queue = create_uart_queue();
  receive_queue = create_uart_queue();
  if(send_queue == NULL || receive_queue == NULL) {
    destroy_uart_queue(&send_queue);
    destroy_uart_queue(&receive_queue);
    return -5;
  }

	//Everything went as expected, returning bitmask of the uart_hookID for interrupt handling
	return BIT(temp);
}

static int uart_enable_rd_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier |= UART_IER_RD;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_enable_te_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier |= UART_IER_TE;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_enable_rls_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier |= UART_IER_RLS;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_disable_rd_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier &= ~UART_IER_RD;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_disable_te_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier &= ~UART_IER_TE;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_disable_rls_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier &= ~UART_IER_RLS;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

static int uart_disable_ms_int() {
  unsigned long ier = 0;
  //Reading IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &ier) != 0) {
    return -1;
  }

  //Changing desired bit
  ier &= ~UART_IER_MS;

  //Writing IER
  if(sys_outb(UART_COM1_BASE_ADDR + UART_IER_ADDR, ier) != 0) {
    return -2;
  }

  return 0;
}

int uart_set_desired_interrupts() {
  if(uart_enable_rd_int() != 0) {
    return -1;
  }

  if(uart_enable_te_int() != 0) {
    return -2;
  }

  if(uart_enable_rls_int() != 0) {
    return -3;
  }

  if(uart_disable_ms_int() != 0) {
    return -4;
  }

  return 0;
}

int uart_unset_desired_interrupts() {
  if(uart_disable_rd_int() != 0) {
    return -1;
  }

  if(uart_disable_te_int() != 0) {
    return -2;
  }

  if(uart_disable_rls_int() != 0) {
    return -3;
  }

  return 0;
}

int uart_enable_FIFO() {
  //Setting FCR configuration, FIFO enabled, clearing receive and transmit FIFOs, and using a trigger level of 4
  unsigned long fcr = UART_FCR_EF | UART_FCR_CRF | UART_FCR_CTF | UART_FCR_ITL_4;
  if(sys_outb(UART_COM1_BASE_ADDR + UART_FCR_ADDR, fcr) != 0) {
    printf("uart_enable_FIFO::Error writing FIFO configuration!\n");
    return -1;
  }

  return 0;
}

int uart_disable_FIFO() {
  //Setting FCR configuration, FIFO disabled, clearing receive and transmit FIFOs
  unsigned long fcr = UART_FCR_CRF | UART_FCR_CTF;
  if(sys_outb(UART_COM1_BASE_ADDR + UART_FCR_ADDR, fcr) != 0) {
    printf("uart_enable_FIFO::Error writing FIFO configuration!\n");
    return -1;
  }

  return 0;
}

int uart_unsubscribe_int() {

  destroy_uart_queue(&send_queue);
  destroy_uart_queue(&receive_queue);

	if(sys_irqdisable(&uart_hookID) != OK) {
		printf("uart_unsubscribe_int::Error disabling interrupts on the IRQ line\n");
		return 1;
	}

	if(sys_irqrmpolicy(&uart_hookID) != OK) {
		printf("uart_unsubscribe_int::Error removing the previous subscription of the interrupt notification on the IRQ line!\n");
		return 2;
	}

  if(uart_unset_desired_interrupts() != 0) {
    printf("uart_unsubscribe_int::Error unsetting set interrupts in the IER!\n");
		return 3;
  }

  if(uart_disable_FIFO() != 0) {
    printf("uart_unsubscribe_int::Error disabling FIFO!\n");
		return 4;
  }

	//Everything went as expected
	return 0;
}

void uart_get_conf() {
  unsigned long tempvar = 0;
  //IER
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IER_ADDR, &tempvar) != 0) {
    printf("uart_read_conf::Error reading IER\n");
  } else {
    printf("uart_read_conf::IER: %02X\n", tempvar);
  }

  //LCR
  if(sys_inb(UART_COM1_BASE_ADDR + UART_LCR_ADDR, &tempvar) != 0) {
    printf("uart_read_conf::Error reading LCR\n");
  } else {
    printf("uart_read_conf::LCR: %02X\n", tempvar);
  }

  //LSR
  if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &tempvar) != 0) {
    printf("uart_read_conf::Error reading LSR\n");
  } else {
    printf("uart_read_conf::LSR: %02X\n", tempvar);
  }

  //FCR
  if(sys_inb(UART_COM1_BASE_ADDR + UART_FCR_ADDR, &tempvar) != 0) {
    printf("uart_read_conf::Error reading FCR\n");
  } else {
    printf("uart_read_conf::FCR: %02X\n", tempvar);
  }
}

void uart_set_DLAB_status(unsigned char status) {
  unsigned long lcr = 0;
  if(sys_inb(UART_COM1_BASE_ADDR + UART_LCR_ADDR, &lcr) != 0) {
    printf("uart_set_DLAB_status::Error reading LCR\n");
    return;
  }
  if(status) {
    //If status is not 0 then set DLAB to 1
    lcr |= UART_LCR_DLAB;
  } else {
    //If status is 0 then unset DLAB
    lcr &= ~UART_LCR_DLAB;
  }

  if(sys_outb(UART_COM1_BASE_ADDR + UART_LCR_ADDR, lcr) != 0) {
    printf("uart_set_DLAB_status::Error writing new DLAB status\n");
    return;
  }
}

int uart_set_conf(unsigned int n_bpc, unsigned int n_stop_bits, unsigned char parity) {
  //Just in case for the future, setting DLAB access to 0
  uart_set_DLAB_status(0);

  //Expects the parity argument to be in the correct binary value, the others are decimal human input value

  //Verifying input
  if(n_bpc < 5 || n_bpc > 8 || n_stop_bits < 1 || n_stop_bits > 2 || parity > 7) {
    printf("uart_set_conf::Invalid arguments!\n");
    return -1;
  }

  //Reading current uart configuration
  unsigned long uart_conf = 0;
  if(sys_inb(UART_COM1_BASE_ADDR + UART_LCR_ADDR, &uart_conf) != 0) {
    printf("uart_set_conf::Error reading current UART configuration!\n");
    return -2;
  }

  //Setting the passed values
  //Number of bits per char
  //Since the 0s are not extended automatically, we first clear the respective part of the configuration
  //(by unsetting the correct bits)
  uart_conf &= ~UART_LCR_NBPC;
  uart_conf |= (n_bpc - 5);
  //Number of stop bits
  uart_conf |= (n_stop_bits - 1) << 2;
  //Parity control
  //Since the 0s are not extended automatically, we first clear the respective part of the configuration
  //(by unsetting the parity control bits)
  uart_conf &= ~UART_LCR_PAR_CTRL;
  uart_conf |= parity << 3;

  //Writing the changed register
  if(sys_outb(UART_COM1_BASE_ADDR + UART_LCR_ADDR, uart_conf) != 0) {
    printf("uart_set_conf::Error writing UART configuration!\n");
    return -3;
  }

  return 0;
}

int uart_set_rate(unsigned int rate) {
  //rate = Divisor / DL_val
  //DL_val = Divisor / rate
  if(rate == 0 || rate > UART_DIVISOR) {
    printf("uart_set_rate::Can't set the passed rate!\n");
    return -1;
  }
  unsigned int DL_val = UART_DIVISOR / rate;
  //Setting DLAB access to activated
  uart_set_DLAB_status(1);

  //Splitting DL_val into LSB and MSB
  unsigned long lsb = GET_LSBYTE(DL_val);
  unsigned long msb = GET_MSBYTE(DL_val);

  //Writing the values calculated
  //LSB
  if(sys_outb(UART_COM1_BASE_ADDR + UART_DLL_ADDR, lsb) != 0) {
    printf("uart_set_rate::Error writing DL LSB!\n");
    return -2;
  }
  //MSB
  if(sys_outb(UART_COM1_BASE_ADDR + UART_DLM_ADDR, msb) != 0) {
    printf("uart_set_rate::Error writing DL MSB!\n");
    return -3;
  }

  //Resetting DLAB access
  uart_set_DLAB_status(0);

  return 0;
}

void uart_get_rate() {
  unsigned long lsb = 0;
  unsigned long msb = 0;

  //Activating DLAB access
  uart_set_DLAB_status(1);

  //Reading the values of the DLAB
  //LSB
  if(sys_inb(UART_COM1_BASE_ADDR + UART_DLL_ADDR, &lsb) != 0) {
    printf("uart_get_rate::Error reading DL LSB!\n");
    return;
  }
  //MSB
  if(sys_inb(UART_COM1_BASE_ADDR + UART_DLM_ADDR, &msb) != 0) {
    printf("uart_get_rate::Error reading DL MSB!\n");
    return;
  }
  //Resetting DLAB access, for reading is done
  uart_set_DLAB_status(0);

  //Joining the values
  unsigned int DL_val = JOIN_LSB_MSB(lsb, msb);
  printf("uart_get_rate::Current DLAB value: %u\n", DL_val);

  if(DL_val == 0) {
    printf("uart_get_rate::Can't calculate rate, DLAB value was 0!\n");
    return;
  }

  //Calculating rate
  //rate = Divisor / DL_val
  unsigned int rate = UART_DIVISOR / DL_val;
  printf("uart_get_rate::Current rate: %u\n", rate);
}

int uart_IH() {
  //Reading IIR
  unsigned long iir = 0;
  if(sys_inb(UART_COM1_BASE_ADDR + UART_IIR_ADDR, &iir) != 0) {
    printf("uart_IH::Error reading IIR\n");
    return -1;
  }

  //printf("DBG: IIR: %x\n", iir);

  //Checking if interrupt is pending
  if(iir & UART_IIR_IP) {
		//If IP is active, no interrupt is pending
		//These were debug messages, now disabled due to causing too much "spam" (we are sure we are always enabling the FIFO correctly now)
		/*
    switch(iir & UART_IIR_FS_MASK) {
      case UART_IIR_FS_NF:
        printf("uart_IH::No FIFO!\n");
        break;
      case UART_IIR_FS_U:
        printf("uart_IH::FIFO Unusable!\n");
        break;
      case UART_IIR_FS_E:
        printf("uart_IH::FIFO correctly enabled!\n");
        break;
    }
		*/
    //If the Interrupt Pending bit is active then no interrupt is pending, return
    return 0;
  }

  //Seeing which interrupt ocurred
  switch (iir & UART_IIR_IO_MASK) {
    case UART_IIR_IO_MS:
      //Modem Status Interrupt - should not happen since we are disabling this interrupt
      printf("uart_IH::Received Modem Status interrupt\n");
      break;
    case UART_IIR_IO_TE:
      //Transmitter Empty Interrupt
      //printf("DBG: Received transmitter empty interrupt\n");
      if(is_uart_queue_empty(send_queue)) {
        //If we don't have anything to send at the moment we should save that state to know for when we have
        can_send = true;
      } else {
        //Send current queue contents here directly
        if(uart_send() != 0) {
					printf("uart_IH::Error in sending\n");
					return -2;
				}
      }
      break;
    case UART_IIR_IO_CTI:
      //Character Timeout Indication Interrupt
			//This ocurrs if no characters have been removed from or input to the receiver FIFO during the last 4 char. times and there is at least 1 char in it during this time
			//Thus, there is something in the receiver FIFO available to read but the trigger level was not met and probably will not be met due to the time already passed (4 char times)
			//So, we can handle this time of interrupt just by reading the receiver FIFO, just like in Received Data Available interrupts
      //printf("DBG: Received Character Timeout Indication Interrupt\n");
			if(uart_receive() != 0) {
				printf("uart_IH::Error in receiving\n");
				return -3;
			}
      break;
    case UART_IIR_IO_RDA:
      //Received Data Available Interrupt
      //printf("DBG: Received received data available interrupt\n");
			if(uart_receive() != 0) {
				printf("uart_IH::Error in receiving\n");
				return -4;
			}
      break;
    case UART_IIR_IO_LS:
      //Line Status Interrupt
      //printf("DBG: Received Line Status Interrupt\n");
			if(uart_error_handler() != 0) {
				printf("uart_IH::Error in handling errors!\n");
				return -5;
			}
      break;
    default:
      printf("uart_IH::Received unknown interrupt. IIR: %x\n", iir);
      break;
  }

  return 0;
}

int uart_send() {
	//Verifying using the Line Status Register if the Transmitting Holding Register is empty (respective bit must be active)
	unsigned long lsr = 0;
	if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
		printf("uart_send::Error reading the LSR\n");
		return -1;
	}

	if((lsr & UART_LSR_THRE) == 0) {
		printf("uart_send::Transmitter Holding Register was not empty!\n");
		return -2;
	}

	//Writing the topmost character in the queue to the buffer
	if(send_queue == NULL || send_queue->n_elems == 0) {
		printf("uart_send::Queue was empty or NULL!\n");
		return -3;
	}

	//printf("DBG:Queue contents before sending character:\n");
	//print_uart_queue(send_queue);

	char c_to_write = uart_queue_top(send_queue);
	//Popping the queue since the top is being written
	uart_queue_pop(send_queue);

	//Resetting the can_send flag to false
	can_send = false;

	//printf("DBG:uart_send::Sending character %c through UART\n", c_to_write);

	//Write the character to the THR
	if(sys_outb(UART_COM1_BASE_ADDR + UART_THR_ADDR, c_to_write)) {
		printf("uart_send::Error sending character to THR!\n");
		return -4;
	}
	//printf("DBG:Queue contents after sending character:\n");
	//print_uart_queue(send_queue);

	//No error ocurred, everything went as expected
	return 0;
}

void uart_send_string(char * string) {
  if(string == NULL || strlen(string) == 0) {
    return;
  }
	//printf("DBG: Pushing string\n");

	//Pushes the string to the send queue with the correct start and end characters
  uart_queue_push_string(send_queue, string);

	//printf("DBG: New queue contents:\n");
	//print_uart_queue(send_queue);

  if(can_send) {
    //If we received an interrupt previously about the buffer to send being empty we can attempt to send directly
    if(uart_send() != 0) {
      printf("uart_send_string::Error in uart_send()\n");
      return;
    }
  } else {
		//Because we might have missed a THR Empty interrupt, we quickly check the LSR and if sending is possible (THR Empty) we send
		//This should kickstart the remaining interrupts process
		unsigned long lsr = 0;
		if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
			printf("uart_send_string::Error checking LSR\n");
			return;
		}

		if(lsr & UART_LSR_THRE) {
			//Only "force send" like this if there is nothing to receive, otherwise we are almost asking for an Overrun Error
			if(lsr & UART_LSR_RD) {
				//If there is data to read, do nothing, as reading data should have higher priority!!
				return;
			}
			//It is possible to send
			uart_send();
		}
	}

  //If we cannot send we wait until the next buffer empty interrupt
}

int uart_error_handler() {
	//Reading LSR to see which error ocurred
	unsigned long lsr = 0;
	if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
		printf("uart_error_handler::Error in reading LSR\n");
		return -1;
	}

	printf("uart_error_handler::LSR: %x\n", lsr);

	if(lsr & UART_LSR_RD) {
		printf("uart_error_handler::There is data available for receiving\n");
	}

	if(lsr & UART_LSR_OE) {
		printf("uart_error_handler::Overrun Error ocurred\n");
		//In case of an Overrun Error the correct behaviour is to clear the receiver buffer, for the data there is considered invalid
		//Thus we read it until there is nothing in the buffer and "trash" the contents
		if(uart_clear_receiver_buffer() != 0) {
			printf("uart_error_handler::Error in clearing receiver buffer after Overrun Error\n");
		}
	}

	if(lsr & UART_LSR_PE) {
		printf("uart_error_handler::Parity Error ocurred\n");
	}

	if(lsr & UART_LSR_FE) {
		printf("uart_error_handler::Framing Error ocurred\n");
	}

	if(lsr & UART_LSR_BI) {
		printf("uart_error_handler::Break Interrupt ocurred\n");
	}

	if(lsr & UART_LSR_THRE) {
		printf("uart_error_handler::Transmitter Holding Register Empty\n");
	}

	if(lsr & UART_LSR_TER) {
		printf("uart_error_handler::Transmitter Holding Register and Transmitter Shift Register empty\n");
	}

	if(lsr & UART_LSR_FIFOE) {
		printf("uart_error_handler::Parity or framing error or break indication in the FIFO\n");
		//To solve this, we clear the FIFO by reading it and "trashing the contents"
		if(uart_clear_receiver_buffer() != 0) {
			printf("uart_error_handler::Error in clearing receiver buffer after FIFO Error\n");
		}
	}

	return 0;
}

int uart_clear_receiver_buffer() {
	//Initial verification if the Receive Data bit in the LSR is set (There is data available to read)

	unsigned long lsr = 0;
	if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
		printf("uart_clear_receiver_buffer::Error reading the LSR\n");
		return -1;
	}

	if((lsr & UART_LSR_RD) == 0) {
		printf("Debug: uart_clear_receiver_buffer::Receive Data was not set\n");
		//This is not an error, the buffer was simply already emptied
		return 0;
	}

	//To clear the FIFO, we must read while Receiver Data in the LSR is active
	while(lsr & UART_LSR_RD) {
		//Reading a character in the FIFO, and not pushing it to the receiver queue (trashing it)

		unsigned long c_received = 0;
		//Reading received character
		if(sys_inb(UART_COM1_BASE_ADDR + UART_RBR_ADDR, &c_received) != 0) {
			printf("uart_clear_receiver_buffer::Error receiving character from RBR!\n");
			return -4;
		}

		//printf("uart_clear_receiver_buffer::DBG: Received character %c\n", (char) c_received);

		//Updating the LSR to know if continuing
		if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
			printf("uart_clear_receiver_buffer::Error re-reading the LSR\n");
			return -1;
		}
	}

	printf("uart_clear_receiver_buffer::RB successfully cleared!\n");

	//No error ocurred, everything went as expected
	return 0;
}

int uart_receive() {
	//Initial verification if the Receive Data bit in the LSR is set (There is data available to read)

	unsigned long lsr = 0;
	if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
		printf("uart_receive::Error reading the LSR\n");
		return -1;
	}

	if((lsr & UART_LSR_RD) == 0) {
		printf("uart_receive::Receive Data was not set!\n");
		return -2;
	}

	//Since we are using FIFO, we must read while Receiver Data in the LSR is active
	while(lsr & UART_LSR_RD) {
		////Pushing the received character into the queue
		if(receive_queue == NULL) {
			printf("uart_receive::Queue was NULL!\n");
			return -3;
		}

		//printf("DBG:Queue contents before receiving character:\n");
		//print_uart_queue(receive_queue);

		unsigned long c_received = 0;
		//Reading received character
		if(sys_inb(UART_COM1_BASE_ADDR + UART_RBR_ADDR, &c_received) != 0) {
			printf("uart_receive::Error receiving character from RBR!\n");
			return -4;
		}

		//printf("DBG: Received character %c\n", (char) c_received);

		//Pushing the received character into the receiver queue
		uart_queue_push(receive_queue, (char) c_received);

		if((char)c_received == UART_SEQ_END) {
			//If a sequence end was received, then the queue has a full sequence
			receive_queue->seq_received++;
		}

		//printf("DBG:Queue contents after receiving character:\n");
		//print_uart_queue(receive_queue);

		//Updating the LSR to know if continuing
		if(sys_inb(UART_COM1_BASE_ADDR + UART_LSR_ADDR, &lsr) != 0) {
			printf("uart_receive::Error re-reading the LSR\n");
			return -1;
		}
	}


	while(uart_queue_has_sequence(receive_queue)){
		char * seq = uart_queue_get_top_sequence(receive_queue);
		if(seq == NULL) {
			printf("NULL sequence extracted from the uart queue\n");
			continue;
		}
		//printf("DBG: Received string \"%s\" through UART\n", seq);

		Robinix * rob = get_rob();

		if(rob == NULL) {
			//If rob is null the game is not running in play mode so we just print the string as debug and free it afterwards
			printf("Testing: Received string \"%s\" through UART\n", seq);
			free(seq);
			continue;
		}

		//If rob is not null we are playing the game and thus we add the event to the queue
		add_event_to_buffer(rob, create_event(RECEIVED_REMOTE_MESSAGE, 0, 0, '?', seq));

		//printf("DBG: Queue contents after removing string:\n");
		//print_uart_queue(receive_queue);
	}


	//No error ocurred, everything went as expected
	return 0;
}
