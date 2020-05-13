
#include "TaskService.h"
#include "Task.h"
#include "Descriptor.h"
#include "Synchronization.h"
#include "Utility.h"
#include "Keyboard.h"

HISTORY* gs_history = HISTORYADDRESS;
TLCPOOL* gs_TLCpool = TLCPOOLADDRESS;

void kInitializeTLCPool( void )
{						
	kMemSet( TLCPOOLADDRESS, 0, sizeof( TLCPOOL ) );					
}

void TLCManager( void )
{
	TLC* pstTLC;
	int i = 0;	
	
	//kPrintf("setTLC...\n");
	while( gs_history -> hTask[ i+1 ].iNum != 0 )	
	{
		pstTLC = searchTLC( gs_history -> hTask[ i ].iNum , gs_history -> hTask[ i+1 ].iNum, 1 );
		pstTLC -> weight += 50;
		i++;
	}
	i = 0;
	while( gs_history -> hTask[ i+2 ].iNum != 0 )	
	{
		pstTLC = searchTLC( gs_history -> hTask[ i ].iNum , gs_history -> hTask[ i+2 ].iNum, 2 );
		pstTLC -> weight += 50;
		i++;
	}
	i = 0;
	while( gs_history -> hTask[ i+3 ].iNum != 0 )	
	{
		pstTLC = searchTLC( gs_history -> hTask[ i ].iNum , gs_history -> hTask[ i+3 ].iNum, 3 );
		pstTLC -> weight += 35;
		i++;
	}	
	
}

TLC* searchTLC( int a, int b, int level )
{
	TLC* pickTLC;
	int i = 0;
	
	
	pickTLC = NULL;
	//kPrintf("searchTLC...\n");
	if( gs_TLCpool -> tlc[ i ].fINum == 0 )
	{
		createTLC( &(gs_TLCpool -> tlc[ i ]), a, b);
		
		return &(gs_TLCpool -> tlc[ i ]);
	}
	while( gs_TLCpool -> tlc[ i ].fINum != 0 )
	{
		if( gs_TLCpool -> tlc[ i ].fINum == a )
		{
			if(gs_TLCpool -> tlc[ i ].bINum == b)			
			{
				pickTLC = &( gs_TLCpool -> tlc[ i ] );			
			}
			else 
			{
				if(level == 1 && gs_TLCpool -> tlc[ i ].weight > 0)
				{				
					gs_TLCpool -> tlc[ i ].weight -= 25; 
				}
			}
		}
		i++;
	}
	if( pickTLC == NULL )
	{
		createTLC( &(gs_TLCpool -> tlc[ i ]), a, b ); 
		pickTLC = &(gs_TLCpool -> tlc[ i ]);		
	}
	return pickTLC;
}

void createTLC( TLC* pstTLC, int a, int b )
{	
	
	pstTLC -> fINum = a;
	pstTLC -> bINum = b;
	pstTLC -> weight = 0;
	
}

void checkTLC( TASKINFO taskInfo )
{
	TLC tlc;
	int count = 0;
	
	while( gs_TLCpool -> tlc[count].fINum != 0 )
	{
		if( gs_TLCpool -> tlc[count].fINum == taskInfo.iNum )			
		{
			if( gs_TLCpool -> tlc[count].weight > 1000 )
			{
				if( searchTask( ( TASKINFO )( getTaskInfo( gs_TLCpool -> tlc[count].bINum ) ) ) == TRUE )
				{
					gs_TLCpool -> tlc[count].weight += 200;					
					break;	
				}
				gs_TLCpool -> tlc[count].weight -= 300;
				break;	
			}
		}
		count++;
	}
}

TASKINFO getTaskInfo( int iNum )
{
	TASKINFO taskInfo;
	char* nameBuff;	
	switch( iNum )
	{
		case 1: 
			taskInfo.iNum = iNum;
			nameBuff = "testTask1";
			kMemCpy( &( taskInfo.name ), nameBuff, sizeof( nameBuff ) + 1 );
			break;
		case 2: 
			taskInfo.iNum = iNum;
			nameBuff = "testTask2";
			kMemCpy( &( taskInfo.name ), nameBuff, sizeof( nameBuff ) + 1 );
			break;
		case 3: 
			taskInfo.iNum = iNum;
			nameBuff = "testTask3";
			kMemCpy( &( taskInfo.name ), nameBuff, sizeof( nameBuff ) + 1 );
			break;
		default:
			break;	
	}
	return taskInfo;
}

void getTLCInfo( void )
{
	TLC tlc;	
	int count;
	
	while( gs_TLCpool -> tlc[count].fINum != 0 )
	{
		tlc = gs_TLCpool -> tlc[count];
		kPrintf( " a[%d] -> b[%d] weight:[%d]\n " , tlc.fINum , tlc.bINum , tlc.weight );
		count++;
	}
	kPrintf( "TLC coutn = [%d]\n ", count );
}

