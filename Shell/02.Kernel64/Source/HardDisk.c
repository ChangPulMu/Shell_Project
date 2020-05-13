
#include "HardDisk.h"

static HDDMANAGER gs_stHDDManager;

BOOL kInitializeHDD(void)
{
	kInitializeMutex( &( gs_stHDDManager.stMutex ) );

    	gs_stHDDManager.bPrimaryInterruptOccur = FALSE;
    	gs_stHDDManager.bSecondaryInterruptOccur = FALSE;

    	kOutPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    	kOutPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_DIGITALOUTPUT, 0 );
    
    	if( kReadHDDInformation( TRUE, TRUE, &( gs_stHDDManager.stHDDInformation ) ) 
    	        == FALSE )
    	{
        	gs_stHDDManager.bHDDDetected = FALSE;
        	gs_stHDDManager.bCanWrite = FALSE;
        	return FALSE;
    	}

    	gs_stHDDManager.bHDDDetected = TRUE;
    	if( kMemCmp( gs_stHDDManager.stHDDInformation.vwModelNumber, "QEMU", 4 ) == 0 )
    	{
        	gs_stHDDManager.bCanWrite = TRUE;
    	}
    	else
    	{
        	gs_stHDDManager.bCanWrite = FALSE;
    	}
    	return TRUE;
}

static BYTE kReadHDDStatus( BOOL bPrimary )
{
    	if( bPrimary == TRUE )
    	{
       		return kInPortByte( HDD_PORT_PRIMARYBASE + HDD_PORT_INDEX_STATUS );
    	}	
    	return kInPortByte( HDD_PORT_SECONDARYBASE + HDD_PORT_INDEX_STATUS );
}

static BOOL kWaitForHDDNoBusy( BOOL bPrimary )
{
    	QWORD qwStartTickCount;
    	BYTE bStatus;

    	qwStartTickCount = kGetTickCount();

    	while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    	{
        	bStatus = kReadHDDStatus( bPrimary );

        	if( ( bStatus & HDD_STATUS_BUSY ) != HDD_STATUS_BUSY )
        	{
            		return TRUE;
        	}
        	kSleep( 1 );
    	}
    	return FALSE;
}

static BOOL kWaitForHDDReady(BOOL bPrimary)
{
    	QWORD qwStartTickCount;
    	BYTE bStatus;

    	qwStartTickCount = kGetTickCount();

    	while( ( kGetTickCount() - qwStartTickCount ) <= HDD_WAITTIME )
    	{
        	bStatus = kReadHDDStatus( bPrimary );

        	if( ( bStatus & HDD_STATUS_READY ) == HDD_STATUS_READY )
        	{
            		return TRUE;
        	}
        	kSleep( 1 );
    	}
    	return FALSE;
}

void kSetHDDInterruptFlag( BOOL bPrimary, BOOL bFlag )
{
    	if( bPrimary == TRUE )
    	{
        	gs_stHDDManager.bPrimaryInterruptOccur = bFlag;
    	}
    	else
    	{
        	gs_stHDDManager.bSecondaryInterruptOccur = bFlag;
    	}
}

static BOOL kWaitForHDDInterrupt( BOOL bPrimary )
{
    	QWORD qwTickCount;
    
    	qwTickCount = kGetTickCount();
    
    	while( kGetTickCount() - qwTickCount <= HDD_WAITTIME )
    	{
        	if( ( bPrimary == TRUE ) && 
            		( gs_stHDDManager.bPrimaryInterruptOccur == TRUE ) )
        	{
            		return TRUE;
        	}
        	else if( ( bPrimary == FALSE ) && 
                 	( gs_stHDDManager.bSecondaryInterruptOccur == TRUE ) )
        	{
            		return TRUE;
        	}
    	}
    	return FALSE;
}

