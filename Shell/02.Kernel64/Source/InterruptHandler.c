
#include "InterruptHandler.h"
#include "PIC.h"
#include "PIT.h"
#include "RTC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    	char vcBuffer[ 3 ] = { 0, };
	
	
	
	
	vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
	
	
	kPrintStringXY( 0, 0, "====================================================" );
    	kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    	kPrintStringXY( 0, 2, "                    Vector:                         " );
    	kPrintStringXY( 27, 2, vcBuffer );
    	kPrintStringXY( 0, 3, "====================================================" );
	
	while(1);
}


void kCommonInterruptHandler( int iVectorNumber )
{
    	char vcRBuffer[] = "[Running time=  :  ]";
	char vcCBuffer[] = "[Current time=  :  :  ]";
	int min,sec;
	BYTE bSec, bMin, bHour;
    
	timeCounter++;
	
	if( timerON == 1 )
	{
			
		sec = (timeCounter / 20) % 60;
		vcRBuffer[ 18 ] = '0' + (sec % 10) ;
		vcRBuffer[ 17 ] = '0' + (sec / 10) ;
		if( timeCounter > 1200 )
		{	
			min = timeCounter / 1200;
			vcRBuffer[ 15 ] = '0' + (min % 10) ;			
			vcRBuffer[ 14 ] = '0' + (min / 10) ;									
		}
		else
		{
			vcRBuffer[ 15 ] = '0' ;			
			vcRBuffer[ 14 ] = '0' ;
		}	

		kPrintStringXY( 0, 24, vcRBuffer );	
		
	}

	kReadRTCTime( &bHour, &bMin, &bSec );

	vcCBuffer[ 14 ] = '0' + (bHour / 10);
	vcCBuffer[ 15 ] = '0' + (bHour % 10);
	vcCBuffer[ 17 ] = '0' + (bMin / 10);
	vcCBuffer[ 18 ] = '0' + (bMin % 10);
	vcCBuffer[ 20 ] = '0' + (bSec / 10);
	vcCBuffer[ 21 ] = '0' + (bSec % 10);
	
	kPrintStringXY( 57, 24, vcCBuffer );
			
    	kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}


void kKeyboardHandler( int iVectorNumber )
{
    	char vcBuffer[] = "[INT:  , ]";
    	static int g_iKeyboardInterruptCount = 0;
	BYTE bTemp;

    	vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    	vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    	g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    	kPrintString( 0, 0, vcBuffer );
    	
	if( kIsOutputBufferFull() == TRUE)
	{
		bTemp = kGetKeyboardScanCode();		
		kConvertScanCodeAndPutQueue( bTemp );
	}

	kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

void kPagingHandler( int iVectorNumber, QWORD qwErrorCode, QWORD address )
{
	char hexBuffer[16] = {0, };
	getHex(hexBuffer, address);
	char vcBuffer[ 3 ] = { 0, };
	BYTE bTemp;	

	vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
	
	if( qwErrorCode == 3 ){
		kPrintStringXY( 0, 0, "====================================================" );
    		kPrintStringXY( 0, 1, "             Protection Fult Occur~!!!!             " );
    		kPrintStringXY( 0, 2, "                Address:                            " );
		kPrintStringXY( 27, 2, hexBuffer );
    		kPrintStringXY( 0, 3, "====================================================" );		
	}
	else if( qwErrorCode == 2 ){
		kPrintStringXY( 0, 0, "====================================================" );
    		kPrintStringXY( 0, 1, "                 Page Fult Occur~!!!!               " );
    		kPrintStringXY( 0, 2, "                  Address:                          " );
		kPrintStringXY( 27, 2, hexBuffer );    		
		kPrintStringXY( 0, 3, "====================================================" );
	}
	else
	{
    		kPrintStringXY( 0, 0, "====================================================" );
    		kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    		kPrintStringXY( 0, 2, "                    Vector:                         " );
    		kPrintStringXY( 27, 2, hexBuffer );
    		kPrintStringXY( 0, 3, "====================================================" );
	}
	
	fixPageEntry(address);
		
}


void getHex(char* hexBuffer , QWORD input )
{
	char reHex[16] = {0, }; 
	int position = 0;
	int j = 0;
	while(1)
	{
		int mod = input % 16;
		if(mod <10)
		{
			reHex[position] = 48 + mod;			
		}
		else
		{
			reHex[position] = 65 + (mod - 10);
		}
		input = input / 16;
		position++;
		if(input == 0)
		{
			break;
		}
	}
	for( int i = position - 1 ; i >= 0 ; i--)
	{
		hexBuffer[j] = reHex[i];
		j++;
	}
	
}

void fixPageEntry(QWORD address)
{
	QWORD* page;	
	QWORD entryPoint = address >> 21  ;
	QWORD entryAddress;
	if(entryPoint == 0)
	{
		entryPoint = address >> 12 ;
		entryAddress = 0x102000 + ( entryPoint * 8 ) ;
		page = (QWORD*) entryAddress;
	}
	else
	{
		entryAddress = 0x103000 + ( entryPoint * 8 ) ;
		page = (QWORD*) entryAddress;	
	}
	
	*page = *page | 0x03;	
		
} 

void kTimerHandler( int iVectorNumber )
{
    	char vcBuffer[] = "[INT:  , ]";
    	static int g_iTimerInterruptCount = 0;

    	vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    	vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    	g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
	timeCounter++;	//
	kPrintStringXY( 70, 0, vcBuffer );
    	kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
	if(timeCounter > 1000 )
	{
		timeCounter = 0;
	}
    	g_qwTickCount++;

	kDecreaseProcessorTime();
    	if( kIsProcessorTimeExpired() == TRUE )
    	{
        	kScheduleInInterrupt();
    	}
}

void kDeviceNotAvailableHandler( int iVectorNumber )
{
    	TCB* pstFPUTask, * pstCurrentTask;
    	QWORD qwLastFPUTaskID;

    	char vcBuffer[] = "[EXC:  , ]";
    	static int g_iFPUInterruptCount = 0;

    	vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    	vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    	g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    	kPrintStringXY( 0, 0, vcBuffer );    
    	
	kClearTS();

    	qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    	pstCurrentTask = kGetRunningTask();
    
    	if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    	{
        	return ;
    	}
    	else if( qwLastFPUTaskID != TASK_INVALIDID )
    	{
        	pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        	if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        	{
            		kSaveFPUContext( pstFPUTask->vqwFPUContext );
        	}
    	}
    
    	if( pstCurrentTask->bFPUUsed == FALSE )
    	{
        	kInitializeFPU();
        	pstCurrentTask->bFPUUsed = TRUE;
    	}
    	else
    	{
        	kLoadFPUContext( pstCurrentTask->vqwFPUContext );
    	}
    
    	kSetLastFPUUsedTaskID( pstCurrentTask->stLink.qwID );
}

void kHDDHandler( int iVectorNumber )
{
    	char vcBuffer[] = "[INT:  , ]";
    	static int g_iHDDInterruptCount = 0;
    	BYTE bTemp;

    	vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    	vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    	vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    	g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    	kPrintStringXY( 10, 0, vcBuffer );
    	if( iVectorNumber - PIC_IRQSTARTVECTOR == 14 )
    	{
        	kSetHDDInterruptFlag( TRUE, TRUE );
    	}
    	else
    	{
        	kSetHDDInterruptFlag( FALSE, TRUE );
    	}
    
    	kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

