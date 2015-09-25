/* main.c --- 
 * 
 * Filename: main.c
 * Description: 
 * Author: Bryce Himebaugh
 * Maintainer: 
 * Created: Sun Sep 20 09:32:19 2015
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

/* Copyright (c) 2014-2015 Analog Computing Solutions  
 * 
 * All rights reserved. 
 * 
 * Additional copyrights may follow 
 */

/* Code: */
#include "ch.h"
#include "hal.h"
#include "test.h"
#include "shell.h" 
#include "chprintf.h"
#include <chstreams.h>
#include "gyro.h"
#include "caann.h"
#include "console.h"
#include "stm32f30x_flash.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define UNUSED(x) (void)(x)
static THD_WORKING_AREA(waShell,2048);

static thread_t *shelltp1;

/* Thread that blinks North LED as an "alive" indicator */
static THD_WORKING_AREA(waCounterThread,128);
static THD_FUNCTION(counterThread,arg) {
  UNUSED(arg);
  while (TRUE) {
    palSetPad(GPIOE, GPIOE_LED3_RED);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOE, GPIOE_LED3_RED);
    chThdSleepMilliseconds(500);
  }
}

static void cmd_myecho(BaseSequentialStream *chp, int argc, char *argv[]) {
  int32_t i;

  (void)argv;

  for (i=0;i<argc;i++) {
    chprintf(chp, "%s\n\r", argv[i]);
  }
}

static const ShellCommand commands[] = {
  {"myecho", cmd_myecho},
  {"gr",cmd_gyro_read},
  {"cr",cmd_gyro_read},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
};

static void termination_handler(eventid_t id) {

  (void)id;
  chprintf((BaseSequentialStream*)&SD2, "Shell Died\n\r");

  if (shelltp1 && chThdTerminatedX(shelltp1)) {
    chThdWait(shelltp1);
    chprintf((BaseSequentialStream*)&SD2, "Restarting from termination handler\n\r");
    chThdSleepMilliseconds(100);
    shelltp1 = shellCreate(&shell_cfg1, sizeof(waShell), NORMALPRIO);
  }
}

static evhandler_t fhandlers[] = {
  termination_handler
};


/* typedef struct daydata { */
/*   uint32_t epoch; */
/*   uint16_t hourly_count[24]; */
/* } daydata_t; */

#define MAX_DAYS 100
// static const uint32_t epoch_data[MAX_DAYS] __attribute__((section (".rodata"))) = { [0 ... MAX_DAYS-1] = 0xFFFFFFFF};
// static const uint16_t hourly_data[24*MAX_DAYS] __attribute__((section (".rodata"))) = { [0 ... (24*MAX_DAYS)-1] = 0xFFFF};

volatile uint32_t epoch_data[MAX_DAYS] __attribute__((section (".rodata"))) = { [0 ... MAX_DAYS-1] = 0xFFFFFFFF};
volatile uint16_t hourly_data[24*MAX_DAYS] __attribute__((section (".rodata"))) = { [0 ... (24*MAX_DAYS)-1] = 0xFFFF};


/* static const daydata_t data_array __attribute__((section (".rodata"))) = {.epoch=0xFFFFFFFF, .hourly_count[]={ [0 ... 23] = 0xFFFF}}; */

/*
 * Application entry point.
 */

