#ifndef TM_SPI_H
#define TM_SPI_H
#include "arduino.h"

#define TMSPI_P_READ   2
#define TMSPI_P_WRITE  4
#define TMSPI_P_CLK    3
#define TMSPI_INT_CLK  1
#define TMSPI_P_LED    13

#define TMSPI_RWAIT    0
#define TMSPI_RPREP    1
#define TMSPI_RRCV     2
#define TMSPI_SPREP    3
#define TMSPI_SENDING  4
#define TMSPI_SENT     5

class TmSpi
{
    TmSpi();
    static void int_rcv_clk();
    static void int_snd_clk();
    volatile void (*cb_func)(byte);
    // for timeout
    volatile unsigned long time;
    // rig communication data
    volatile byte bit_stat;
    volatile byte bit_data;
    volatile byte bit_cnt;
    byte retry_cnt;
  public:
    static TmSpi & get_instance(){
      static TmSpi obj;
      return obj;
    }
    
    void setup(volatile void (*func)(byte));
    char loop();
    char send_cmd(byte c);
    boolean is_waiting(){ return bit_stat == TMSPI_RWAIT; }
};

#endif
