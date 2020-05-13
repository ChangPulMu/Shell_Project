
#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"
#include "Task.h"
#include "Utility.h"


static BYTE gs_vbTempBuffer[ FILESYSTEM_SECTORSPERCLUSTER * 512 ];


fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;

BOOL kInitializeFileSystem( void )
{
    	kMemSet( &gs_stFileSystemManager, 0, sizeof( gs_stFileSystemManager ) );
    	kInitializeMutex( &( gs_stFileSystemManager.stMutex ) );
    
    	if( kInitializeHDD() == TRUE )
    	{
        	gs_pfReadHDDInformation = kReadHDDInformation;
        	gs_pfReadHDDSector = kReadHDDSector;
        	gs_pfWriteHDDSector = kWriteHDDSector;
    	}
    	else
    	{
        	return FALSE;
    	}
    
    	if( kMount() == FALSE )
    	{
        	return FALSE;
    	}
    
    	return TRUE;
}

BOOL kMount( void )
{
    	MBR* pstMBR;
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return FALSE;
    	}
    
    	pstMBR = ( MBR* ) gs_vbTempBuffer;
    	if( pstMBR->dwSignature != FILESYSTEM_SIGNATURE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return FALSE;
    	}
    
    	gs_stFileSystemManager.bMounted = TRUE;
    
    	gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    	gs_stFileSystemManager.dwClusterLinkAreaStartAddress =
        	pstMBR->dwReservedSectorCount + 1;
    	gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    	gs_stFileSystemManager.dwDataAreaStartAddress = 
        	pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    	gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;
	gs_stFileSystemManager.pstDirIndex = 0;
	kMemCpy(gs_stFileSystemManager.dirRootName, "/" , 1);

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return TRUE;
}

