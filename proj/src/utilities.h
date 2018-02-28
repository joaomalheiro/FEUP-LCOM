#ifndef _UTILITIES_H_
#define _UTILITIES_H_

/** @defgroup utilities utilities
 * @{
 *
 * Utilities useful for use throughout the program
 */

#define BIT(n) (0x01<<(n)) /**< @brief Returns the bit number n active using left bitshifts */
#define SWAP(T, n, p) do{T SWAP_TEMP_VAR = n; n = p; p = SWAP_TEMP_VAR;}while (0) /**< @brief Swaps two variables of the passed type*/
#define MIN_VAL(n, p) ((n) < (p) ? (n) : (p)) /**< @brief Returns the minimum of the passed in values */
#define MAX_VAL(n, p) ((n) > (p) ? (n) : (p)) /**< @brief Returns the maximum of the passed in values */
#define ROUND(n) ((n) > 0.0 ? (int)((n)+0.5) : (int)((n)-0.5)) /**< @brief Rounds the passed in value (ends with 0.5 or more, round up, otherwise round down) */
#define GET_MSBYTE(n) ((unsigned char) (((n) >> 8) & 0xff)) /**< @brief Gets the Most Significant Byte of the passed in value */
#define GET_LSBYTE(n) ((unsigned char) ((n) & 0xff)) /**< @brief Gets the Least Significant Byte of the passed in value */
#define JOIN_LSB_MSB(lsb, msb) ((unsigned short) ((unsigned char)(msb) << 8) | ((unsigned char)(lsb) & 0xff)) /**< @brief Joins the LSB and the MSB together */

/**
 * @brief Parses the passed string and returns an unsigned long long interpreted in the passed base
 * @param  str  String to interpret as unsigned long long
 * @param  base Base in which to perform the conversion
 * @return      Retuns the converted value or ULLONG_MAX in case of error
 */
unsigned long long parse_ullong(char *str, int base);

/**@}*/

#endif /* _UTILITIES_H */
