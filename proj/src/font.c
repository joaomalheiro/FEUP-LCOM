#include "font.h"
//#define _GNU_SOURCE /* for using asprintf */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* for tolower */
#include "bitmap.h"

void string_to_screen (char * text, char * font , int x , int y) {
  if(text == NULL || strlen(text) == 0 || font == NULL || strlen(font) == 0) {
    return;
  }

  //Failsafe for strcat'ing question mark address (to avoid writing off the limit of the char array)
  //(46 is the size of the base address in which the font name is inserted)
  if(strlen(font) + 46 > 70) {
    printf("Debug: Font exceeds question mark address character limit\n");
    return;
  }

  //Fallback question mark character
  //The path has 46 of base length (without the argument received part), so we create a char array of 46 + 14 (the largest font name) + 10 for safety = 70
  //Initializing with base address
  char question_mark_address[70] = "/home/Robinix/res/img/fonts/";
  //Adding passed font
  strcat(question_mark_address, font);
  //File termination
  strcat(question_mark_address, "/question_mark.bmp");

  //Attempting to load failsafe bmp
  Bitmap* question_mark_bmp = loadBitmap(question_mark_address);
  if(question_mark_bmp == NULL) {
    //Load not possible, it is considered that the font does not exist because we always need a failsafe character to print
    //No need to free question_mark_address string since it is stack allocated
    return;
  }

  //No need to free question_mark_address string since it is stack allocated

  int s_size = strlen(text);
  Bitmap* bmp;
  char address[70];
  int i;

  //Some fonts are lowercase only, list them here to convert all the text to lowercase
  if(strncmp(font, "codebold-36", strlen("codebold-36")) == 0 || strncmp(font, "monofonto-22", strlen("monofonto-22")) == 0) {
    for(i = 0; i < s_size; i++) {
      text[i] = tolower((unsigned char)text[i]);
    }
  }

  for (i = 0; i < s_size; i++) {
    //If the text is just a space, then just increase the x variable and be done with it
    if(text[i] == ' ') {
      x += question_mark_bmp->bitmapInfoHeader.width;
      continue;
    }

    //Base address
    strcpy(address, "/home/Robinix/res/img/fonts/");

    //Concatenating passed font
    strcat(address, font);
    strcat(address, "/");

    //Because we need to consider special symbols (they have no representation in file names)
    if(text[i] == '/') {
      strcat(address, "slash");
    } else if(text[i] == ':') {
      strcat(address, "colon");
    } else if(text[i] == '.'){
      strcat(address, "dot");
    } else if(text[i] == '?') {
      //The case where no matching bmp is found is the one where ? is drawn so there is no need to allocate a special bitmap just for that
      //Just pass in a random filename
      strcat(address, "random_string");
    } else {
      //If letter is uppercase
      if(text[i] >= 'A' && text[i] <= 'Z') {
        text[i] = tolower((unsigned char)text[i]);
        //Because strcat must receive a null terimanted string, we generate one temporarily in order to concatenate the symbol
        strcat(address, (char[2]) { text[i], '\0' });
        strcat(address, "_caps");
      } else {
        //If letter is lowercase
        //Because strcat must receive a null terimanted string, we generate one temporarily in order to concatenate the symbol
        strcat(address, (char[2]) { text[i], '\0' });
      }
    }
    //Adding file termination
    strcat(address, ".bmp");

    //Attempting to load the bitmap
    bmp = loadBitmap(address);
    //If the bmp is NULL then the path was not found
    if (bmp == NULL) {
      //The fallback sprite is the question mark
      drawBitmap(question_mark_bmp, x, y);
      x += question_mark_bmp->bitmapInfoHeader.width;
    } else {
      drawBitmap(bmp,x,y);
      x += bmp->bitmapInfoHeader.width;
      //Only if the bitmap was allocated should it need to be freed
      deleteBitmap(bmp);
      bmp = NULL;
    }
  }

  deleteBitmap(question_mark_bmp);
  question_mark_bmp = NULL;
}
