/* Author: Mark Faust; Thomas M. DuBuisson
 * Description: This file defines the two required functions for the branch predictor.
*/

#include <stdint.h>
#include "predictor.h"

// Sizes in cells
#define LocalHistoryTableSize (1024)
#define LocalPredictionSize (1024)
#define GlobalPredictionSize (4096)
#define ChoicePredictionSize (4096)

#define PCtoLHistIdx(x) { (x & 0x3FF) }

// Debug levels
#define CRIT  0
#define ERROR 1
#define WARN  2
#define INFO1 3
#define INFO2 4
#define HAPPY_FUN_BALL 5

typedef uint16_t PathHistory; // a 3 bytes value of path history
typedef unsigned int PC;      // matching tread.h

/*** Debugging ***/
define debug(w,s,args...) { if (w <= DEBUG_LEVEL) { fprintf(stderr, s,##args); } }

#ifndef DEFAULT_DEBUG
#define DEFAULT_DEBUG CRIT
#endif

static uint8_t DEBUG_FLAG = DEFAULT_DEBUG;

/*** Saturation Counter Macros ***/
#define MAX(a,b) { (a > b ? a : b) }
#define MIN(a,b) { (a < b ? a : b) }

#define INC_SAT3(x) {  MAX(x,x + 1 & 0x7) }
#define DEC_SAT3(x) {  MIN(x,x - 1 & 0x7) }
#define SAT3_IS_HIGH(x) { (x >= 4) }

#define INC_SAT2(x) {  MAX(x, x + 1 & 0x3) }
#define DEC_SAT2(x) {  MIN(x, x - 1 & 0x3) }
#define SAT2_IS_HIGH(x) { (x >= 2) }

/*** Globals (Tables used by the predictor) ***/

// 10 bit pointers into the local predictor table
static uint16_t gLocalHistoryTable[LocalHistoryTableSize];

// 3 bit predictor (TODO: Behavior???)
static uint8_t gLocalPrediction[LocalPredictionSize];

// 2 bit saturating counter
static uint8_t gGlobalPrediction[GlobalPredictionSize];

// 2 bit predictor (TODO: Behavior???)
static uint8_t gChoicePrediction[ChoicePredictionSize];

// 12 bit path history.  Oldest branch observed is in the MSB
// (2^11 == oldest branch in history, 2^0 == most recent branch)
static PathHistory gPathHistory;

// Accessor functions for the tables and path history.  Using these we
// abstract away from the in-memory representation, allowing the
// arrays to be packed if needed.
uint16_t lookupLocalHistoryTable(PC x)
{
  debug(INFO2, "Looking up the local history for PC of %0x\n", x);
  return gLocalHistoryTable[PCtoLHistIdx(x)];
}

// Get the 3 bit local predictor based on the history
uint8_t lookupLocalPrediction(uint16_t offset)
{
  debug(INFO2, "Looking up the local prediction for offset: %d\n", offset);
  return gLocalPrediction[offset]
}

// Get the 3 bit local predictor by first getting this history from the LocalHistoryTable
uint8_t getLocalPrediction(PC x)
{
  debug(INFO2, "Looking up the local prediction for PC: %0x\n", x);
  return (lookupLocalPrediction( lookupLocalHistoryTable(x) ));
}

// Get the 2 bit global prediction based on path history
uint8_t lookupGlobalPrediction(PathHistory x)
{
  debug(INFO2, "Looking up the global prediction for history: %03x\n", x);
  return gGlobalPrediction[x];
}

uint8_t lookupGlobalPrediction()
{
  return gGlobalPrediction[gPathHistory];
}

uint8_t lookupChoicePrediction(PathHistory x)
{
  debug(INFO2, "Looking up the choice prediction for history: %03x\n",x);
  return gLocalChoice[x];
}

uint8_t lookupChoicePrediction()
{
  return gLocalChoice[gPathHistory];
}

void updateChoicePrediction(const branch_record_c *br)
{
  // TODO determine behavior of this predictor
}

void updateGlobalPrediction(PathHistory x, const branch_record_c *br)
{
  // TODO gGlobalPrediction is a two bit saturation counter
}

void updateLocalPrediction(uint16_t offset, const branch_record_c *br)
{
  uint8_t h = gLocalPrediction[offset];
  // TODO this is a three bit preditor of unknown behavior
}

void updateLocalHistoryTable(PC x, bool t)
{
  int idx = PCToLHistIdx(x);
  uint16_t h = 0;
  h = gLocalHistoryTable[idx];
  h = h << 1 | (t ? 1 : 0);
  h &= 0x3FF;
  LocalHistoryTable[idx] = h;
}

void updatePathHistory(bool t)
{
  gPathHistory = gPathHistory << 1 + (t ? 1 : 0);
}

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
  /* replace this code with your own */
  bool prediction = false;

  printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
	 br->branch_target,br->is_indirect,br->is_conditional,
	 br->is_call,br->is_return);
  if (br->is_conditional)
    prediction = true;
  return prediction;   // true for taken, false for not taken
}

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
  /* replace this code with your own */
  printf("%1d\n",taken);

}
