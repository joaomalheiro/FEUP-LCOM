///bitmap.h and bitmap.c were taken from http://difusal.blogspot.pt/2014/09/minixtutorial-8-loading-bmp-images.html with permission from the teacher, with some alterations
//Modifications were made to drawBitmap (and a slight one to loadBitmap, that could have caused memory leaks, and switching over to unsigned shorts - since 16 bit color mode is being used);
//drawBitmapWithoutTransparency, drawFullscreenBitmap, drawBitmapWithRotation, get_rot_x, get_rot_y, copyBitmap and collision functions were all implemented by ourselves
#include "bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcpy */
#include <math.h>
#include "video_gr.h"

//Since PI was not found in math.h's defines we define it here (at the highest precision possible with native C types)
#define PI 3.14159265358979323846

//Color to ignore when drawing BMPs, to be able to use transparency
//Values obtained from testing with color #ff00ff converted to BMP through GIMP, in 5R 6G 5B format
#define IGNORE_COLOR 0xf81f
//Color a pixel has when it is empty
#define EMPTY_PIXEL 0x0000

///Helper private functions

//Get the delta along the x axis to shift pixels by
static double get_rot_x(double angle, double x, double y) {
  // - PI/2 is a correction factor we are using since we want up to be angle 0, and not 90º
  double cossine = cos(angle - PI/2);
  double sine = sin(angle - PI/2);
  return x * cossine + y * -sine;
}

//Gets the delta along the y axis to shift pixels by
static double get_rot_y(double angle, double x, double y) {
  // - PI/2 is a correction factor we are using since we want up to be angle 0, and not 90º
  double cossine = cos(angle - PI/2);
  double sine = sin(angle - PI/2);
  return x * sine + y * cossine;
}

//Performs pixel calculations and returns a rotated Bitmap, based on the passed one and the passed angle
static Bitmap * rotateBitmap(Bitmap * bmp, double angle) {
  //TODO: It is not necessary to fully copy the bitmap, only allocate space, since we are copying in the nested for loop. Change that maybe?
  //We are using memcpy to copy so it should not be a problem for it is very efficient...

  //Change that in the future, after it is working
  //Copying the passed bitmap into a new one, since we will be changing pixels
  Bitmap * result = copyBitmap(bmp);

  //If there was a problem in copying, return NULL
  if(result == NULL){
    return NULL;
  }

  int bmp_width = result->bitmapInfoHeader.width;
  int bmp_height = result->bitmapInfoHeader.height;

  //Using doubles in a lot of places onwards to ensure for precision when applying mathematical operations (sin and cos, for example)

  //Getting the shift in the x and y for each pixel's x and y
  //Negative angle is passed since we are considering clockwise roations positive
  //(human measures > mathematical convention measures)
  double newx_x = get_rot_x(-angle, 1.0, 0.0); //How x is going to shift in the x axis
  double newx_y = get_rot_y(-angle, 1.0, 0.0); //How x is going to shift in the y axis
  double newy_x = get_rot_x(-angle, 0.0, 1.0); //How y is going to shift in the x axis
  double newy_y = get_rot_y(-angle, 0.0, 1.0); //How y is going to shift in the y axis

  //Getting the x and y at which to start
  double xi = get_rot_x(-angle, -bmp_width/2.0, -bmp_height/2.0) + bmp_width / 2.0;
  double yi = get_rot_y(-angle, -bmp_width/2.0, -bmp_height/2.0) + bmp_height / 2.0;

  //Everything until here was just simple geometry, the reason for having to pass in a bitmap is to know the colors of the contents of the bitmap
  //Because the same transformation is applied to everything

  //Actually changing the pixels now
  int x, y;

  for(y = 0; y < bmp_height; y++){
    double xf = xi;
    double yf = yi;
    for(x = 0; x < bmp_width; x++) {
      //Converting the doubles into integers because we will use indexes
      int xx = (int) xf;
      int yy = (int) yf;

      //Detecting if the access to the original sprite would be out of bounds
      if(xx < 0 || xx >= bmp_width || yy < 0 || yy >= bmp_height){
        //This fills the pixels with the "transparent color"
        result->bitmapData[x + y*bmp_width] = IGNORE_COLOR;
      } else {
        result->bitmapData[x + y*bmp_width] = bmp->bitmapData[xx + yy*bmp_width];
      }
      //This also fixes the "void pixels" according to my testing

      //Incrementing xf based on the calculated direction
      xf += newx_x;
      yf += newx_y;
    }
    //Incrementing yf based on the calculated direction
    xi += newy_x;
    yi += newy_y;
  }

  return result;
}

