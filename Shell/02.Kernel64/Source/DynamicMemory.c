
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"

static DYNAMICMEMORY gs_stDynamicMemory;
static MALLOCMANAGER gs_sMallocManager;

void kInitializeDynamicMemory( void )
{
    	QWORD qwDynamicMemorySize;
    	int i, j;
    	BYTE* pbCurrentBitmapPosition;
    	int iBlockCountOfLevel, iMetaBlockCount;

    	qwDynamicMemorySize = kCalculateDynamicMemorySize();
    	iMetaBlockCount = kCalculateMetaBlockCount( qwDynamicMemorySize );

  	gs_stDynamicMemory.iBlockCountOfSmallestBlock = 
        	( qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE ) - iMetaBlockCount;

    	for( i = 0 ; ( gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    	{
        	;
    	}
    	gs_stDynamicMemory.iMaxLevelCount = i;
    
    	gs_stDynamicMemory.pbAllocatedBlockListIndex = ( BYTE* ) DYNAMICMEMORY_START_ADDRESS;
    	for( i = 0 ; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock ; i++ )
    	{
        	gs_stDynamicMemory.pbAllocatedBlockListIndex[ i ] = 0xFF;
    	}
    
    	gs_stDynamicMemory.pstBitmapOfLevel = ( BITMAP* ) ( DYNAMICMEMORY_START_ADDRESS +
        	( sizeof( BYTE ) * gs_stDynamicMemory.iBlockCountOfSmallestBlock ) );
    	pbCurrentBitmapPosition = ( ( BYTE* ) gs_stDynamicMemory.pstBitmapOfLevel ) + 
        	( sizeof( BITMAP ) * gs_stDynamicMemory.iMaxLevelCount );
    	for( j = 0 ; j < gs_stDynamicMemory.iMaxLevelCount ; j++ )
    	{
        	gs_stDynamicMemory.pstBitmapOfLevel[ j ].pbBitmap = pbCurrentBitmapPosition;
        	gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 0;
        	iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;

        	for( i = 0 ; i < iBlockCountOfLevel / 8 ; i++ )
        	{
            		*pbCurrentBitmapPosition = 0x00;
            		pbCurrentBitmapPosition++;
        	}

        	if( ( iBlockCountOfLevel % 8 ) != 0 )
        	{
            		*pbCurrentBitmapPosition = 0x00;
            		i = iBlockCountOfLevel % 8;
            		if( ( i % 2 ) == 1 )
            		{
                		*pbCurrentBitmapPosition |= ( DYNAMICMEMORY_EXIST << ( i - 1 ) );
                		gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 1;
            		}
            		pbCurrentBitmapPosition++;
        	}
    	}        
    
    	gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + 
        	iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
    	gs_stDynamicMemory.qwEndAddress = kCalculateDynamicMemorySize() + 
        	DYNAMICMEMORY_START_ADDRESS;
    	gs_stDynamicMemory.qwUsedSize = 0;
}

static QWORD kCalculateDynamicMemorySize( void )
{
    	QWORD qwRAMSize;
    
    	qwRAMSize = ( kGetTotalRAMSize() * 1024 * 1024 );
    	if( qwRAMSize > ( QWORD ) 3 * 1024 * 1024 * 1024 )
    	{
        	qwRAMSize = ( QWORD ) 3 * 1024 * 1024 * 1024;
    	}   
    
    	return qwRAMSize - DYNAMICMEMORY_START_ADDRESS;
}

static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize )
{
    	long lBlockCountOfSmallestBlock;
    	DWORD dwSizeOfAllocatedBlockListIndex;
    	DWORD dwSizeOfBitmap;
    	long i;
    
    	lBlockCountOfSmallestBlock = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
    	dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof( BYTE );
    
    	dwSizeOfBitmap = 0;
    	for( i = 0 ; ( lBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    	{
        	dwSizeOfBitmap += sizeof( BITMAP );
        	dwSizeOfBitmap += ( ( lBlockCountOfSmallestBlock >> i ) + 7 ) / 8;
    	}
    
    	return ( dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + 
        	DYNAMICMEMORY_MIN_SIZE - 1 ) / DYNAMICMEMORY_MIN_SIZE;
}

void* kAllocateMemory( QWORD qwSize )
{
    	QWORD qwAlignedSize;
    	QWORD qwRelativeAddress;
    	long lOffset;
    	int iSizeArrayOffset;
    	int iIndexOfBlockList;

    	qwAlignedSize = kGetBuddyBlockSize( qwSize );
    	if( qwAlignedSize == 0 )
    	{
        	return NULL;
    	}
    
    	if( gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize +
            	qwAlignedSize > gs_stDynamicMemory.qwEndAddress )
    	{
        	return NULL;
    	}

    	lOffset = kAllocationBuddyBlock( qwAlignedSize );
    	if( lOffset == -1 )
    	{
        return NULL;
    	}
    
    	iIndexOfBlockList = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    
    	qwRelativeAddress = qwAlignedSize * lOffset;
    	iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
    	gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 
        	( BYTE ) iIndexOfBlockList;
    	gs_stDynamicMemory.qwUsedSize += qwAlignedSize;
    
    	return ( void* ) ( qwRelativeAddress + gs_stDynamicMemory.qwStartAddress );
}

static QWORD kGetBuddyBlockSize( QWORD qwSize )
{
    	long i;

    	for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    	{
        	if( qwSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        	{
            		return ( DYNAMICMEMORY_MIN_SIZE << i );
        	}
    	}
    	return 0;
}

static int kAllocationBuddyBlock( QWORD qwAlignedSize )
{
    	int iBlockListIndex, iFreeOffset;
    	int i;
    	BOOL bPreviousInterruptFlag;

    	iBlockListIndex = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    	if( iBlockListIndex == -1 )
    	{
        	return -1;
    	}
    
    	bPreviousInterruptFlag = kLockForSystemData();
    
    	for( i = iBlockListIndex ; i< gs_stDynamicMemory.iMaxLevelCount ; i++ )
    	{
        	iFreeOffset = kFindFreeBlockInBitmap( i );
        	if( iFreeOffset != -1 )
        	{
            	break;
        	}
    	}
    
    	if( iFreeOffset == -1 )
    	{
        	kUnlockForSystemData( bPreviousInterruptFlag );
        	return -1;
    	}

    	kSetFlagInBitmap( i, iFreeOffset, DYNAMICMEMORY_EMPTY );

    	if( i > iBlockListIndex )
    	{
        	for( i = i - 1 ; i >= iBlockListIndex ; i-- )
        	{
            		kSetFlagInBitmap( i, iFreeOffset * 2, DYNAMICMEMORY_EMPTY );
            		kSetFlagInBitmap( i, iFreeOffset * 2 + 1, DYNAMICMEMORY_EXIST ); 
            		iFreeOffset = iFreeOffset * 2;
        	}
    	}    
    	kUnlockForSystemData( bPreviousInterruptFlag );
    
    	return iFreeOffset;
}

static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize )
{
    	int i;

    	for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    	{
        	if( qwAlignedSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        	{
            		return i;
        	}
    	}
    	return -1;
}

static int kFindFreeBlockInBitmap( int iBlockListIndex )
{
    	int i, iMaxCount;
    	BYTE* pbBitmap;
    	QWORD* pqwBitmap;

    	if( gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount == 0 )
    	{
        	return -1;
    	}
    
    	iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
    	pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    	for( i = 0 ; i < iMaxCount ; )
    	{
        	if( ( ( iMaxCount - i ) / 64 ) > 0 )
        	{
            		pqwBitmap = ( QWORD* ) &( pbBitmap[ i / 8 ] );
            		if( *pqwBitmap == 0 )
            		{
                		i += 64;
                		continue;
            		}
        	}                
        
        if( ( pbBitmap[ i / 8 ] & ( DYNAMICMEMORY_EXIST << ( i % 8 ) ) ) != 0 )
        {
            return i;
        }
        i++;
    }
    return -1;
}

static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag )
{
    BYTE* pbBitmap;

    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( bFlag == DYNAMICMEMORY_EXIST )
    {
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) == 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount++;
        }
        pbBitmap[ iOffset / 8 ] |= ( 0x01 << ( iOffset % 8 ) );
    }
    else 
    {
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount--;
        }
        pbBitmap[ iOffset / 8 ] &= ~( 0x01 << ( iOffset % 8 ) );
    }
}

