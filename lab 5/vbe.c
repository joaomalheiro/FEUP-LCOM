#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>

#include "vbe.h"
#include "lmlib.h"

#include "video_test.h"

#define LINEAR_MODEL_BIT 14

#define PB2BASE(x) (((x) >> 4) & 0x0F000)
#define PB2OFF(x) ((x) & 0x0FFFF)

//Useful macros for converting from real mode far pointers to linear addresses
#define RM_SEGMENT(x) (((x) >> 16) & 0x0FFFF)
#define RM_OFFSET(x) ((x) & 0x0FFFF)

//VBE commands
#define VBE_GET_MODE_INFO 0x4F01
#define VBE_GET_CTRL_INFO 0x4F00

//Other
#define VIDEO_MODE_MODE_LIST_END 0x0FFFF
#define MAJ_VERSION_BITMASK 0x0FF00
#define MIN_VERSION_BITMASK 0x00FF

/* VBE call macros */
#define VBE_CALL_SUPPORTED 0x4F
#define VBE_CALL_SUCCESSFUL 0x00
#define VBE_CALL_FAILED 0x01
#define VBE_CALL_NOT_SUPPORTED_CURRENT_HW 0x02
#define VBE_INVALID_IN_VIDEO_MODE 0x03

int vbe_get_mode_info(unsigned short mode, vbe_mode_info_t *vmi_p) {

  mmap_t buf;
  struct reg86u r;

  lm_init();

  lm_alloc(sizeof(vbe_mode_info_t), &buf);

  r.u.w.ax = VBE_GET_MODE_INFO; /* VBE get mode info */
  /* translate the buffer linear address to a far pointer */
  r.u.w.es = PB2BASE(buf.phys); /* set a segment base */
  r.u.w.di = PB2OFF(buf.phys); /* set the offset accordingly */
  r.u.w.cx = mode;
  r.u.b.intno = 0x10;

  if(sys_int86(&r) != OK) {
    printf("vbe_get_mode_info::Error in sys_int86() \n");
    lm_free(&buf);
    return 1;
  }

  if(r.u.b.al != VBE_CALL_SUPPORTED){
    printf("vbe_get_mode_info::VBE call not supported!\n");
    return 2;
  }

  if(r.u.b.ah != VBE_CALL_SUCCESSFUL){
    if(r.u.b.ah == VBE_CALL_FAILED){
      printf("vbe_get_mode_info::VBE call failed\n");
      return 3;
    } else if(r.u.b.ah == VBE_CALL_NOT_SUPPORTED_CURRENT_HW){
      printf("vbe_get_mode_info::VBE call not supported in current HW configuration\n");
      return 4;
    } else if(r.u.b.ah == VBE_INVALID_IN_VIDEO_MODE){
      printf("vbe_get_mode_info::VBE call invalid in current video mode\n");
      return 5;
    }
  }

  *vmi_p = *(vbe_mode_info_t*) buf.virtual;

  lm_free(&buf);

  return 0;
}

int vbe_get_ctrl_info() {
  mmap_t buf;
  struct reg86u r;

  //Getting the virtual memory base address, will be useful for interpreting the read result later on
  void* memory_base_ptr = lm_init();

  lm_alloc(sizeof(vbe_ctrl_info_t), &buf);

  r.u.w.ax = VBE_GET_CTRL_INFO; /* VBE get mode info */
  /* translate the buffer linear address to a far pointer */
  r.u.w.es = PB2BASE(buf.phys); /* set a segment base */
  r.u.w.di = PB2OFF(buf.phys); /* set the offset accordingly */
  r.u.b.intno = 0x10;

  if(sys_int86(&r) != OK) {
    printf("vbe_get_ctrl_info::Error in sys_int86() \n");
    lm_free(&buf);
    return 1;
  }

  if(r.u.b.al != VBE_CALL_SUPPORTED){
    printf("vbe_get_ctrl_info::VBE call not supported!\n");
    return -1;
  }

  if(r.u.b.ah != VBE_CALL_SUCCESSFUL){
    if(r.u.b.ah == VBE_CALL_FAILED){
      printf("vbe_get_ctrl_info::VBE call failed\n");
      return -2;
    } else if(r.u.b.ah == VBE_CALL_NOT_SUPPORTED_CURRENT_HW){
      printf("vbe_get_ctrl_info::VBE call not supported in current HW configuration\n");
      return -3;
    } else if(r.u.b.ah == VBE_INVALID_IN_VIDEO_MODE){
      printf("vbe_get_ctrl_info::VBE call invalid in current video mode\n");
      return -4;
    }
  }

  //Getting the read information from the buffer to our new struct, to be able to correctly interpret the data
  vbe_ctrl_info_t vbe_ctrl_info_p = *(vbe_ctrl_info_t *) buf.virtual;

  lm_free(&buf);
  /* Data has now been read and is now stored in vbe_ctrl_info_p */

  //Printing the results:
  //First, parsing the data retrieved

  //Getting VBE Version
  //VBE version is stored in an unsigned short (16 bits), being that there are two numbers in BCD (Minor version and Major version numbers)
  //To retrieve them, we must retrieve both the lowest 8 bits and the highest 8 bits separately
  //As such we can store each version number in a separate unsigned char variable
  //The 8 MSb represent the major version
  unsigned char min_version_nr = (vbe_ctrl_info_p.VBEVersion & MIN_VERSION_BITMASK);
  //The 8 LSb represent the minor version
  unsigned char maj_version_nr = (vbe_ctrl_info_p.VBEVersion & MAJ_VERSION_BITMASK) >> 8;

  //Getting the access to the supported modes list
  //VideoModePtr is a Real Mode far pointer, and it must be converted to a linear address
  //The 16 MSb of VideoModePtr are the segment and the 16 LSb are the offset
  //The address to read from is given by:
  //VRAM_base_addr + (segment << 4) + offset
  //Obtaining the segment using a defined macro (16 bits, u_short)
  unsigned short segment = RM_SEGMENT(vbe_ctrl_info_p.VideoModePtr);
  //Obtaining the offset using a defined macro (16 bits, u_short)
  unsigned short offset = RM_OFFSET(vbe_ctrl_info_p.VideoModePtr);
  //Getting the actual VideoModePtr in a linear address that we can access (each mode occupies one word, 16 bits, therefore unsigned short)
  unsigned short * video_modes_list_ptr = (unsigned short *) (memory_base_ptr + (segment << 4) + offset);
  //The address to read from has now been retrieved

  //Getting the size of VRAM memory in KB
  //Since TotalMemory is the number of 64KB blocks of memory, multiplying it by 64 results in the VRAM size in KB
  unsigned int VRAM_mem_size_kb = vbe_ctrl_info_p.TotalMemory * 64;


  //Secondly, priting the obtained results
  //VBE Version
  printf("%x.%x\n", maj_version_nr, min_version_nr);

  //Supported modes in hexadecimal
  while(*video_modes_list_ptr != VIDEO_MODE_MODE_LIST_END){
    printf("0x%X:", *video_modes_list_ptr);
    video_modes_list_ptr++;
  }

  //Size of VRAM memory in KB
  printf("\n%u\n", VRAM_mem_size_kb);

  //Everything went as expected
  return 0;
}
