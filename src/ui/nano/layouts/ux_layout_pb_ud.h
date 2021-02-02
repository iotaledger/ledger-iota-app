/*********************************************************************************
 * ICON
 * 1 bold text line
 */

#include "bolos_target.h"

#include "bagl.h"

typedef struct ux_layout_pb_ud_params_s {
	const bagl_icon_details_t* icon;
	const char* line1;
} ux_layout_pb_ud_params_t;

void ux_layout_pb_ud_init(unsigned int stack_slot);
