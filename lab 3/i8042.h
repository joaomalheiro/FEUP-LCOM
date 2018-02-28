#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

/** @defgroup i8042 i8042
 * @{
 *
 * Constants that are helpful for programming the i8042 keyboard controller.
 */

 #define BIT(n) (0x01<<(n)) /**< @brief Returns the bit number n active using left bitshifts */

 #define KBC_IRQ 1

 ///KBC Addresses
 #define KBC_STATUS_REG 0x64
 #define KBC_CMD_REG 0x64 /** "'Not named' for writing KBC commands" */
 #define KBC_OUT_BUF 0x60
 #define KBC_IN_BUF 0x60

 #define KBC_RETRY_DELAY 20000 /** Time in ms between retires */
 #define KBC_MAX_RETRIES 5 /** Maximum number of retries for reading */

 ///KBC Status Register
 #define KBC_PARITY BIT(7) /** Parity error (if set): invalid data */
 #define KBC_TIMEOUT BIT(6) /** Timeout error (if set): invalid data */
 #define KBC_AUX BIT(5) /* Mouse data */
 #define KBC_INH BIT(4) /* Inhibit flag */
 #define KBC_A2 BIT(3) /** A2 Input Line */
 #define KBC_SYS BIT(2) /** System Flag */
 #define KBC_IBF BIT(1) /** Input buffer full */
 #define KBC_OBF BIT(0) /** Output buffer full - data available for reading */

 ///KB scancodes
 #define ESC_BREAKCODE 0x81

 ///KBC Commands:
 // The commands must be written to the address 0x64 ('Not named' and also status register)
 /* Arguments, if any, must be passed using address 0x60 (input buffer)
  * Return values, if any, are passed using the OUT_BUF (output buffer)
  */
 #define KBC_READCMDBYTE 0x20 /** Read command byte, returns command byte */
 #define KBC_WRITECMDBYTE 0x60 /** Write a command byte, takes in a command by_te; Write this to 0x64 then command byte to 0x60 */
 #define KBC_SELFTEST 0xAA /** Check KBC, returns 0x55 if OK, 0xFC if error */
    #define KBC_SELFTEST_OK 0x55
    #define KBC_SELFTEST_ERROR 0xFC
 #define KBC_CHECK_KBI /** Check keyboard interface, returns 0 if OK */
 #define KBC_DISABLE_KBD_I 0xAD /** Disables KBD interface */
 #define KBC_ENABLE_KBD_I 0xAE /** Enables KBD interface */

 //KBC "Command byte"
 #define KBC_CB_DIS2 BIT(5) /** disable mouse */
 #define KBC_CB_DIS BIT(4) /** disable keyboard interface */
 #define KBC_CB_INT2 BIT(1) /** Enable interrupt on OBF (output buffer full), from mouse*/
 #define KBC_CB_INT BIT(0) /** Enable interrupt on OBF from keyboard */

/**@}*/

#endif /* _LCOM_I8254_H */
