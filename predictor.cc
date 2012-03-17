/* Author: Mark Faust; Thomas M. DuBuisson
 * Description: This file defines the two required functions for the branch predictor.
 *
 *
 * Nomenclature:
 *
 * "lookup" - obtain a counter value from the associated global table.
 *            These functions are sometimes overloaded.
 * "get"    - obtain a boolean prediction by referencing the global tables.
 * "update" - increment or decrement the associated global values using the actual
 *            result of a branch.
 */

#include <stdio.h>
#include <stdint.h>
#include "predictor.h"

// Sizes in cells
#define LocalHistoryTableSize (1024)
#define LocalPredictionSize (1024)
#define GlobalPredictionSize (4096)
#define ChoicePredictionSize (4096)

#define PCtoLHistIdx(x) (x & 0x3FF)

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
#define debug(w,s,args...) { if (w <= DEBUG_LEVEL) { fprintf(stderr, s, ##args); } }

#ifndef DEFAULT_DEBUG
#define DEFAULT_DEBUG CRIT
#endif

static uint8_t DEBUG_LEVEL = DEFAULT_DEBUG;

/*** Saturation Counter Macros ***/
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#define INC_SAT3(x) (MAX(x, ((x + 1) & 0x7)))
#define DEC_SAT3(x) (MIN(x, ((x - 1) & 0x7)))
#define SAT3_IS_HIGH(x) (x >= 4)

#define INC_SAT2(x) (MAX(x, ((x + 1) & 0x3)))
#define DEC_SAT2(x) (MIN(x, ((x - 1) & 0x3)))
#define SAT2_IS_HIGH(x) (x >= 2)

/*** Globals (Tables used by the predictor) ***/
static bool firstRun = true;

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
  debug(INFO2, "Looking up the local history for PC of %x\n", x);
  return gLocalHistoryTable[PCtoLHistIdx(x)];
}

// Get the 3 bit local predictor based on the history
/*
uint8_t lookupLocalPrediction(uint16_t offset)
{
  let idx = PCtoLHistIdx(x);
  debug(INFO2, "Looking up the local prediction for offset: %d\n", idx);
  return gLocalPrediction[idx];
}
*/

// Get the 3 bit local predictor by first getting this history from the LocalHistoryTable
uint8_t lookupLocalPrediction(PC x)
{
  debug(INFO2, "Looking up the local prediction for PC: %x\n", x);
  return (gLocalPrediction[lookupLocalHistoryTable(x)]);
}

bool getLocalPrediction(PC x)
{
  return (SAT3_IS_HIGH(lookupLocalPrediction(x)));
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

bool getGlobalPrediction()
{
  return (SAT2_IS_HIGH(lookupGlobalPrediction()));
}

uint8_t lookupChoicePrediction()
{
  return gChoicePrediction[gPathHistory];
}

// Assuming the gPathHistory has yet to be updated,
// update the choice prediction
void updateChoicePrediction(const branch_record_c *br, bool t)
{
  bool g,l;

  g = getGlobalPrediction();
  l = getLocalPrediction(br->instruction_addr);

  if (g != l) {
    if(g == t) { // global was correct
      gChoicePrediction[gPathHistory] = INC_SAT2(gChoicePrediction[gPathHistory]);
    } else { // local was correct
      gChoicePrediction[gPathHistory] = DEC_SAT2(gChoicePrediction[gPathHistory]);
    }
  }
}

void updateGlobalPrediction(bool t)
{
  if(t) {
    gGlobalPrediction[gPathHistory] = INC_SAT2(gGlobalPrediction[gPathHistory]);
  } else {
    gGlobalPrediction[gPathHistory] = DEC_SAT2(gGlobalPrediction[gPathHistory]);
  }
}

// Assumes the local history table has not been updated yet!
void updateLocalPrediction(PC x, bool t)
{
  uint16_t lclHist = lookupLocalHistoryTable(x);

  if(t) {
    gLocalPrediction[lclHist] = INC_SAT3(gLocalPrediction[lclHist]);
  } else {
    gLocalPrediction[lclHist] = DEC_SAT3(gLocalPrediction[lclHist]);
  }

}

// Maps the last ten bits of the PC to the 10 bit branch taken/not-taken history.
void updateLocalHistoryTable(PC x, bool t)
{
  int idx = PCtoLHistIdx(x);
  uint16_t h = 0;
  h = gLocalHistoryTable[idx];
  h = h << 1 | (t ? 1 : 0);
  h &= 0x3FF;
  gLocalHistoryTable[idx] = h;
}

void updateLocal(PC x, bool t)
{
  updateLocalPrediction(x,t);
  updateLocalHistoryTable(x,t);
}

// This should be the last global to be updated!
void updatePathHistory(bool t)
{
  gPathHistory = 0x3FF & ((gPathHistory << 1) + (t ? 1 : 0));
}

// TODO: Assuming all zero init.  All saturated HIGH might be the truth though...
static void init()
{
  int i;
  firstRun = false;
  for(i=0; i< LocalHistoryTableSize; i++)
    gLocalHistoryTable[i] = 0;

  for(i=0; i < LocalPredictionSize; i++)
    gLocalPrediction[i] = 0;

  for(i=0; i < GlobalPredictionSize; i++)
    gGlobalPrediction[i] = 0;

  for(i=0; i < ChoicePredictionSize; i++)
    gChoicePrediction[i] = 0;

  gPathHistory = 0;
}

bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
  bool prediction;
  uint8_t c;

  return true;

  if(firstRun) init();

  c = lookupChoicePrediction();
  if(SAT2_IS_HIGH(c)) {
    prediction = getGlobalPrediction();
  } else {
    prediction = getLocalPrediction(br->instruction_addr);
  }

  //  printf("%0x %0x %1d %1d %1d %1d ",br->instruction_addr,
  //	 br->branch_target,br->is_indirect,br->is_conditional,
  //	 br->is_call,br->is_return);
  return prediction;   // true for taken, false for not taken
}

// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
  if(firstRun) init();

  updateChoicePrediction(br,taken);
  updateLocal(br->instruction_addr,taken);
  updateGlobalPrediction(taken);
  updatePathHistory(taken);
}
