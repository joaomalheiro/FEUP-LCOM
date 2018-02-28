#ifndef _FONT_H
#define _FONT_H

/** @defgroup font font
 * @{
 *
 * Module that handles variable text output to the screen in graphical mode
 */

////Currently available fonts are
//codebold-36 - CODE Bold, 36px, has / and : - lowercase only - really uppercase
//kenneypixel-38 - Kenney Pixel, 38px, has /, : and .
//monofonto-18 - Monofonto, 18px, only has number and /, : and . (no lower or uppercase letters)
//monofonto-22 - Monofonto, 22px, has /, : and . lowercase only - really uppercase

/**
 * @brief Draws given string in given font at given coordinates
 * @param string String to display in screen
 * @param font   The font to display the string in
 * @param x      The x at which to draw the font
 * @param y      The y at which to draw the font
 */
void string_to_screen (char * string, char * font, int x , int y);

#endif /* __FONT_H */
