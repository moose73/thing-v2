#include <Arduino.h>
#pragma once


struct Task {
  uint32_t (*taskCall)(void);
  uint32_t nexttime;
  uint8_t uid;
  bool running;
};

#define MAX_TASKS 20


void init_scheduler(void);
void start_scheduler(void);

uint8_t add_task(uint32_t (*taskCall)(void), uint32_t start_delay);
void remove_task(uint8_t uid);
void pause_task(uint8_t uid);
void resume_task(uint8_t uid);
void run_task(uint8_t uid);
void clear_all_tasks();

