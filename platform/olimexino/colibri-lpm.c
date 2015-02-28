#include "colibri-lpm.h"

uint8_t colibri_lpm_status = 0;

uint8_t lpm_active(void) {
	return (colibri_lpm_status & LPM_IS_ENABLED) != 0;
}

void lpm_request_enter(void) {
	if(!(colibri_lpm_status & (LPM_IS_ENABLED|LPM_REQUEST)))
		colibri_lpm_status |= LPM_REQUEST;
}

void lpm_request_exit(void) {
	if(colibri_lpm_status & LPM_IS_ENABLED)
		colibri_lpm_status |= LPM_REQUEST;
}
