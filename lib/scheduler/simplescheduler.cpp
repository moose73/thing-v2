#include "simplescheduler.h"



Task task_table[MAX_TASKS];

void init_scheduler(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    task_table[i].taskCall = NULL;
    task_table[i].nexttime = 0;
    task_table[i].uid = 0;
    task_table[i].running = false;
  }
}

void start_scheduler(void) {
  while (true) {
  for (uint32_t i = 0; i < MAX_TASKS; 
       i++) { 
      uint32_t ticks = micros(); 
      if (task_table[i].running && task_table[i].nexttime - ticks > UINT32_MAX / 2) {
        // Serial.printf("running task %d\n", i);
        int retval = task_table[i].taskCall();
        if (retval > 0) {
          task_table[i].nexttime = ticks + retval;
        } else {
           remove_task(i);
        }
      }
       }
  }
}

uint8_t add_task(uint32_t (*taskCall)(void), uint32_t start_delay) {
  //check the task isn't already in the list
  for (int i = 0; i < MAX_TASKS; i++) {
    if (task_table[i].taskCall == taskCall) {
      Serial.printf("Error: Task already exists\n");
      return 0;
    }
  }

  for (int i = 0; i < MAX_TASKS; i++) {
    if (task_table[i].taskCall == NULL) {
      task_table[i].taskCall = taskCall;
      task_table[i].nexttime = micros() + start_delay;
      task_table[i].uid = i;
      task_table[i].running = true;
      return i;
    }
  }
  Serial.printf("Error: No more tasks can be added\n");
  return 0;
}

void remove_task(uint8_t uid) {
  task_table[uid].taskCall = NULL;
  task_table[uid].nexttime = 0;
  task_table[uid].uid = 0;
  task_table[uid].running = false;
}

void pause_task(uint8_t uid) {
  task_table[uid].running = false;
}

void resume_task(uint8_t uid) {
  task_table[uid].running = true;
}

void run_task(uint8_t uid) {
  task_table[uid].taskCall();
}

void clear_all_tasks() {
  for (int i = 0; i < MAX_TASKS; i++) {
    task_table[i].taskCall = NULL;
    task_table[i].nexttime = 0;
    task_table[i].uid = 0;
    task_table[i].running = false;
  }
}

