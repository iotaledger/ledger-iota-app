#ifndef UI_H
#define UI_H

#include <stdint.h>

void ui_init(void);
void ui_reset(void);
void ui_user_confirm_essence(void);

void ui_timer_event(void);

uint8_t ui_show(uint8_t flow);

#endif // UI_H