void putHistory( TASKINFO taskInfo )
{
	int i = gs_history -> count;
	gs_history -> hTask[ i ] = taskInfo;
	gs_history -> count++;
	if( ( gs_history -> count ) > 30 )
	{
		gs_history -> count = 0;
	}
		
}

void removeHistory( TASKINFO taskInfo )
{
	int iNum;
	int i, j;

	i = 0;
	iNum = taskInfo.iNum;	
	while(1)
	{
		if(gs_history -> hTask[ i ].iNum == iNum)
		{
			kMemSet( &( gs_history -> hTask[ i ] ), 0, sizeof( TASKINFO ) );					
			gs_history -> count--;	
			for( j = i ; j < MAX_HTASKCOUNT - 1 ; j++ )
			{
				gs_history -> hTask[ j ] = gs_history -> hTask[ j+1 ];	 
			}
			break;
		}
		i++;
		if( i > MAX_HTASKCOUNT )
		{
			break;	
		}	
	}
}

void kInitializeHistory( void )
{
	int i;
	gs_history -> count = 0;
	for( i = 0 ; i < MAX_HTASKCOUNT ; i++)
	{
		kMemSet( &( gs_history -> hTask[ i ] ), 0, sizeof( TASKINFO ) );					
	}
}

void getHistoryInfo( int* count, int* hTaskBuff )
{	
	int i;
	
	*count = gs_history -> count;
	for( i = 0 ; i < ( gs_history -> count ) ; i++ )
	{
		hTaskBuff[i] = gs_history -> hTask[i].iNum;
	}	
}

void checkPreHistory( void )
{
	HISTORY preHistory;
	int i;
	kPrintf( "checkHistory...\n" );

	kMemCpy( &preHistory , gs_history , sizeof( HISTORY ) );
	kInitializeHistory();
	for( i = 0 ; i < preHistory.count ; i++ )
	{
		if( preHistory.hTask[ i ].iNum != 0 )
		{
			kPrintf( "is History \n" );		
			searchTask( preHistory.hTask[ i ] );
		}
	}
}

BOOL searchTask( TASKINFO taskInfo )
{
	char* taskName;
	BOOL isRunTask;
	
	taskName = taskInfo.name;
	isRunTask = sendMessage( taskInfo );	
	if(isRunTask == TRUE)
	{
		if( taskInfo.iNum == 1 )
		{
			kCreateTask( TASK_FLAGS_MEDIUM | TASK_FLAGS_PROCESS, 0, 0, ( QWORD ) kTestTask1, "testTask1", 1 );
			kPrintf( "Run testTask1 \n" );		
		}
		else if( taskInfo.iNum == 2 )
		{
			kCreateTask( TASK_FLAGS_MEDIUM | TASK_FLAGS_PROCESS, 0, 0, ( QWORD ) kTestTask2, "testTask2", 2 );
			kPrintf( "Run testTask2 \n" );		
		}
		else if( taskInfo.iNum == 3 )
		{
			kCreateTask( TASK_FLAGS_MEDIUM | TASK_FLAGS_PROCESS, 0, 0, ( QWORD ) kTestTask3, "testTask3", 3 );
			kPrintf( "Run testTask2 \n" );		
		}
		return TRUE;
	}
	return FALSE;

}

BOOL sendMessage( TASKINFO taskInfo )
{
	char vcCommandBuffer[ 10 ];
    	int iCommandBufferIndex = 0;
    	BYTE bKey;
   	int iCursorX, iCursorY;
	BOOL isRunTask;
	BOOL bPreviousFlag;

    	bPreviousFlag = kLockForSystemData();    
	isRunTask  = FALSE;
	kPrintf( "are you start [%s]?   y/n \n", taskInfo.name );
	while(1)
	{
		bKey = kGetCh();
		if( bKey == KEY_BACKSPACE )
        	{
            		if( iCommandBufferIndex > 0 )
            		{
                	kGetCursor( &iCursorX, &iCursorY );
                	kPrintStringXY( iCursorX - 1, iCursorY, " " );
                	kSetCursor( iCursorX - 1, iCursorY );
                	iCommandBufferIndex--;
        	    	}
        	}
		else if( bKey == KEY_ENTER )
        	{
        		kPrintf( "\n" );
            
            		if( iCommandBufferIndex > 0 )
            		{
				if( vcCommandBuffer[ 0 ] == 'y')
				{
					isRunTask = TRUE;
					break;
				}
				else
				{
										
					break;	
				}				
            		}
            
          	}
		else if( iCommandBufferIndex < 10 )
            	{
                	vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
                	kPrintf( "%c", bKey );
            	}
	}
        
	kUnlockForSystemData( bPreviousFlag );

	return isRunTask;	
}