//Determines if we are able to draw the bitmap bmp in the coords x and y in the buffer passed
//Useful for checking for collisions with sprites, because we can draw to a temporary buffer and then
//use this to know if the bitmaps collided
//Returns true if there was a "collision"
static bool check_if_already_drawn(Bitmap * bmp, int x, int y, unsigned short * buffer) {
  if (bmp == NULL)
      return false;

  int width = bmp->bitmapInfoHeader.width;
  int drawWidth = width;
  int height = bmp->bitmapInfoHeader.height;

  if (x + width < 0 || x > getHorResolution() || y + height < 0 || y > getVerResolution())
      return false;

  int xCorrection = 0;

  if (x < 0) {
      xCorrection = -x;
      drawWidth -= xCorrection;
      x = 0;

      if (drawWidth > getHorResolution())
          drawWidth = getHorResolution();
  } else if (x + drawWidth >= getHorResolution()) {
      drawWidth = getHorResolution() - x;
  }

  unsigned short* bufferStartPos;
  unsigned short* imgStartPos;

  int i;
  for (i = 0; i < height; i++) {
    int pos = y + height - 1 - i;

    if (pos < 0 || pos >= getVerResolution())
      continue;

    bufferStartPos = buffer;
    bufferStartPos += x  + pos * getHorResolution();

    imgStartPos = bmp->bitmapData + xCorrection + i * width;

    int j;
    for(j = 0; j < drawWidth; j++){
      //If the position of the buffer is not transparent nor empty,
      //and the image position we would draw is not transparent, then we are trying to draw over something already drawn
      if(bufferStartPos[j] != IGNORE_COLOR && bufferStartPos[j] != EMPTY_PIXEL && imgStartPos[j] != IGNORE_COLOR) {
        return true;
      }
    }
  }

  //If no "collision" ocurred so far, then none ocurred
  return false;
}

//Draws the passed bitmap in the passed positions in the passed buffer, not considering transparency (IGNORE_COLOR)
static void drawBitmapWithoutTransparency_aux(Bitmap* bmp, int x, int y, unsigned short * buffer) {
  if (bmp == NULL)
      return;

  int width = bmp->bitmapInfoHeader.width;
  int drawWidth = width;
  int height = bmp->bitmapInfoHeader.height;

  if (x + width < 0 || x > getHorResolution() || y + height < 0 || y > getVerResolution())
      return;

  int xCorrection = 0;

  if (x < 0) {
      xCorrection = -x;
      drawWidth -= xCorrection;
      x = 0;

      if (drawWidth > getHorResolution())
          drawWidth = getHorResolution();
  } else if (x + drawWidth >= getHorResolution()) {
      drawWidth = getHorResolution() - x;
  }

  unsigned short* bufferStartPos;
  unsigned short* imgStartPos;

  int i;
  for (i = 0; i < height; i++) {
    int pos = y + height - 1 - i;

    if (pos < 0 || pos >= getVerResolution())
        continue;

    bufferStartPos = buffer;
    bufferStartPos += x  + pos * getHorResolution();

    imgStartPos = bmp->bitmapData + xCorrection + i * width;

    memcpy(bufferStartPos, imgStartPos, drawWidth * 2);
  }

}

