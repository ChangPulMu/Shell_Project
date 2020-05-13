

#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "TaskService.h"
#include "HardDisk.h"
#include "FileSystem.h"

void kPrintString( int iX, int iY, const char* pcString );
void kMappingPrintString( int iX, int iY, const char* pcString );

void Main( void )
{	
	int iCursorX, iCursorY;
	char vcTemp[ 2 ] = { 0, };
    	BYTE bFlags;
    	BYTE bTemp;
    	int i = 0;
	KEYDATA stData;

	kInitializeConsole( 0, 10 );
    	kPrintf( "Switch To IA-32e Mode Success~!!\n" );
    	kPrintf( "IA-32e C Language Kernel Start..............[Pass]\n" );
	kPrintf( "initialze Console.......................[Pass]\n" );

	//kMappingPrintString(0, 13, "this is mapping printString!!");


	kGetCursor( &iCursorX, &iCursorY );
 	kPrintf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    	kInitializeGDTTableAndTSS();
    	kLoadGDTR( GDTR_STARTADDRESS );
	kSetCursor( 45, iCursorY++);    	
	kPrintf( "Pass\n" );

	kPrintf( "TSS Segment Load............................[    ]" );
    	kLoadTR( GDT_TSSSEGMENT );
	kSetCursor( 45, iCursorY++);    	
    	kPrintf( "Pass\n" );

    	kPrintf( "IDT Initialize..............................[    ]" );
    	kInitializeIDTTables();    
    	kLoadIDTR( IDTR_STARTADDRESS );
	kSetCursor( 45, iCursorY++);    		
	kPrintf( "Pass\n" );

	kPrintf( "Total RAM Size Check........................[    ]" );
	kCheckTotalRAMSize();
	kSetCursor( 45, iCursorY++);    		
	kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );
	
	kPrintf( "TCB Pool And Scheduler Initialize...........[Pass]\n" );
    	iCursorY++;
    	kInitializeScheduler();
	
	kPrintf( "Dynamic Memory Initialize...................[Pass]\n" );
    	iCursorY++;
    	kInitializeDynamicMemory();
    	kInitializeFreeList();
    	kInitializePIT( MSTOCOUNT( 1 ), 1 );
    
    	kPrintf( "Keyboard Activate And Queue Initialize......[    ]" );
    
	if( kInitializeKeyboard() == TRUE )
    	{
		kSetCursor( 45, iCursorY++);    		
		kPrintf( "Pass\n" );
        	kChangeKeyboardLED( FALSE, FALSE, FALSE );
    	}
    	else
    	{
        	kSetCursor( 45, iCursorY++);    		
		kPrintf( "Fail\n" );
        	while( 1 ) ;
    	}
    	
	kPrintf( "PIC Controller And Interrupt Initialize.....[    ]" );
    	kInitializePIC();
    	kMaskPICInterrupt( 0 );
    	kEnableInterrupt();
 	kSetCursor( 45, iCursorY++);    		
	kPrintf( "Pass\n" );

	kPrintf( "HDD Initialize..............................[    ]" );
    	if( kInitializeHDD() == TRUE )
    	{
        	kSetCursor( 45, iCursorY++ );
        	kPrintf( "Pass\n" );
    	}
    	else
    	{
        	kSetCursor( 45, iCursorY++ );
        	kPrintf( "Fail\n" );
    	}

	kPrintf( "File System Initialize......................[    ]" );
    	if( kInitializeFileSystem() == TRUE )
    	{
        	kSetCursor( 45, iCursorY++ );
        	kPrintf( "Pass\n" );
    	}
    	else
    	{
        	kSetCursor( 45, iCursorY++ );
        	kPrintf( "Fail\n" );
    	}



	kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, 
            	( QWORD ) kIdleTask ,0 ,0 );
		kPrintf( "PassCreateTask\n" );
	//kInitializeTLCPool();    	
	kStartConsoleShell();
	

/*	while( 1 )
    	{
        	if( kGetKeyFromKeyQueue( &stData ) == TRUE )
        	{
            		
                	if( stData.bFlags & KEY_FLAGS_DOWN )
                	{
				vcTemp[0] = stData.bASCIICode;
                    		kPrintString( i++, 19, vcTemp );
				if( vcTemp[0] == '0')
				{
					long* ptr = (long*) 0x1FF000;				
					*ptr = 0;
					
					//bTemp = bTemp / 0;
				}
            		}
        	}
    	}
*/
		
}


void kPrintString( int iX, int iY, const char* pcString )
{
    	CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    	int i;
    
    	pstScreen += ( iY * 80 ) + iX;


    	for( i = 0 ; pcString[ i ] != 0 ; i++ )
    	{
        	pstScreen[ i ].bCharactor = pcString[ i ];
    	}
}

void kMappingPrintString( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xAB8000;
    int i;
    
   
    pstScreen += ( iY * 80 ) + iX;
    
  
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}