BOOL kFreeMemory( void* pvAddress )
{
    QWORD qwRelativeAddress;
    int iSizeArrayOffset;
    QWORD qwBlockSize;
    int iBlockListIndex;
    int iBitmapOffset;

    if( pvAddress == NULL )
    {
        return FALSE;
    }

    qwRelativeAddress = ( ( QWORD ) pvAddress ) - gs_stDynamicMemory.qwStartAddress;
    iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

    if( gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] == 0xFF )
    {
        return FALSE;
    }

    iBlockListIndex = ( int ) gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ];
    gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 0xFF;
    // 버디 블록의 최소 크기를 블록 리스트 인덱스로 시프트하여 할당된 블록의 크기 계산
    qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

    // 블록 리스트 내의 블록 오프셋을 구해서 블록 해제
    iBitmapOffset = qwRelativeAddress / qwBlockSize;
    if( kFreeBuddyBlock( iBlockListIndex, iBitmapOffset ) == TRUE )
    {
        gs_stDynamicMemory.qwUsedSize -= qwBlockSize;
        return TRUE;
    }
    
    return FALSE;
}

static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset )
{
    int iBuddyBlockOffset;
    int i;
    BOOL bFlag;
    BOOL bPreviousInterruptFlag;

    // 동기화 처리
    bPreviousInterruptFlag = kLockForSystemData();
    
    // 블록 리스트의 끝까지 인접한 블록을 검사하여 결합할 수 없을 때까지 반복
    for( i = iBlockListIndex ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        // 현재 블록은 존재하는 상태로 설정
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EXIST );
        
        // 블록의 오프셋이 짝수(왼쪽)이면 홀수(오른쪽)을 검사하고, 홀수이면 짝수의
        // 비트맵을 검사하여 인접한 블록이 존재한다면 결합
        if( ( iBlockOffset % 2 ) == 0 )
        {
            iBuddyBlockOffset = iBlockOffset + 1;
        }
        else
        {
            iBuddyBlockOffset = iBlockOffset - 1;
        }
        bFlag = kGetFlagInBitmap( i, iBuddyBlockOffset );

        // 블록이 비어있으면 종료
        if( bFlag == DYNAMICMEMORY_EMPTY )
        {
            break;
        }
        
        // 여기까지 왔다면 인접한 블록이 존재하므로, 블록을 결합
        // 블록을 모두 빈 것으로 만들고 상위 블록으로 이동
        kSetFlagInBitmap( i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY );
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EMPTY );
        
        // 상위 블록 리스트의 블록 오프셋으로 변경하고, 위의 과정을 상위 블록에서
        // 다시 반복
        iBlockOffset = iBlockOffset/ 2;
    }
    
    kUnlockForSystemData( bPreviousInterruptFlag );
    return TRUE;
}