BOOL kFormat( void )
{
    	HDDINFORMATION* pstHDD;
    	MBR* pstMBR;
    	DWORD dwTotalSectorCount, dwRemainSectorCount;
    	DWORD dwMaxClusterCount, dwClsuterCount;
    	DWORD dwClusterLinkSectorCount;
    	DWORD i;
	DIRECTORYENTRY dot, dotDot;
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	pstHDD = ( HDDINFORMATION* ) gs_vbTempBuffer;
    	if( gs_pfReadHDDInformation( TRUE, TRUE, pstHDD ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return FALSE;
    	}    
   	dwTotalSectorCount = pstHDD->dwTotalSectors;
    
    	dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    	dwClusterLinkSectorCount = ( dwMaxClusterCount + 127 ) / 128;
    
    	dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    	dwClsuterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;
    
    	dwClusterLinkSectorCount = ( dwClsuterCount + 127 ) / 128;

    	if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return FALSE;
    	}        
    
    	pstMBR = ( MBR* ) gs_vbTempBuffer;
    	kMemSet( pstMBR->vstPartition, 0, sizeof( pstMBR->vstPartition ) );
    	pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    	pstMBR->dwReservedSectorCount = 0;
    	pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    	pstMBR->dwTotalClusterCount = dwClsuterCount;
    
    	if( gs_pfWriteHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return FALSE;
    	}
    
    	kMemSet( gs_vbTempBuffer, 0, 512 );
    	for( i = 0 ; i < ( dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER );
         	i++ )
    	{
        	if( i == 0 )
        	{
            		( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_LASTCLUSTER;
        	}
        	else
       		{
            		( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_FREECLUSTER;
        	}
        
        	if( gs_pfWriteHDDSector( TRUE, TRUE, i + 1, 1, gs_vbTempBuffer ) == FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return FALSE;
        	}
    	}   

	kMemCpy(dot.vcFileName, "/" , 1);
	dot.bType = FILESYSTEM_TYPE_DIRECTORY;
	dot.dwStartClusterIndex = 0;
	dot.dwFileSize = 0;
	dot.dirClusterIndex = 0;	 
	kSetDirectoryEntryData(0,0,&dot);

	kMemCpy(dotDot.vcFileName, "/" , 1);
	dotDot.bType = FILESYSTEM_TYPE_DIRECTORY;
	dotDot.dwStartClusterIndex = 0;	
 	dotDot.dwFileSize = 0;
	dotDot.dirClusterIndex = 0;	 
	kSetDirectoryEntryData(0,1,&dotDot);

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return TRUE;
}

BOOL kGetHDDInformation( HDDINFORMATION* pstInformation)
{
    	BOOL bResult;
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
    	bResult = gs_pfReadHDDInformation( TRUE, TRUE, pstInformation );
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    
    	return bResult;
}

BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    	return gs_pfReadHDDSector( TRUE, TRUE, dwOffset + 
              gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    	return gs_pfWriteHDDSector( TRUE, TRUE, dwOffset + 
              gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    	return gs_pfReadHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    	return gs_pfWriteHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + 
              gs_stFileSystemManager.dwDataAreaStartAddress, 
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

DWORD kFindFreeCluster( void )
{
    	DWORD dwLinkCountInSector;
    	DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    	DWORD i, j;
    
    	if( gs_stFileSystemManager.bMounted == FALSE )
    	{
        	return FILESYSTEM_LASTCLUSTER;
    	}	
    
    	dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    	for( i = 0 ; i < gs_stFileSystemManager.dwClusterLinkAreaSize ; i++ )
    	{
        	if( ( dwLastSectorOffset + i ) == 
            		( gs_stFileSystemManager.dwClusterLinkAreaSize - 1 ) )
        	{
            		dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128; 
        	}
        	else
        	{
            		dwLinkCountInSector = 128;
        	}
        
        	dwCurrentSectorOffset = ( dwLastSectorOffset + i ) % 
            		gs_stFileSystemManager.dwClusterLinkAreaSize;
        	if( kReadClusterLinkTable( dwCurrentSectorOffset, gs_vbTempBuffer ) == FALSE )
        	{
            		return FILESYSTEM_LASTCLUSTER;
        	}
        
        	for( j = 0 ; j < dwLinkCountInSector ; j++ )
        	{
            		if( ( ( DWORD* ) gs_vbTempBuffer )[ j ] == FILESYSTEM_FREECLUSTER )
            		{
                		break;
            		}
        	}
            
        	if( j != dwLinkCountInSector )
        	{
            		gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = 
                	dwCurrentSectorOffset;
            
            		return ( dwCurrentSectorOffset * 128 ) + j;
        	}
    	}
    
    	return FILESYSTEM_LASTCLUSTER;
}

BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData )
{
    	DWORD dwSectorOffset;
    
    	if( gs_stFileSystemManager.bMounted == FALSE )
    	{
        	return FALSE;
    	}
    
    	dwSectorOffset = dwClusterIndex / 128;

    	if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    	{
        	return FALSE;
    	}    
    
    	( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ] = dwData;

    	if( kWriteClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    	{
        	return FALSE;
    	}

    	return TRUE;
}

BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData )
{
    	DWORD dwSectorOffset;
    
    	if( gs_stFileSystemManager.bMounted == FALSE )
    	{
        	return FALSE;
    	}
    
    	dwSectorOffset = dwClusterIndex / 128;
    
    	if( dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize )
    	{
        	return FALSE;
    	}
    
    
    	if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
   	{
        	return FALSE;
    	}    

    	*pdwData = ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ];
    	return TRUE;
}


int kFindFreeDirectoryEntry( void )
{
    	DIRECTORYENTRY* pstEntry;
    	int i;

    	if( gs_stFileSystemManager.bMounted == FALSE )
    	{
        	return -1;
    	}

    	if( kReadCluster( gs_stFileSystemManager.pstDirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return -1;
    	}
    
    	pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    	for( i = 2 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    	{
        	if( pstEntry[ i ].dwStartClusterIndex == 0 )
        	{
            		return i;
        	}
    	}
    	return -1;
}

 BOOL kSetDirectoryEntryData( int dirIndex ,int iIndex, DIRECTORYENTRY* pstEntry )
{
   	DIRECTORYENTRY* pstDirEntry;
    
    	if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        	( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    	{
        	return FALSE;
    	}

    	if( kReadCluster( dirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return FALSE;
    	}    
    
    	pstDirEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    	kMemCpy( pstDirEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    	if( kWriteCluster( dirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return FALSE;
    	}    
    	return TRUE;
}

 BOOL kGetDirectoryEntryData( int dirIndex, int iIndex, DIRECTORYENTRY* pstEntry )
{
    	DIRECTORYENTRY* pstDirEntry;
    
    	if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        	( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    	{
        	return FALSE;
    	}

    	if( kReadCluster( dirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return FALSE;
    	}    
    
    	pstDirEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
   	kMemCpy( pstEntry, pstDirEntry + iIndex, sizeof( DIRECTORYENTRY ) );
    	return TRUE;
}

int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry )
{
   	DIRECTORYENTRY* pstRootEntry;
    	int i;
    	int iLength;

    	if( gs_stFileSystemManager.bMounted == FALSE )
   	{
        	return -1;
    	}

    	if( kReadCluster( gs_stFileSystemManager.pstDirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return -1;
    	}
    
    	iLength = kStrLen( pcFileName );
    	pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    	for( i = 2 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    	{
        	if( kMemCmp( pstRootEntry[ i ].vcFileName, pcFileName, iLength ) == 0 )
       		{
            		kMemCpy( pstEntry, pstRootEntry + i, sizeof( DIRECTORYENTRY ) );
            		return i;
        	}
    	}
    	return -1;
}

// 디렉토리 인덱스로 찾는 함수
int kFindDirectoryEntryByIndex(int  dirIndex, DIRECTORYENTRY* pstEntry)
{
	DIRECTORYENTRY* pstRootEntry;
	int i;
	int iLength;

	if (gs_stFileSystemManager.bMounted == FALSE)
	{
		return -1;
	}

	if (kReadCluster(gs_stFileSystemManager.pstDirIndex, gs_vbTempBuffer) == FALSE)
	{
		return -1;
	}

	pstRootEntry = (DIRECTORYENTRY*)gs_vbTempBuffer;
	for (i = 2; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++)
	{
		if (pstRootEntry[i].dwStartClusterIndex == dirIndex)
		{
			kMemCpy(pstEntry, pstRootEntry + i, sizeof(DIRECTORYENTRY));
			return i;
		}
	}
	return -1;
}


void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager )
{
    	kMemCpy( pstManager, &gs_stFileSystemManager, sizeof( gs_stFileSystemManager ) );
}



 void* kAllocateFileDirectoryHandle( int bType )
{
    	int i;
    	FILE* pstFile;
    
    	pstFile = gs_stFileSystemManager.pstHandlePool;
    	for( i = 0 ; i < FILESYSTEM_HANDLE_MAXCOUNT ; i++ )
    	{
        	if( pstFile->bType == FILESYSTEM_TYPE_FREE )
        	{
			if( bType == FILESYSTEM_TYPE_FREE )
			{            		
				pstFile->bType = FILESYSTEM_TYPE_FILE;
	            		return pstFile;
			}
			else
			{
				pstFile->bType = FILESYSTEM_TYPE_DIRECTORY;
				return pstFile;				
			}        	
		}
        
        	pstFile++;
    	}
    
    	return NULL;
}

static void kFreeFileDirectoryHandle( FILE* pstFile )
{

    	kMemSet( pstFile, 0, sizeof( FILE ) );
    
    	pstFile->bType = FILESYSTEM_TYPE_FREE;
}

static BOOL kCreateFile( const char* pcFileName, DIRECTORYENTRY* pstEntry, 
        int* piDirectoryEntryIndex )
{
    	DWORD dwCluster;
    
    	dwCluster = kFindFreeCluster();
    	if( ( dwCluster == FILESYSTEM_LASTCLUSTER ) ||
        	( kSetClusterLinkData( dwCluster, FILESYSTEM_LASTCLUSTER ) == FALSE ) )
    	{
        	return FALSE;
    	}

    	*piDirectoryEntryIndex = kFindFreeDirectoryEntry();
    	if( *piDirectoryEntryIndex == -1 )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}
    
    	kMemCpy( pstEntry->vcFileName, pcFileName, kStrLen( pcFileName ) + 1 );
    	pstEntry->dwStartClusterIndex = dwCluster;
    	pstEntry->dwFileSize = 0;
	pstEntry->bType = FILESYSTEM_TYPE_FILE;
	pstEntry->dirClusterIndex = gs_stFileSystemManager.pstDirIndex;
    
    	if( kSetDirectoryEntryData( gs_stFileSystemManager.pstDirIndex, *piDirectoryEntryIndex, pstEntry ) == FALSE )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}
    	return TRUE;
}

BOOL kCreateDir( const char* pcDirName )
{
    	DWORD dwCluster;
    	DIRECTORYENTRY dot, dotDot;
	int piDirectoryEntryIndex;
    	dwCluster = kFindFreeCluster();
	char* sDirectoryName;
    	if( ( dwCluster == FILESYSTEM_LASTCLUSTER ) ||
        	( kSetClusterLinkData( dwCluster, FILESYSTEM_LASTCLUSTER ) == FALSE ) )
    	{
        	return FALSE;
    	}
	kLock( &( gs_stFileSystemManager.stMutex ) );

    	piDirectoryEntryIndex = kFindFreeDirectoryEntry();
    	if( piDirectoryEntryIndex == -1 )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}
    
    	kMemCpy( dot.vcFileName, pcDirName, kStrLen( pcDirName ) + 1 );
    	dot.dwStartClusterIndex = dwCluster;
    	dot.dwFileSize = 0;
	dot.bType = FILESYSTEM_TYPE_DIRECTORY;
	dot.dirClusterIndex = gs_stFileSystemManager.pstDirIndex;
    
    	if( kSetDirectoryEntryData( gs_stFileSystemManager.pstDirIndex, piDirectoryEntryIndex, &dot ) == FALSE )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}


	kMemCpy( dotDot.vcFileName, gs_stFileSystemManager.dirRootName, sizeof(gs_stFileSystemManager.dirRootName) + 1 );
	dotDot.dwStartClusterIndex = gs_stFileSystemManager.pstDirIndex;
	dotDot.dirClusterIndex = 0;
	dotDot.dwFileSize = 0;
	dotDot.bType = FILESYSTEM_TYPE_DIRECTORY;
	
	if( kSetDirectoryEntryData( dwCluster, 1, &dotDot ) == FALSE )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}

	kSPrintf( sDirectoryName, "%s%s/", gs_stFileSystemManager.dirRootName, pcDirName );
	kMemCpy( dot.vcFileName, sDirectoryName, kStrLen(sDirectoryName) + 1 );

	if( kSetDirectoryEntryData( dwCluster, 0, &dot ) == FALSE )
    	{
        	kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        	return FALSE;
    	}
	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return TRUE;
}


static BOOL kFreeClusterUntilEnd( DWORD dwClusterIndex )
{
    	DWORD dwCurrentClusterIndex;
    	DWORD dwNextClusterIndex;
    
    	dwCurrentClusterIndex = dwClusterIndex;
    
    	while( dwCurrentClusterIndex != FILESYSTEM_LASTCLUSTER )
   	{
        	if( kGetClusterLinkData( dwCurrentClusterIndex, &dwNextClusterIndex )
                	== FALSE )
        	{
            		return FALSE;
        	}
        
        	if( kSetClusterLinkData( dwCurrentClusterIndex, FILESYSTEM_FREECLUSTER )
                	== FALSE )
        	{
            		return FALSE;
        	}
        
        	dwCurrentClusterIndex = dwNextClusterIndex;
    	}
    
    	return TRUE;
}

FILE* kOpenFile( const char* pcFileName, const char* pcMode )
{
    	DIRECTORYENTRY stEntry ;
    	int iDirectoryEntryOffset;
    	int iFileNameLength;
	DWORD preFileSize;
    	DWORD dwSecondCluster;
    	FILE* pstFile;

    	iFileNameLength = kStrLen( pcFileName );
    	if( ( iFileNameLength > ( sizeof( stEntry.vcFileName ) - 1 ) ) || 
        	( iFileNameLength == 0 ) )
    	{
        	return NULL;
    	}
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	iDirectoryEntryOffset = kFindDirectoryEntry( pcFileName, &stEntry );
   	if( iDirectoryEntryOffset == -1 )
    	{
        	if( pcMode[ 0 ] == 'r' )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;
        	}
        
        	if( kCreateFile( pcFileName, &stEntry, &iDirectoryEntryOffset ) == FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
           	 	return NULL;
        	}
    	}    
    	else if( pcMode[ 0 ] == 'w' )
    	{
		if( stEntry.bType == FILESYSTEM_TYPE_DIRECTORY )
		{
			kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;		
		}		        	
		
		if( kGetClusterLinkData( stEntry.dwStartClusterIndex, &dwSecondCluster )
                	== FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;
        	}
        
        	if( kSetClusterLinkData( stEntry.dwStartClusterIndex, 
               		FILESYSTEM_LASTCLUSTER ) == FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;
        	}
        
        	if( kFreeClusterUntilEnd( dwSecondCluster ) == FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;
        	}
       
        	stEntry.dwFileSize = 0;
		
        	if( kSetDirectoryEntryData( stEntry.dirClusterIndex, iDirectoryEntryOffset, &stEntry ) == FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return NULL;
        	}
    	}
	
	if( stEntry.bType == FILESYSTEM_TYPE_DIRECTORY )
	{
		kUnlock( &( gs_stFileSystemManager.stMutex ) );            		
		return NULL;		
	}
    
    	pstFile = kAllocateFileDirectoryHandle( FILESYSTEM_TYPE_FILE );
   	if( pstFile == NULL )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return NULL;
    	}
    
    	pstFile->bType = FILESYSTEM_TYPE_FILE;
    	pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
    	pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
    	pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
    	pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
    	pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
    	pstFile->stFileHandle.dwCurrentOffset = 0;
	pstFile->stFileHandle.dirClusterIndex = stEntry.dirClusterIndex;
       
    	if( pcMode[ 0 ] == 'a' )
    	{
        	kSeekFile( pstFile, 0, FILESYSTEM_SEEK_END );
    	}


    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return pstFile;
}

DWORD kReadFile( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    	DWORD dwTotalCount;
    	DWORD dwReadCount;
    	DWORD dwOffsetInCluster;
    	DWORD dwCopySize;
   	FILEHANDLE* pstFileHandle;
    	DWORD dwNextClusterIndex;    

    	if( ( pstFile == NULL ) ||
        	( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    	{	
        	return 0;
    	}
    	pstFileHandle = &( pstFile->stFileHandle );
    
    	if( ( pstFileHandle->dwCurrentOffset == pstFileHandle->dwFileSize ) ||
        	( pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER ) )
    	{
        	return 0;
    	}

    	dwTotalCount = MIN( dwSize * dwCount, pstFileHandle->dwFileSize - 
                        	pstFileHandle->dwCurrentOffset );
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
    	dwReadCount = 0;
    	while( dwReadCount != dwTotalCount )
    	{
        	if( kReadCluster( pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer )
                	== FALSE )
        	{
            		break;
        	}

        	dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;
        
        	dwCopySize = MIN( FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, 
                          dwTotalCount - dwReadCount );
        	kMemCpy( ( char* ) pvBuffer + dwReadCount, 
                	gs_vbTempBuffer + dwOffsetInCluster, dwCopySize );

        	dwReadCount += dwCopySize;
        	pstFileHandle->dwCurrentOffset += dwCopySize;

        	if( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) == 0 )
        	{
            		if( kGetClusterLinkData( pstFileHandle->dwCurrentClusterIndex, 
                                     &dwNextClusterIndex ) == FALSE )
            		{
               			break;
            		}
            
            		pstFileHandle->dwPreviousClusterIndex = 
                		pstFileHandle->dwCurrentClusterIndex;
            		pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        	}
    	}
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    
    	return ( dwReadCount / dwSize );
}

 BOOL kUpdateDirectoryEntry( FILEHANDLE* pstFileHandle )
{
    	DIRECTORYENTRY stEntry;
    
    	if( ( pstFileHandle == NULL ) ||
        	( kGetDirectoryEntryData( pstFileHandle->dirClusterIndex, pstFileHandle, &stEntry)
            	== FALSE ) )
    	{
        	return FALSE;
    	}
    
    	stEntry.dwFileSize = pstFileHandle->dwFileSize;
    	stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;


    	if( kSetDirectoryEntryData( stEntry.dirClusterIndex ,pstFileHandle->iDirectoryEntryOffset, &stEntry )
            	== FALSE )
    	{
        	return FALSE;
    	}

	
    	return TRUE;
}

DWORD kWriteFile( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    	DWORD dwWriteCount;
    	DWORD dwTotalCount;
    	DWORD dwOffsetInCluster;
   	DWORD dwCopySize;
   	DWORD dwAllocatedClusterIndex;
    	DWORD dwNextClusterIndex;
    	FILEHANDLE* pstFileHandle;

    if( ( pstFile == NULL ) ||
        	( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    	{
        	return 0;
    	}
    	pstFileHandle = &( pstFile->stFileHandle );

    	dwTotalCount = dwSize * dwCount;
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	dwWriteCount = 0;
    	while( dwWriteCount != dwTotalCount )
    	{	
        	if( pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER )
        	{
            		dwAllocatedClusterIndex = kFindFreeCluster();
            		if( dwAllocatedClusterIndex == FILESYSTEM_LASTCLUSTER )
            		{
                		break;
            		}
            
            		if( kSetClusterLinkData( dwAllocatedClusterIndex, FILESYSTEM_LASTCLUSTER )
                    		== FALSE )
            		{
                		break;
            		}
            
            		if( kSetClusterLinkData( pstFileHandle->dwPreviousClusterIndex, 
                        	dwAllocatedClusterIndex ) == FALSE )
            		{
                		kSetClusterLinkData( dwAllocatedClusterIndex, FILESYSTEM_FREECLUSTER );
                		break;
            		}
            
            		pstFileHandle->dwCurrentClusterIndex = dwAllocatedClusterIndex;
            
            		kMemSet( gs_vbTempBuffer, 0, FILESYSTEM_LASTCLUSTER );
        	}        
        	else if( ( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) != 0 ) ||
                 		( ( dwTotalCount - dwWriteCount ) < FILESYSTEM_CLUSTERSIZE ) )
        	{
            		if( kReadCluster( pstFileHandle->dwCurrentClusterIndex, 
                              gs_vbTempBuffer ) == FALSE )
            		{
                		break;
            		}
        	}

        	dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;
        
        	dwCopySize = MIN( FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster, 
                          	dwTotalCount - dwWriteCount );
        	kMemCpy( gs_vbTempBuffer + dwOffsetInCluster, ( char* ) pvBuffer + 
                 	dwWriteCount, dwCopySize );
        
        	if( kWriteCluster( pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer ) 
                	== FALSE )
        	{
            		break;
        	}
        
        	dwWriteCount += dwCopySize;
        	pstFileHandle->dwCurrentOffset += dwCopySize;

        	if( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) == 0 )
        	{
            		if( kGetClusterLinkData( pstFileHandle->dwCurrentClusterIndex, 
                                     		&dwNextClusterIndex ) == FALSE )
            		{
                		break;
            		}
            
            		pstFileHandle->dwPreviousClusterIndex = 
                		pstFileHandle->dwCurrentClusterIndex;
            		pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        	}
    	}

    	if( pstFileHandle->dwFileSize < pstFileHandle->dwCurrentOffset )
    	{
        	pstFileHandle->dwFileSize = pstFileHandle->dwCurrentOffset;
        	kUpdateDirectoryEntry( pstFileHandle );
    	}
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    
    	return ( dwWriteCount / dwSize );
}

BOOL kWriteZero( FILE* pstFile, DWORD dwCount )
{
    	BYTE* pbBuffer;
    	DWORD dwRemainCount;
    	DWORD dwWriteCount;
    
    	if( pstFile == NULL )
    	{
        	return FALSE;
    	}
    
    	pbBuffer = ( BYTE* ) kMalloc( FILESYSTEM_CLUSTERSIZE );
    	if( pbBuffer == NULL )
    	{
        	return FALSE;
    	}
    
    	kMemSet( pbBuffer, 0, FILESYSTEM_CLUSTERSIZE );
    	dwRemainCount = dwCount;
    
    	while( dwRemainCount != 0 )
    	{
        	dwWriteCount = MIN( dwRemainCount , FILESYSTEM_CLUSTERSIZE );
        	if( kWriteFile( pbBuffer, 1, dwWriteCount, pstFile ) != dwWriteCount )
        	{
            		kFree( pbBuffer );
            		return FALSE;
        	}
        	dwRemainCount -= dwWriteCount;
    	}
    	kFree( pbBuffer );
    	return TRUE;
}

int kSeekFile( FILE* pstFile, int iOffset, int iOrigin )
{
    	DWORD dwRealOffset;
    	DWORD dwClusterOffsetToMove;
    	DWORD dwCurrentClusterOffset;
    	DWORD dwLastClusterOffset;
   	DWORD dwMoveCount;
   	DWORD i;
    	DWORD dwStartClusterIndex;
    	DWORD dwPreviousClusterIndex;
    	DWORD dwCurrentClusterIndex;
    	FILEHANDLE* pstFileHandle;

    	if( ( pstFile == NULL ) ||
        	( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    	{
        	return 0;
    	}
    	pstFileHandle = &( pstFile->stFileHandle );
    
    	switch( iOrigin )
    	{
    		case FILESYSTEM_SEEK_SET:
        		if( iOffset <= 0 )
        		{
            			dwRealOffset = 0;
        		}
        		else
        		{
            			dwRealOffset = iOffset;
        		}
        		break;

    		case FILESYSTEM_SEEK_CUR:
        		if( ( iOffset < 0 ) && 
            			( pstFileHandle->dwCurrentOffset <= ( DWORD ) -iOffset ) )
        		{
            			dwRealOffset = 0;
        		}
        		else
        		{
            			dwRealOffset = pstFileHandle->dwCurrentOffset + iOffset;
        		}
        		break;

	    	case FILESYSTEM_SEEK_END:
        		if( ( iOffset < 0 ) && 
            			( pstFileHandle->dwFileSize <= ( DWORD ) -iOffset ) )
        		{
            			dwRealOffset = 0;
        		}
        		else
        		{
            			dwRealOffset = pstFileHandle->dwFileSize + iOffset;
        		}
        		break;
    	}

	dwLastClusterOffset = pstFileHandle->dwFileSize / FILESYSTEM_CLUSTERSIZE;
    	dwClusterOffsetToMove = dwRealOffset / FILESYSTEM_CLUSTERSIZE;
    	dwCurrentClusterOffset = pstFileHandle->dwCurrentOffset / FILESYSTEM_CLUSTERSIZE;

    	if( dwLastClusterOffset < dwClusterOffsetToMove )
    	{
        	dwMoveCount = dwLastClusterOffset - dwCurrentClusterOffset;
        	dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    	}
    	else if( dwCurrentClusterOffset <= dwClusterOffsetToMove )
    	{
        	dwMoveCount = dwClusterOffsetToMove - dwCurrentClusterOffset;
        	dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    	}
    	else
    	{
        	dwMoveCount = dwClusterOffsetToMove;
        	dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;
    	}

    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	dwCurrentClusterIndex = dwStartClusterIndex;
    	for( i = 0 ; i < dwMoveCount ; i++ )
    	{
        	dwPreviousClusterIndex = dwCurrentClusterIndex;
        
        	if( kGetClusterLinkData( dwPreviousClusterIndex, &dwCurrentClusterIndex ) ==
            		FALSE )
        	{
            		kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return -1;
        	}
    	}

    	if( dwMoveCount > 0 )
    	{
        	pstFileHandle->dwPreviousClusterIndex = dwPreviousClusterIndex;
        	pstFileHandle->dwCurrentClusterIndex = dwCurrentClusterIndex;
    	}
    	else if( dwStartClusterIndex == pstFileHandle->dwStartClusterIndex )
    	{
        	pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwStartClusterIndex;
        	pstFileHandle->dwCurrentClusterIndex = pstFileHandle->dwStartClusterIndex;
    	}	
    
    	if( dwLastClusterOffset < dwClusterOffsetToMove )
    	{
        	pstFileHandle->dwCurrentOffset = pstFileHandle->dwFileSize;
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );

        	if( kWriteZero( pstFile, dwRealOffset - pstFileHandle->dwFileSize )
                	== FALSE )
        	{
            		return 0;
        	}
    	}

    	pstFileHandle->dwCurrentOffset = dwRealOffset;

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );

    	return 0;    
}

int kCloseFile( FILE* pstFile )
{
   	if( ( pstFile == NULL ) ||
        	( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    	{
        	return -1;
    	}
    
    	kFreeFileDirectoryHandle( pstFile );
    	return 0;
}

BOOL kIsFileOpened( const DIRECTORYENTRY* pstEntry )
{
    	int i;
    	FILE* pstFile;
    
    	pstFile = gs_stFileSystemManager.pstHandlePool;
    	for( i = 0 ; i < FILESYSTEM_HANDLE_MAXCOUNT ; i++ )
    	{
        	if( ( pstFile[ i ].bType == FILESYSTEM_TYPE_FILE ) &&
            		( pstFile[ i ].stFileHandle.dwStartClusterIndex == 
              			pstEntry->dwStartClusterIndex ) )
        	{
            		return TRUE;
        	}
    	}
    	return FALSE;
}

int kRemoveFile( const char* pcFileName )
{
    	DIRECTORYENTRY stEntry;
	int pstDirIndex;
    	int iDirectoryEntryOffset;
    	int iFileNameLength;

    	iFileNameLength = kStrLen( pcFileName );
    	if( ( iFileNameLength > ( sizeof( stEntry.vcFileName ) - 1 ) ) || 
        	( iFileNameLength == 0 ) )
    	{
        	return NULL;
    	}
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
    	iDirectoryEntryOffset = kFindDirectoryEntry( pcFileName, &stEntry );
    	if( iDirectoryEntryOffset == -1 ) 
    	{ 
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    	pstDirIndex = stEntry.dirClusterIndex;
    	if( kIsFileOpened( &stEntry ) == TRUE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    
    	if( kFreeClusterUntilEnd( stEntry.dwStartClusterIndex ) == FALSE )
    	{ 
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}

    	kMemSet( &stEntry, 0, sizeof( stEntry ) );
    	if( kSetDirectoryEntryData( pstDirIndex, iDirectoryEntryOffset, &stEntry ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return 0;
}

int kRenameFile(const char* pcFileName, const char* pcNewFileName)
{
	DIRECTORYENTRY stEntry;
	int pstDirIndex;
	int iDirectoryEntryOffset;
	int iFileNameLength, iNewFileNameLength;
	iNewFileNameLength = kStrLen(pcNewFileName);
	iFileNameLength = kStrLen(pcFileName);
	if ((iFileNameLength > (sizeof(stEntry.vcFileName) - 1)) ||
		(iFileNameLength == 0))
	{
		return NULL;
	}

	kLock(&(gs_stFileSystemManager.stMutex));

	iDirectoryEntryOffset = kFindDirectoryEntry(pcFileName, &stEntry);

	if (iDirectoryEntryOffset == -1)
	{
		kUnlock(&(gs_stFileSystemManager.stMutex));
		return -1;
	}
	pstDirIndex = stEntry.dirClusterIndex;
	if (kIsFileOpened(&stEntry) == TRUE)
	{
		kUnlock(&(gs_stFileSystemManager.stMutex));
		return -1;
	}

	if (kFreeClusterUntilEnd(stEntry.dwStartClusterIndex) == FALSE)
	{
		kUnlock(&(gs_stFileSystemManager.stMutex));
		return -1;
	}

	kMemCpy(&stEntry.vcFileName, pcNewFileName, iNewFileNameLength + 1);
	
	if (kSetDirectoryEntryData(pstDirIndex, iDirectoryEntryOffset, &stEntry) == FALSE)
	{
		kUnlock(&(gs_stFileSystemManager.stMutex));
		return -1;
	}

	kUnlock(&(gs_stFileSystemManager.stMutex));
	return 0;
}

DIR* kOpenDirectory( void )
{
    	DIR* pstDirectory;
    	DIRECTORYENTRY* pstDirectoryBuffer;

    	kLock( &( gs_stFileSystemManager.stMutex ) );

    	pstDirectory = kAllocateFileDirectoryHandle(FILESYSTEM_TYPE_DIRECTORY);
    	if( pstDirectory == NULL )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return NULL;
    	}
    
    	pstDirectoryBuffer = ( DIRECTORYENTRY* ) kMalloc( FILESYSTEM_CLUSTERSIZE );
    	if( pstDirectory == NULL )
    	{
        	kFreeFileDirectoryHandle( pstDirectory );
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return NULL;
    	}
    
    	if( kReadCluster( gs_stFileSystemManager.pstDirIndex , ( BYTE* ) pstDirectoryBuffer ) == FALSE )
    	{
        	kFreeFileDirectoryHandle( pstDirectory );
        	kFree( pstDirectoryBuffer );
        
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return NULL;
        
    	}
    
    	pstDirectory->bType = FILESYSTEM_TYPE_DIRECTORY;
    	pstDirectory->stDirectoryHandle.iCurrentOffset = 0;
    	pstDirectory->stDirectoryHandle.pstDirectoryBuffer = pstDirectoryBuffer;
	

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return pstDirectory;
}

struct kDirectoryEntryStruct* kReadDirectory( DIR* pstDirectory, int* offset )
{
    	DIRECTORYHANDLE* pstDirectoryHandle;
    	DIRECTORYENTRY* pstEntry;
    
    	if( ( pstDirectory == NULL ) ||
        	( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
    	{
        	return NULL;
    	}
    	pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );
    	if( ( pstDirectoryHandle->iCurrentOffset < 0 ) ||
        	( pstDirectoryHandle->iCurrentOffset >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    	{
        	return NULL;
    	}

    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
    	pstEntry = pstDirectoryHandle->pstDirectoryBuffer;
    	while( pstDirectoryHandle->iCurrentOffset < FILESYSTEM_MAXDIRECTORYENTRYCOUNT )
    	{
		  	
		if( pstEntry[ pstDirectoryHandle->iCurrentOffset ].bType
                	!= 0 )
        	{	
			*offset = pstDirectoryHandle->iCurrentOffset;            		
			kUnlock( &( gs_stFileSystemManager.stMutex ) );
            		return &( pstEntry[ pstDirectoryHandle->iCurrentOffset++ ] );
       	 	}
        
        	pstDirectoryHandle->iCurrentOffset++;
    	}

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
    	return NULL;
}

void kRewindDirectory( DIR* pstDirectory )
{
    	DIRECTORYHANDLE* pstDirectoryHandle;
    
    	if( ( pstDirectory == NULL ) ||
        	( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
    	{	
        	return ;
    	}
    	pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
    	pstDirectoryHandle->iCurrentOffset = 0;
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
}

int kCloseDirectory( DIR* pstDirectory )
{
    	DIRECTORYHANDLE* pstDirectoryHandle;
    
    	if( ( pstDirectory == NULL ) ||
        	( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
   	{
        	return -1;
    	}
    	pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );

    	kLock( &( gs_stFileSystemManager.stMutex ) );
    	
	
    	kFree( pstDirectoryHandle->pstDirectoryBuffer );
    	kFreeFileDirectoryHandle( pstDirectory );

    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );

    	return 0;
}

int kOpenDir( const char* pcDirectoryName )
{

	DIRECTORYENTRY dirEntry, dotEntry;
	int iDirectoryEntryOffset;
	
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
    
	iDirectoryEntryOffset = kFindDirectoryEntry( pcDirectoryName, &dirEntry );
	if( iDirectoryEntryOffset == -1 ) 
    	{ 
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    
    	if( kGetDirectoryEntryData( dirEntry.dwStartClusterIndex, 0, &dotEntry) == FALSE )
	{
		kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
	}

	gs_stFileSystemManager.pstDirIndex = dirEntry.dwStartClusterIndex;
	kMemCpy( gs_stFileSystemManager.dirRootName, dotEntry.vcFileName, kStrLen( dotEntry.vcFileName ) );

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
	return 0;
}

int kRemoveDir( const char* pcDirectoryName )
{

	DIRECTORYENTRY dirEntry, dotEntry;
	int iDirectoryEntryOffset;	
    
    	kLock( &( gs_stFileSystemManager.stMutex ) );
	
    
	iDirectoryEntryOffset = kFindDirectoryEntry( pcDirectoryName, &dirEntry );
	if( iDirectoryEntryOffset == -1 ) 
    	{ 
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    	
	if( kRemoveAllFile( dirEntry.dwStartClusterIndex ) == -1 )
	{
		kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
	}
	
	kMemSet( &dirEntry, 0, sizeof( dirEntry ) );
	if( kSetDirectoryEntryData( gs_stFileSystemManager.pstDirIndex, iDirectoryEntryOffset, &dirEntry ) == FALSE )
    	{
        	kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
    	}
    

    	kUnlock( &( gs_stFileSystemManager.stMutex ) );
	return 0;
}
int kRemoveAllFile( int dirIndex )
{
	DIRECTORYENTRY* pstEntry;
    	int i;

    	if( gs_stFileSystemManager.bMounted == FALSE )
    	{
        	return -1;
    	}

    	if( kReadCluster( dirIndex, gs_vbTempBuffer ) == FALSE )
    	{
        	return -1;
    	}
    
    	pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
	kMemSet( pstEntry, 0, sizeof( pstEntry ) );
    	
    	return 0;
}


int kCloseDir( void )
{
	DIRECTORYENTRY dotDotEntry;    	

    	kLock( &( gs_stFileSystemManager.stMutex ) );
	if( kGetDirectoryEntryData( gs_stFileSystemManager.pstDirIndex , 1, &dotDotEntry) == FALSE )
	{
		kUnlock( &( gs_stFileSystemManager.stMutex ) );
        	return -1;
	}
    	
	gs_stFileSystemManager.pstDirIndex = dotDotEntry.dwStartClusterIndex;

	kMemSet( gs_stFileSystemManager.dirRootName, ' ' , kStrLen( gs_stFileSystemManager.dirRootName ) );
	kMemCpy( gs_stFileSystemManager.dirRootName, dotDotEntry.vcFileName, kStrLen( dotDotEntry.vcFileName ) + 1 );
    
    	kUnlock( &( gs_stFileSystemManager.stMutex ) );

    	return 0;
}