BOOL kReadHDDInformation( BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation )
{
    	WORD wPortBase;
    	QWORD qwLastTickCount;
   	BYTE bStatus;
    	BYTE bDriveFlag;
    	int i;
   	WORD wTemp;
    	BOOL bWaitResult;

    	if( bPrimary == TRUE )
    	{
    		wPortBase = HDD_PORT_PRIMARYBASE;
    	}
    	else
    	{
        	wPortBase = HDD_PORT_SECONDARYBASE;
    	}

    	kLock( &(gs_stHDDManager.stMutex ) );
    
    	if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    	{
        	kUnlock( &(gs_stHDDManager.stMutex ) );
        	return FALSE;
    	}
    
    	if( bMaster == TRUE )
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    	}
    	else
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    	}
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag );

    	if( kWaitForHDDReady( bPrimary ) == FALSE )
    	{
        	kUnlock( &(gs_stHDDManager.stMutex ) );
        	return FALSE;
    	}

    	kSetHDDInterruptFlag( bPrimary, FALSE );
    
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_IDENTIFY );

    	bWaitResult = kWaitForHDDInterrupt( bPrimary );
    	bStatus = kReadHDDStatus( bPrimary );
    	if( ( bWaitResult == FALSE ) || 
        	( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR ) )
    	{
        	kUnlock( &( gs_stHDDManager.stMutex ) );
        	return FALSE;
    	}

    	for( i = 0; i < 512 / 2; i++ )
    	{
        	( ( WORD* ) pstHDDInformation )[ i ] =
        	kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
    	}

    	kSwapByteInWord( pstHDDInformation->vwModelNumber,
            	sizeof( pstHDDInformation->vwModelNumber ) / 2 );
   	kSwapByteInWord( pstHDDInformation->vwSerialNumber,
            	sizeof( pstHDDInformation->vwSerialNumber ) / 2 );

	kUnlock( &(gs_stHDDManager.stMutex ) );
	return TRUE;
}

static void kSwapByteInWord(WORD* pwData, int iWordCount)
{
    	int i;
    	WORD wTemp;

    	for (i = 0; i < iWordCount; i++)
    	{
        	wTemp = pwData[ i ];
        	pwData[ i ] = (wTemp >> 8 ) | (wTemp << 8 );
    	}
}

int kReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer )
{
    	WORD wPortBase;
    	int i, j;
    	BYTE bDriveFlag;
    	BYTE bStatus;
    	long lReadCount = 0;
    	BOOL bWaitResult;

    	if( ( gs_stHDDManager.bHDDDetected == FALSE ) ||
        	( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        	( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )
    	{
        	return 0;
    	}

    	if( bPrimary == TRUE )
    	{
        	wPortBase = HDD_PORT_PRIMARYBASE;
    	}
    	else
    	{
        	wPortBase = HDD_PORT_SECONDARYBASE;
    	}

    	kLock( &( gs_stHDDManager.stMutex ) );
    
    	if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    	{
        	kUnlock( &( gs_stHDDManager.stMutex ) );
        	return FALSE;
    	}

	kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
   	kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    	if( bMaster == TRUE )
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA;
    	}
    	else
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    	}
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            	>> 24 ) & 0x0F ) );

    	if( kWaitForHDDReady( bPrimary ) == FALSE )
    	{
        	kUnlock( &( gs_stHDDManager.stMutex ) );
        	return FALSE;
    	}

    	kSetHDDInterruptFlag( bPrimary, FALSE );
    
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_READ );

    	for( i = 0 ; i < iSectorCount ; i++ )
    	{
        	bStatus = kReadHDDStatus( bPrimary );
        	if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        	{
            		kPrintf( "Error Occur\n" );
            		kUnlock( &( gs_stHDDManager.stMutex ) );
            		return i;
        	}

        	if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        	{
            		bWaitResult = kWaitForHDDInterrupt( bPrimary );
            		kSetHDDInterruptFlag( bPrimary, FALSE );
            		if( bWaitResult == FALSE )
            		{
                		kPrintf( "Interrupt Not Occur\n" );
                		kUnlock( &( gs_stHDDManager.stMutex ) );
                		return FALSE;
            		}
        	}
        
        	for( j = 0 ; j < 512 / 2 ; j++ )
        	{
            		( ( WORD* ) pcBuffer )[ lReadCount++ ]
                    	= kInPortWord( wPortBase + HDD_PORT_INDEX_DATA );
        	}
    	}

    	kUnlock( &( gs_stHDDManager.stMutex ) );
    	return i;
}

int kWriteHDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
        char* pcBuffer)
{
    	WORD wPortBase;
    	WORD wTemp;
    	int i, j;
    	BYTE bDriveFlag;
    	BYTE bStatus;
    	long lReadCount = 0;
    	BOOL bWaitResult;

    	if( ( gs_stHDDManager.bCanWrite == FALSE ) || 
        	( iSectorCount <= 0 ) || ( 256 < iSectorCount ) ||
        	( ( dwLBA + iSectorCount ) >= gs_stHDDManager.stHDDInformation.dwTotalSectors ) )

    	{
    		return 0;
    	}

    	if( bPrimary == TRUE )
    	{
        	wPortBase = HDD_PORT_PRIMARYBASE;
    	}
    	else
    	{
        	wPortBase = HDD_PORT_SECONDARYBASE;
    	}

    	if( kWaitForHDDNoBusy( bPrimary ) == FALSE )
    	{
        	return FALSE;
    	}

	kLock( &(gs_stHDDManager.stMutex ) );

    	kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORCOUNT, iSectorCount );
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_SECTORNUMBER, dwLBA );
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERLSB, dwLBA >> 8 );
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_CYLINDERMSB, dwLBA >> 16 );
    	if( bMaster == TRUE )
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA;
   	}
    	else
    	{
        	bDriveFlag = HDD_DRIVEANDHEAD_LBA | HDD_DRIVEANDHEAD_SLAVE;
    	}
    	kOutPortByte( wPortBase + HDD_PORT_INDEX_DRIVEANDHEAD, bDriveFlag | ( (dwLBA
            	>> 24 ) & 0x0F ) );

    	if( kWaitForHDDReady( bPrimary ) == FALSE )
    	{
        	kUnlock( &( gs_stHDDManager.stMutex ) );
       		return FALSE;
    	}

    	kOutPortByte( wPortBase + HDD_PORT_INDEX_COMMAND, HDD_COMMAND_WRITE );
    
    	while( 1 )
    	{
        	bStatus = kReadHDDStatus( bPrimary );
        	if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        	{
            		kUnlock( &(gs_stHDDManager.stMutex ) );
            		return 0;
        	}
        
        	if( ( bStatus & HDD_STATUS_DATAREQUEST ) == HDD_STATUS_DATAREQUEST )
        	{
            		break;
        	}
        
        	kSleep( 1 );        
    	}

    	for( i = 0 ; i < iSectorCount ; i++)
    	{
        	kSetHDDInterruptFlag( bPrimary, FALSE );
        	for( j = 0 ; j < 512 / 2 ; j++ )
        	{
            		kOutPortWord( wPortBase + HDD_PORT_INDEX_DATA, 
                         	( ( WORD* ) pcBuffer )[ lReadCount++ ]);
        	}
        
        	bStatus = kReadHDDStatus( bPrimary );
        	if( ( bStatus & HDD_STATUS_ERROR ) == HDD_STATUS_ERROR )
        	{
            		kUnlock( &(gs_stHDDManager.stMutex ) );
            		return i;
        	}
        
        	if( ( bStatus & HDD_STATUS_DATAREQUEST ) != HDD_STATUS_DATAREQUEST )
        	{
            		bWaitResult = kWaitForHDDInterrupt( bPrimary );
            		kSetHDDInterruptFlag( bPrimary, FALSE );
            		if( bWaitResult == FALSE )
            		{
                		kUnlock( &( gs_stHDDManager.stMutex ) );
                		return FALSE;
            		}
        	}        
    	}
    
    	kUnlock( &(gs_stHDDManager.stMutex ) );
    	return i;
}
