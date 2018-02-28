#ifndef __UART_H
#define __UART_H

#include "utilities.h"

/** @defgroup uart uart
 * @{
 *
 * Base implementation of the UART Device Driver
 */

////START OF UART DEFINES

//We are only using the COM1 so COM2 details are ommitted
#define UART_COM1_BASE_ADDR			0x3F8
#define UART_COM1_IRQ			4
/* NOTE: All other addresses are shifts upon the COM base addresses above */
#define UART_RBR_ADDR			0 /* Receiver Buffer Register */
#define UART_THR_ADDR			0 /* Transmitter Holding Register */
#define UART_IER_ADDR			1 /* Interrupt Enable Register */
#define UART_IIR_ADDR			2 /* Interrupt Identification Register */
#define UART_FCR_ADDR			2 /* FIFO Control Register */
#define UART_LCR_ADDR			3 /* Line Control Register */
#define UART_MCR_ADDR			4 /* Modem Control Register */
#define UART_LSR_ADDR			5 /* Line Status Register */
#define UART_MSR_ADDR			6 /* Modem Status Register */
#define UART_SR_ADDR			7 /* Scratchpad Register */
/* If the DLAB bit of the LCR register is set to 1 then we are instead accessing these: */
#define UART_DLL_ADDR			0 /* Divisor Latch LSB */
#define UART_DLM_ADDR			1 /* Divisor Latch MSB */

/* Line Control Register (LCR) R/W */
#define UART_LCR_NBPC           (BIT(1) | BIT(0)) /* Number of bits per char, use this as mask then 0 is 5, 1 is 6, 2 is 7, 3 is 8 */
#define UART_LCR_8BPC           (BIT(1) | BIT(0)) /* 8 bits per char setting */
#define UART_LCR_SB             BIT(2) /* Number of stop bits, if set is 2, unset is 1 except when 5 bits char */
#define UART_LCR_PAR_CTRL		   	(BIT(5) | BIT(4) | BIT(3)) /* Parity control, use as mask then shift 3 right and the sequence is: no parity, odd parity, even parity, always 1, always 0 */
#define UART_LCR_PAR_ODD			  BIT(3) /* Odd Parity */
#define UART_LCR_PAR_EVEN			  (BIT(4) | BIT(3)) /* Even Parity */
#define UART_LCR_BC					    BIT(6) /* Break Control, sets serial output to low */
#define UART_LCR_DLAB				    BIT(7) /* Divisor Latch Access, if set then accessing divisor latch, if unset accessing RBR and THR */

/* Line Status Register (LSR) R */
#define UART_LSR_RD				  BIT(0) /* Receiver Data, set to 1 when there is data for receiving */
#define UART_LSR_OE				  BIT(1) /* Overrun Error, set to 1 when a character received is overwritten by another one */
#define UART_LSR_PE				  BIT(2) /* Parity Error, set to 1 when a character with a parity error is received */
#define UART_LSR_FE				  BIT(3) /* Framing Error, set to 1 when a received character does not have a valid Stop bit */
#define UART_LSR_BI				  BIT(4) /* Break Interrupt, set to 1 when the serial data input line is held in the low level for longer than a full "word" transmission */
#define UART_LSR_THRE			  BIT(5) /* Transmitter Holding Register Empty, when set means that the UART is ready to accept a new character for transmitting */
#define UART_LSR_TER			  BIT(6) /* Transmitter Empty Register, when set, means that both the THR and the Transmitter Shift Register are both empty */
#define UART_LSR_FIFOE			BIT(7) /* FIFO Error, Set to 1 when there is at least one parity error or framing error or break indication in the FIFO */
/* Bits 0 to 4 are reset when LSR is read. Bit 7 as well when using FIFO, supposedly */

/* Interrupt Enable Register (IER) R/W */
#define UART_IER_RD          BIT(0) /* Enable Received Data interrupt */
#define UART_IER_TE          BIT(1) /* Enable Transmitter Empty interrupt */
#define UART_IER_RLS         BIT(2) /* Enable Receiver Line Status interrupt */
#define UART_IER_MS          BIT(3) /* Enable Modem Status interrupt */
/* The remainder are non relevant and reserved bits */

