#ifndef TM_FRAME_H
#define TM_FRAME_H
#include "arduino.h"
#include "print.h"

class TmFrame
{
  public:
    byte freq[8];    // 0x00
    byte disp[6];    // 0x02
    byte smeter;     // 0x05
    byte alt;        // 0x06
    byte mr[2];      // 0x07
    byte fstat;      // 0x08
    
    byte fid;        // frame id
    byte pfid;       // previous frame id
    byte fbyte;      // receive byte of a frame

    TmFrame();
    byte parse(byte v);
    
    char * freq_s();
    char * freq_ss();  // short format
    char * smeter_s();
    char * alt_s();
    char * mr_s();
    char * power_s();
    char CMV_s();

    void print_fix16_col0(Print &p);
    void print_fix16_col1(Print &p);
    
    void print_freq_frame(Print &p);
    void print_disp_frame(Print &p);

    boolean is_REV(){ return (disp[0] & 0x4); };
    boolean is_BUSY(){ return (disp[0] & 0x8); };  // BUSY on display frame
    boolean is_CALL(){ return (disp[1] & 0x1); };
    boolean is_AL(){ return (disp[1] & 0x2); };
    boolean is_STEP_MODE(){ return (disp[2] & 0x1); };
    boolean is_TONE_MODE(){ return (disp[2] & 0x2); };  
    boolean is_BEEP_OFF(){ return (disp[2] & 0x4); };  
    boolean is_MR(){ return (disp[2] & 0x8); };
    boolean is_SCAN_MODE(){ return (disp[3] & 0x4); };
    boolean is_STAR(){ return (disp[3] & 0x8); };
    boolean is_BELL(){ return (disp[4] & 0x1); };
    
    boolean is_SBUSY(){ return (smeter & 0x10); };  // BUSY on smeter frame
    boolean isnt_MR(){ return (mr[0] & 0x10); };
  private:
    char str[16];
};

#endif
