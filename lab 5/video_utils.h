#ifndef _LCOM_VIDEO_UTILS_H_
#define _LCOM_VIDEO_UTILS_H_

/** @defgroup video_utils video_utils
 * @{
 *
 * Constants that are helpful for programming the video graphics controller
 */

 #define BIT(n) (0x01<<(n)) /**< @brief Returns the bit number n active using left bitshifts */
 #define VBE_SET_MODE  0x4F02 /** VBE call to function 02 - set VBE mode*/
 #define VIDEO_INT     0x10   /** Video card interrupt*/

 #define SWAP(T, n, p) do{T SWAP_TEMP_VAR = n; n = p; p = SWAP_TEMP_VAR;}while (0)
 #define MIN_VAL(n, p) ((n) < (p) ? (n) : (p))
 #define MAX_VAL(n, p) ((n) > (p) ? (n) : (p))
 #define ROUND(n) ((n) > 0.0 ? (int)((n)+0.5) : (int)((n)-0.5))

/**@}*/

#endif /* _LCOM_VIDEO_UTILS_H */