//Draws the passed bitmap in the passed positions in the passed buffer, considering transparency (IGNORE_COLOR)
static void drawBitmap_aux(Bitmap* bmp, int x, int y, unsigned short * buffer) {
  if (bmp == NULL)
      return;

  int width = bmp->bitmapInfoHeader.width;
  int drawWidth = width;
  int height = bmp->bitmapInfoHeader.height;

  if (x + width < 0 || x > getHorResolution() || y + height < 0 || y > getVerResolution())
      return;

  int xCorrection = 0;

  if (x < 0) {
      xCorrection = -x;
      drawWidth -= xCorrection;
      x = 0;

      if (drawWidth > getHorResolution())
          drawWidth = getHorResolution();
  } else if (x + drawWidth >= getHorResolution()) {
      drawWidth = getHorResolution() - x;
  }

  unsigned short* bufferStartPos;
  unsigned short* imgStartPos;

  int i;
  for (i = 0; i < height; i++) {
    int pos = y + height - 1 - i;

    if (pos < 0 || pos >= getVerResolution())
      continue;

    bufferStartPos = buffer;
    bufferStartPos += x  + pos * getHorResolution();

    imgStartPos = bmp->bitmapData + xCorrection + i * width;

    //memcpy(bufferStartPos, imgStartPos, drawWidth * 2);

    int j;
    for(j = 0; j < drawWidth; j++){
      if(imgStartPos[j] != IGNORE_COLOR) {
        //printf("colors at i=%d j=%d\tvalue:%x\n", i, j, imgStartPos[j]);
        bufferStartPos[j] = imgStartPos[j];
      }
    }
  }

}

////Public functions

Bitmap* loadBitmap(const char* filename) {
    // allocating necessary size
    Bitmap* bmp = (Bitmap*) malloc(sizeof(Bitmap));

    // If memory could not be allocated
    if(bmp == NULL) {
      return NULL;
    }

    // open filename in read binary mode
    FILE *filePtr;
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL){
      free(bmp);
      return NULL;
    }

    // read the bitmap file header
    BitmapFileHeader bitmapFileHeader;
    fread(&bitmapFileHeader, 2, 1, filePtr);

    // verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader.type != 0x4D42) {
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    int rd;
    do {
        if ((rd = fread(&bitmapFileHeader.size, 4, 1, filePtr)) != 1)
            break;
        if ((rd = fread(&bitmapFileHeader.reserved, 4, 1, filePtr)) != 1)
            break;
        if ((rd = fread(&bitmapFileHeader.offset, 4, 1, filePtr)) != 1)
            break;
    } while (0);

    if (rd != 1) {
        fprintf(stderr, "Error reading file\n");
        exit(-1);
    }

    // read the bitmap info header
    BitmapInfoHeader bitmapInfoHeader;
    fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, filePtr);

    // move file pointer to the begining of bitmap data
    fseek(filePtr, bitmapFileHeader.offset, SEEK_SET);

    // allocate enough memory for the bitmap image data
    unsigned short* bitmapImage = malloc(bitmapInfoHeader.imageSize);

    // verify memory allocation
    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // read in the bitmap image data
    fread(bitmapImage, bitmapInfoHeader.imageSize, 1, filePtr);

    // make sure bitmap image data was read
    if (bitmapImage == NULL) {
        free(bitmapImage);
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // close file and return bitmap image data
    fclose(filePtr);

    bmp->bitmapData = bitmapImage;
    bmp->bitmapInfoHeader = bitmapInfoHeader;

    return bmp;
}

void drawBitmap(Bitmap* bmp, int x, int y) {
  drawBitmap_aux(bmp, x, y, getBackBuffer());
}

void drawBitmapWithoutTransparency(Bitmap* bmp, int x, int y) {
  drawBitmapWithoutTransparency_aux(bmp, x, y, getBackBuffer());
}

void drawBitmapWithRotation(Bitmap* oldbmp, int x, int y, double angle) {
  if (oldbmp == NULL)
      return;

  //Getting the new, rotated bitmap
  Bitmap * bmp = rotateBitmap(oldbmp, angle);

  //If it could not be correctly calculated or allocated
  if(bmp == NULL){
    return;
  }

  //Now just drawing the transformed bitmap normally
  drawBitmap(bmp, x, y);

  //Before returning, destroying the newly created sprite for this rotation to avoid memory leaks
  deleteBitmap(bmp);
}

void drawFullscreenBitmap(Bitmap * bmp) {
  if(bmp == NULL) {
    printf("DBG: fullscreen bmp was null\n");
    return;
  }
  //When a bitmap is fullscreen we don't care about transparency (normally backgrounds)
  //This can't be made more efficient since bitmaps store the row order inverted, so we can only at best memcpy row by row
  drawBitmapWithoutTransparency_aux(bmp, 0, 0, getBackBuffer());
}

