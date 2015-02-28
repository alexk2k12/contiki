#ifndef COLIBRI_LPM_H
#define COLIBRI_LPM_H
#include <stdint.h>

/* LPM devices */
#define LPM_DEV_MRF49XA		0x01		// Radio module
#define LPM_DEV_AT25F512B	0x02		// External Flash module
#define LPM_DEV_UCA1		0x04		// is UCA1 used by uart
#define LPM_REQUEST			0x40		// LPM is rquested
#define LPM_IS_ENABLED		0x80		// LPM is active

#define LPM_IS_DISABLED		0x00
#define LPM_DEV_ALL			0x07

#define IS_LPM_REQUESTED(X) (((colibri_lpm_status & LPM_REQUEST) != 0) && ((colibri_lpm_status & LPM_IS_ENABLED) == X))
#define CLEAR_LPM_REQUEST(X) colibri_lpm_status = (colibri_lpm_status & ~(LPM_REQUEST|LPM_IS_ENABLED)) | X

extern uint8_t colibri_lpm_status;

uint8_t lpm_active(void);
void lpm_request_enter(void);
void lpm_request_exit(void);

#endif