/**
 *  블록 리스트의 해당 위치에 비트맵을 반환
*/
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset )
{
    BYTE* pbBitmap;
    
    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0x00 )
    {
        return DYNAMICMEMORY_EXIST;
    }
    
    return DYNAMICMEMORY_EMPTY;
}

/**
 *  동적 메모리 영역에 대한 정보를 반환
 */
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize )
{
    *pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
    *pqwDynamicMemoryTotalSize = kCalculateDynamicMemorySize();    
    *pqwMetaDataSize = kCalculateMetaBlockCount( *pqwDynamicMemoryTotalSize ) * 
        DYNAMICMEMORY_MIN_SIZE;
    *pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

/**
 *  동적 메모리 영역을 관리하는 자료구조를 반환
 */
DYNAMICMEMORY* kGetDynamicMemoryManager( void )
{
    return &gs_stDynamicMemory;
}

void kInitializeFreeList(void)
{
	FREELIST* startHead,* pstHead;
	long stHeadAddr;
	int size;

	startHead =(FREELIST*)kAllocateMemory(20000000);

	stHeadAddr = (long)startHead + sizeof(FREELIST);
	pstHead = (FREELIST*)stHeadAddr;

	size = gs_stDynamicMemory.qwUsedSize;
	pstHead -> size = size;
	pstHead -> freeNext = NULL;
		
	startHead -> size = 0;
	startHead -> freeNext = pstHead;	
		
	kPrintf( "mallocAdd =[%Q]\n", sizeof(MALLOCHEAD) );
	gs_sMallocManager.stFreeList = startHead;
	gs_sMallocManager.lastMagic = 0;
	gs_sMallocManager.useNum = 0;
	gs_sMallocManager.lefSize = pstHead->size;

}

void* kMalloc(QWORD size)
{
	MALLOCHEAD* mallocHead;
	FREELIST* preFreeList, * nextFreeList, * bestFreeList, *bestPreFreeList;
	QWORD realSize = size + sizeof(MALLOCHEAD);
    	BOOL bPreviousInterruptFlag;

	preFreeList = gs_sMallocManager.stFreeList; 
	bestFreeList = NULL;
    	bPreviousInterruptFlag = kLockForSystemData();
	while( 1 )
	{	
		nextFreeList = preFreeList->freeNext;	
		if( ( nextFreeList->size ) > realSize )
		{
			if( bestFreeList == NULL || ( bestFreeList->size ) > ( nextFreeList->size ) )
			{
				bestFreeList = nextFreeList;
				bestPreFreeList = preFreeList;	
			}
		
		}
		if(nextFreeList->freeNext == NULL)
		{
			break;
		}
		preFreeList = nextFreeList;
	}
	
	if(bestFreeList == NULL)
	{	
		return;	
	}			
	if(bestFreeList -> freeNext != NULL)
	{	
		if( ( bestFreeList->size ) - realSize >= sizeof(FREELIST))
		{
			nextFreeList = setFreeList( ( bestFreeList->size ) - realSize, (long)bestFreeList + realSize);
			bestPreFreeList -> freeNext = nextFreeList;
			nextFreeList -> freeNext = bestFreeList -> freeNext;			
		}
		else
		{
			bestPreFreeList -> freeNext = bestFreeList -> freeNext; 
		}	
	}
	else
	{
		if( ( bestFreeList->size ) - realSize >= sizeof(FREELIST))
		{
			nextFreeList = setFreeList( ( bestFreeList->size ) - realSize, (long)bestFreeList + realSize);
			bestPreFreeList -> freeNext = nextFreeList;
			nextFreeList -> freeNext = NULL;
		}
		else
		{
			bestPreFreeList -> freeNext = NULL; 
		}	
	}	
	mallocHead = bestFreeList;
	mallocHead->size = size;
	gs_sMallocManager.lastMagic++;
	mallocHead->magic = gs_sMallocManager.lastMagic;
	gs_sMallocManager.useNum++;
	gs_sMallocManager.lefSize -= realSize;
	kUnlockForSystemData( bPreviousInterruptFlag ); 
	kPrintf( "mallocAdd =[0x%Q]\n", mallocHead );
	kPrintf( "mallocsize =[%d]\n", mallocHead->size );	
	return (long)mallocHead + sizeof(MALLOCHEAD);		
}

void kFree(void *ptr)
{
	FREELIST* pstFreeList, * stFreeList;
	MALLOCHEAD* hptr;
	long startAddr;
	long size;
    	BOOL bPreviousInterruptFlag;
	if( ptr == NULL )
    	{
        	return FALSE;
    	}

	startAddr =(long) ptr - sizeof(MALLOCHEAD);
	pstFreeList = startAddr;
	
	kPrintf( "freesize =[%d]\n", pstFreeList->size );	
    	
	bPreviousInterruptFlag = kLockForSystemData();
	stFreeList = gs_sMallocManager.stFreeList;
	
	pstFreeList->freeNext = stFreeList->freeNext;
	stFreeList->freeNext = pstFreeList;
	gs_sMallocManager.useNum--;
	gs_sMallocManager.lefSize += size;
	kUnlockForSystemData( bPreviousInterruptFlag );  
}

FREELIST* setFreeList(int size, long addr)
{
	FREELIST* newFreeList = (FREELIST*)addr;
	newFreeList->size = size;
	return newFreeList;	
}

void getFreeListInfo(int* useNum, int* lefSize)
{
	*useNum = gs_sMallocManager.useNum;
	*lefSize = gs_sMallocManager.lefSize;
}

int getCountFreeList(void)
{
	FREELIST* pstFreeList;
	int iCount;
	
	pstFreeList = gs_sMallocManager.stFreeList;
	while(pstFreeList != NULL)
	{
		iCount++;
		pstFreeList = pstFreeList -> freeNext;
	}
	return iCount;
} 