/* Interrupt Identification Register (IIR) R */
#define UART_IIR_IP         BIT(0) /* Interrupt Status, to see if Interrupt Pending - 0 means pending, 1 means not pending */
#define UART_IIR_IO_MASK    (BIT(3) | BIT(2) | BIT(1)) /* Interrupt Origin Bitmask */
#define UART_IIR_IO_MS      0 /* Interrupt Origin: Modem Status */
#define UART_IIR_IO_TE      BIT(1) /* Interrupt Origin: Transmitter Empty */
#define UART_IIR_IO_CTI     (BIT(3) | BIT(2)) /* Interrupt Origin: Character Timeout Indication */
#define UART_IIR_IO_RDA     BIT(2) /* Interrupt Origin: Received Data Available */
#define UART_IIR_IO_LS      (BIT(2) | BIT(1)) /* Interrupt Origin: Line Status */
//Bit 4 is reserved
//Bit 5 is for 64-byte FIFOs, which ours is not
#define UART_IIR_FS_MASK    (BIT(7) | BIT(6)) /* FIFO Status Bitmask */
#define UART_IIR_FS_NF      0 /* FIFO Status: No FIFO */
#define UART_IIR_FS_U       BIT(7) /* FIFO Status: Unusable */
#define UART_IIR_FS_E       (BIT(7) | BIT(6)) /* FIFO Status: Enabled */

/* FIFO Control Register (FCR) R/W */
#define UART_FCR_EF             BIT(0) /* Enable FIFO */
#define UART_FCR_CRF            BIT(1) /* Clear Receive FIFO */
#define UART_FCR_CTF            BIT(2) /* Clear Transmit FIFO */
//Bit 3 is not relevant (DMA Mode Select), only for some UARTs
//Bit 4 is reserved
//Bit 5 is enable 64-byte FIFO, which ours is not
#define UART_FCR_ITL_MASK       (BIT(7) | BIT(6)) /* Interrupt Trigger Level Bitmask */
#define UART_FCR_ITL_1          0 /* Interrupt Trigger Level : 1 */
#define UART_FCR_ITL_4          BIT(6) /* Interrupt Trigger Level : 4 */
#define UART_FCR_ITL_8          BIT(7) /* Interrupt Trigger Level : 8 */
#define UART_FCR_ITL_14         (BIT(7) | BIT(6)) /* Interrupt Trigger Level : 14 */

/* Other */
#define UART_DIVISOR 115200
//Our self defined start and end character for a sequence
#define UART_SEQ_START  '#'
#define UART_SEQ_END    '!'

////END OF UART DEFINES


#include <stdbool.h>

////Queue implementation

typedef struct {
	char * buffer;
	unsigned int allocated_size;
	unsigned int n_elems;
	unsigned int seq_received;
} uart_queue;

/**
 * @brief Creates a queue of characters, for use with the UART
 * @return Returns a pointer to a valid uart_queue or NULL in case of error
 */
uart_queue * create_uart_queue();

/**
 * @brief Pushes the passed element to the passed queue
 * @param uq  Queue to push the element into
 * @param val Element to push into the queue
 */
void uart_queue_push(uart_queue * uq, char val);

/**
 * @brief Pushes a string into a queue, complete with start and end characters (for use in sending messages with the UART)
 * @param uq     Queue to push the string into
 * @param string String to push into the queue
 */
void uart_queue_push_string(uart_queue * uq, char * string);

/**
 * @brief Returns the top of the queue
 * @param  uq Queue to check the top of
 * @return    Returns the top element of the queue or -1 if it is empty or NULL
 */
char uart_queue_top(uart_queue * uq);

/**
 * @brief Pops the top element of the queue
 * @param uq Queue to pop the top off of
 */
void uart_queue_pop(uart_queue * uq);