bool check_if_bitmaps_collided(Bitmap * bmp1, int b1x, int b1y, Bitmap * bmp2, int b2x, int b2y) {
  //Just in case one of the bitmaps is null
  if(bmp1 == NULL || bmp2 == NULL) {
    return false;
  }

  //First checking for rectangle intersections since the pixel perfect technique is a bit expensive
  //These checks are made using the AABB (Axis-Aligned Bounding Box) algorithm for collision detection
  //If one rectangle is not intersecting another, return false and leave straight away
  if ( !(b1x < b2x + bmp2->bitmapInfoHeader.width && b1x + bmp1->bitmapInfoHeader.width > b2x && b1y < b2y + bmp2->bitmapInfoHeader.height && bmp1->bitmapInfoHeader.height + b1y > b2y)) {
    //One rectangle is not intersecting the other, thus no collision can ocurr
    return false;
  }

  //Using calloc to zero-initialize the temporary buffer
  //(calloc receves nelements, size_of_element so we just tell it that the vram size is in bytes)
  unsigned short * tempbuffer = calloc(getVramSize(), 1);

  //Verifying if the temporary buffer could be correctly allocated
  if(tempbuffer == NULL) {
    return false;
  }

  bool result;

  //Since it is more efficient to draw than to iterate, we want to draw the largest bitmap (because we are using memcpy to draw)
  if(bmp1->bitmapInfoHeader.size > bmp2->bitmapInfoHeader.size) {
    //Drawing bmp1, checking against bmp2
    drawBitmapWithoutTransparency_aux(bmp1, b1x, b1y, tempbuffer);
    result = check_if_already_drawn(bmp2, b2x, b2y, tempbuffer);
  } else {
    //Drawing bmp2, checking against bmp1
    drawBitmapWithoutTransparency_aux(bmp2, b2x, b2y, tempbuffer);
    result = check_if_already_drawn(bmp1, b1x, b1y, tempbuffer);
  }

  //Never forget to free allocated memory...
  free(tempbuffer);
  //That is the reason the result is stored in a temporary variable, we could not return from the function result directly otherwise we could not free the temp buffer
  return result;
}

bool check_if_bitmaps_collided_rotated_w_non_rotated(Bitmap * bmp1, int b1x, int b1y, double angleb1, Bitmap * bmp2, int b2x, int b2y) {
  //Just in case one of the bitmaps is null
  if(bmp1 == NULL || bmp2 == NULL) {
    return false;
  }

  //Getting the new, rotated bitmap
  Bitmap * newbmp1 = rotateBitmap(bmp1, angleb1);

  //If it could not be correctly calculated or allocated
  if(newbmp1 == NULL){
    return false;
  }

  //Now just checking for collisions with the transformed bitmap normally
  //(Storing result in a temp variable because we have to pass as arguments before deleting but also have to delete "after returning")
  bool result = check_if_bitmaps_collided(newbmp1, b1x, b1y, bmp2, b2x, b2y);

  //Before returning, destroying the newly created sprite for this rotation to avoid memory leaks
  deleteBitmap(newbmp1);
  return result;
}

Bitmap * copyBitmap(Bitmap * bmp){
  //If passed bitmap is null, do nothing
  if(bmp == NULL) {
    return NULL;
  }

  //Allocating memory for the new bmp
  Bitmap * newbmp = malloc(sizeof *bmp);
  if(newbmp == NULL) {
    //Couldn't allocate memory
    return NULL;
  }


  //Allocating space for the new bitmap data
  newbmp->bitmapData = malloc(bmp->bitmapInfoHeader.imageSize);

  //if space couldn't be allocated free allocated memory and return NULL
  if(newbmp->bitmapData == NULL){
    free(newbmp);
    return NULL;
  }

  //Otherwise, copy bitmap data using memcpy (to allow changing bitmaps independently)
  memcpy(newbmp->bitmapData, bmp->bitmapData, bmp->bitmapInfoHeader.imageSize);

  //Finally, copying info header
  newbmp->bitmapInfoHeader = bmp->bitmapInfoHeader;

  //Returning copied bitmap
  return newbmp;
}

void deleteBitmap(Bitmap* bmp) {
    if (bmp == NULL)
        return;

    free(bmp->bitmapData);
    free(bmp);
}