int main(void) {
  event_listener_t tel;
  RTCDateTime time;
  struct tm * ltime;
  time_t epoch = 1443110745;
  uint32_t testaddress;
  int i;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  // Board Specific Initilizations
  gyro_init();
  caann_init();
  console_init();
  rtcGetTime(&RTCD1, &time);
  chprintf((BaseSequentialStream*)&SD2,"\n\r");
  chprintf((BaseSequentialStream*)&SD2,"date = %d\n\r",time.year);
  chprintf((BaseSequentialStream*)&SD2,"month = %d\n\r",time.month);
  chprintf((BaseSequentialStream*)&SD2,"dst flag = %d\n\r",time.dstflag);
  chprintf((BaseSequentialStream*)&SD2,"day of week = %d\n\r",time.dayofweek);
  chprintf((BaseSequentialStream*)&SD2,"day = %d\n\r",time.day);
  chprintf((BaseSequentialStream*)&SD2,"milliseconds = %d\n\r",time.millisecond);
  time.year = 2015 - 1980;
  rtcSetTime(&RTCD1, &time);
  rtcGetTime(&RTCD1, &time);
  chprintf((BaseSequentialStream*)&SD2,"\n\r");
  chprintf((BaseSequentialStream*)&SD2,"date = %d\n\r",time.year);
  chprintf((BaseSequentialStream*)&SD2,"month = %d\n\r",time.month);
  chprintf((BaseSequentialStream*)&SD2,"dst flag = %d\n\r",time.dstflag);
  chprintf((BaseSequentialStream*)&SD2,"day of week = %d\n\r",time.dayofweek);
  chprintf((BaseSequentialStream*)&SD2,"day = %d\n\r",time.day);
  chprintf((BaseSequentialStream*)&SD2,"milliseconds = %d\n\r",time.millisecond);

  ltime = localtime(&epoch);
  rtcConvertStructTmToDateTime(ltime, 0, &time);
  rtcSetTime(&RTCD1, &time);
  rtcGetTime(&RTCD1, &time);
  chprintf((BaseSequentialStream*)&SD2,"\n\r");
  chprintf((BaseSequentialStream*)&SD2,"date = %d\n\r",time.year);
  chprintf((BaseSequentialStream*)&SD2,"month = %d\n\r",time.month);
  chprintf((BaseSequentialStream*)&SD2,"dst flag = %d\n\r",time.dstflag);
  chprintf((BaseSequentialStream*)&SD2,"day of week = %d\n\r",time.dayofweek);
  chprintf((BaseSequentialStream*)&SD2,"day = %d\n\r",time.day);
  chprintf((BaseSequentialStream*)&SD2,"milliseconds = %d\n\r",time.millisecond);
  rtcConvertDateTimeToStructTm(&time,ltime, NULL);
  chprintf((BaseSequentialStream*)&SD2,"%s\n\r",asctime(ltime));
  chprintf((BaseSequentialStream*)&SD2,"data start = %x\n\r",epoch_data);
  chprintf((BaseSequentialStream*)&SD2,"epoch data[0] = %x\n\r",&epoch_data[0]);
  chprintf((BaseSequentialStream*)&SD2,"epoch data[MAX_DAYS-1] = %x\n\r",&epoch_data[MAX_DAYS-1]);
  chprintf((BaseSequentialStream*)&SD2,"delta = %d\n\r",(void *)(&epoch_data[MAX_DAYS-1])-(void *)(&epoch_data[0])+4);
  chprintf((BaseSequentialStream*)&SD2,"hourly_data[0] = %x\n\r",hourly_data[0]);
  chprintf((BaseSequentialStream*)&SD2,"epoch data[MAX_DAYS-1] = %x\n\r",hourly_data[(24*MAX_DAYS)-1]);
  chprintf((BaseSequentialStream*)&SD2,"delta = %d\n\r",(void *)(&hourly_data[(24*MAX_DAYS)-1])-(void *)(&hourly_data[0])+2);

  /* chprintf((BaseSequentialStream*)&SD2,"epoch data[0] before = %x\n\r",epoch_data[0]); */
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  testaddress = (uint32_t)(&epoch_data[0]);
  chprintf((BaseSequentialStream*)&SD2,"address = %x, write_status = %d\n\r",testaddress, FLASH_ProgramWord(testaddress,0xA1A2A3A5));
  chprintf((BaseSequentialStream*)&SD2,"address = %x, write_status = %d\n\r",testaddress+4, FLASH_ProgramWord(testaddress+4,0x11111111));

  FLASH_Lock();
  //  chprintf((BaseSequentialStream*)&SD2,"epoch data[0] after = %x\n\r",epoch_data[0]);
  for (i=0;i<10;i++) {
    chprintf((BaseSequentialStream*)&SD2,"%d %x\n\r",i, epoch_data[i]);
  }
  for (i=0;i<2;i++) {
    chprintf((BaseSequentialStream*)&SD2,"%d %x\n\r",i, (volatile int) epoch_data[i]);
  }

  //  chprintf((BaseSequentialStream*)&SD2, "\n\rUp and Running\n\r");
  // chprintf((BaseSequentialStream*)&SD2, "Gyro Whoami Byte = 0x%02x\n\r",gyro_read_register(0x0F));

  /* Initialize the command shell */ 
  shellInit();
  /* 
   *  setup to listen for the shell_terminated event. This setup will be stored in the tel  * event listner structure in item 0
  */
  chEvtRegister(&shell_terminated, &tel, 0);

  shelltp1 = shellCreate(&shell_cfg1, sizeof(waShell), NORMALPRIO);
  chThdCreateStatic(waCounterThread, sizeof(waCounterThread), NORMALPRIO+1, counterThread, NULL);

  while (TRUE) {
    chEvtDispatch(fhandlers, chEvtWaitOne(ALL_EVENTS));
  }
 }


/* main.c ends here */
