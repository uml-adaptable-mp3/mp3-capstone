//
//
// Functions for use of Reed Solomon codec IN FLASH INTERFACE ONLY
//
//

#ifndef RSFUNC_H
#define RSFUNC_H

#include <vs1005h.h>
#include <vo_stdio.h>
#include <timers.h>
#include "compact.h"

//
// Usage:
// u_int16 wAddr    : Address of first word
// u_int16 bytes    : number of bytes to be processed at one run
// u_int16 rfMode   : selects codec mode 1: for rf-link RS(255,223)
//                                       0: for Nflash RS(1023, 1015)
// u_int16 start    : value 1 requests codec reset before starting
//                    value 0 continues from previous state
// u_int16 endLast  : value 1 completes operation after last byte
//                  : value 0 stops operation after last bytes and waits for more
// 


// Note: if inputting parity for 10bit code, rf_mode must be 0 and end_last 1
// 10 bit parity is always processed in single run.
void NFRsDecode(u_int16 wAddr, u_int16 bytes, u_int16 start, u_int16 endLast);

// Reads parity from encoder
// data_pntr is pointer to table for parity data
// if encoder used rf_mode==1 this results in 16 16bit words containing 32 8bit parity symbols
// in case of rf_mode==0 result is 5 16bit words with custom bit order
void NFReadEncParity(u_int16 *data, u_int16 bytes);

// Reads decoding results and makes corrections if needed (and possible)
// data_pntr is pointer to received code payload (parity already discarded)
// corrected_pntr is pointer to int for outputting the number of corrected errors is payload
// total_errors_pntr is pointer to int for outputting number of total errors (parity included)

// Returns:
// 0 : Success
// 1 : Syndrome failure, check inputs
// 2 : Decoder failure, too many errors to correct
u_int16 NFRsFix(u_int16 *data, u_int16 *meta, u_int16 *corrected, u_int16 *totalErrors, u_int16 *locations, u_int16 *magnitudes);

// Reordering for 10bit parity from 8bit mode(memory interface) to 10bit mode(decoder input)
void RsParity10Reorder(u_int16 *source_pntr, u_int16 *target_pntr);


#endif
