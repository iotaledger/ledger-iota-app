#ifndef UI_H
#define UI_H

#include <stdint.h>

enum {
    FLOW_MAIN_MENU = 0,
    FLOW_GENERATING_ADDRESSES = 1,
    FLOW_GENERIC_ERROR = 2,
    FLOW_REJECTED = 3,
    FLOW_SIGNED_SUCCESSFULLY = 4,
    FLOW_SIGNING = 5,
};

void ui_init(void);
void ui_reset(void);
void ui_user_confirm_essence(void);

void ui_timer_event(void);

uint8_t ui_show(uint8_t flow);

#endif // UI_H
