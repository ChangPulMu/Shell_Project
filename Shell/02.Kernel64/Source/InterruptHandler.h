

#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"


void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kCommonInterruptHandler( int iVectorNumber );
void kKeyboardHandler( int iVectorNumber );
void kPagingHandler( int iVectorNumber, QWORD qwErrorCode, QWORD address );
void fixPageEntry(QWORD address);
void kTimerHandler( int iVectorNumber );
void kDeviceNotAvailableHandler( int iVectorNumber );

void kHDDHandler( int iVectorNumber );

#endif /*__INTERRUPTHANDLER_H__*/