/**
 * @brief Checks if a uart_queue is empty or not
 * @param  uq Queue to check for emptiness
 * @return    true if the queue is empty, false if not
 */
bool is_uart_queue_empty(uart_queue * uq);

/**
 * @brief Converts the contents of the queue into a string and prints them on the screen using printf
 * @param uq Queue to print
 */
void print_uart_queue(uart_queue * uq);

/**
 * @brief Checks if a uart_queue contains a sequence
 * @param  uq Queue to check
 * @return    Returns true if the queue has a sequence, false if not
 */
bool uart_queue_has_sequence(uart_queue * uq);

/**
 * @brief Gets the topmost sequence of the queue
 * @param  uq Queue to get the sequence off of
 * @return    A pointer to the topmost sequence without the sequence start and end characters or NULL in case of failure
 */
char * uart_queue_get_top_sequence(uart_queue * uq);

/**
 * @brief Destroys a uart_queue by deleting its elements and freeing all the memory used
 * @param uq Queue to destroy
 */
void destroy_uart_queue(uart_queue ** uq);

////UART DD

/**
 * @brief Subscribes the UART peripheral
 * @return Returns the bitmask to use for checking interrupts with or -1 through -5 in case of error
 */
int uart_subscribe_int();

/**
 * @brief Sets the desired interrupts in the program
 * @return 0 in case of success, not 0 otherwise
 */
int uart_set_desired_interrupts();

/**
 * @brief Unsets the previously set interrupts
 * @return 0 if successful, not 0 if not
 */
int uart_unset_desired_interrupts();

/**
 * @brief Enables the use of the hardware FIFO
 * @return 0 if successful, not 0 if not
 */
int uart_enable_FIFO();

/**
 * @brief Disables the hardware FIFO
 * @return 0 if successful, not 0 if not
 */
int uart_disable_FIFO();

/**
 * @brief Unsubscribes the UART peripheral
 * @return 0 if successful, not 0 otherwise
 */
int uart_unsubscribe_int();

/**
 * @brief Displays the current UART configuration on the screen using printf
 */
void uart_get_conf();

/**
 * @brief Activates or deactivates DLAB access
 * @param status 0 to deactivate DLAB access, not 0 to activate
 */
void uart_set_DLAB_status(unsigned char status);

/**
 * @brief Configures the UART with the passed arguments
 * @param  n_bpc       Number of bits per character
 * @param  n_stop_bits Number of stop bits
 * @param  parity      Type of parity checking, a correctly formatted 3 bit value must be passed
 * @return             0 if successful, not 0 otherwise
 */
int uart_set_conf(unsigned int n_bpc, unsigned int n_stop_bits, unsigned char parity);

/**
 * @brief Sets the rate of the UART to that of the argument. Note: works better with "standard values"
 * @param  rate Rate to set the UART to
 * @return      0 if successful, not 0 otherwise
 */
int uart_set_rate(unsigned int rate);

/**
 * @brief Displays the current UART rate on the screen using printf
 */
void uart_get_rate();

/**
 * @brief UART interrupt handler
 * @return 0 if successful, not 0 in case of error
 */
int uart_IH();

/**
 * @brief Used by the interrupt handler to send characters through the UART
 * @return 0 if successful, not 0 otherwise
 */
int uart_send();

/**
 * @brief Sending interface to call externally from the UART functions. Sends the passed string, correctly encoded. If not possible to send straigh away, it is added to the queue.
 * @param string String to send
 */
void uart_send_string(char * string);

/**
 * @brief Error handler to be called by the interrupt handler
 * @return 0 if successful, not 0 otherwise
 */
int uart_error_handler();

/**
 * @brief Clears the UART's receiver buffer by reading and scrapping contents. To use in case of Overrun Error
 * @return 0 if successful, not 0 otherwise
 */
int uart_clear_receiver_buffer();

/**
 * @brief Used by the interrupt handler to receive characters
 * @return 0 if successful, not 0 otherwise
 */
int uart_receive();

#endif /* __UART_H */
