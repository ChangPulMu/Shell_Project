#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

#define FILESYSTEM_SIGNATURE                0x7E38CF10
#define FILESYSTEM_SECTORSPERCLUSTER        8
#define FILESYSTEM_LASTCLUSTER              0xFFFFFFFF
#define FILESYSTEM_FREECLUSTER              0x00
#define FILESYSTEM_MAXDIRECTORYENTRYCOUNT   ( ( FILESYSTEM_SECTORSPERCLUSTER * 512 ) / \
        sizeof( DIRECTORYENTRY ) )
#define FILESYSTEM_CLUSTERSIZE              ( FILESYSTEM_SECTORSPERCLUSTER * 512 )

#define FILESYSTEM_HANDLE_MAXCOUNT          ( TASK_MAXCOUNT * 3 )

#define FILESYSTEM_MAXFILENAMELENGTH        51

#define FILESYSTEM_TYPE_FREE                0
#define FILESYSTEM_TYPE_FILE                1
#define FILESYSTEM_TYPE_DIRECTORY           2

#define FILESYSTEM_SEEK_SET                 0
#define FILESYSTEM_SEEK_CUR                 1
#define FILESYSTEM_SEEK_END                 2





typedef BOOL (* fReadHDDInformation ) ( BOOL bPrimary, BOOL bMaster, 
        HDDINFORMATION* pstHDDInformation );
typedef int (* fReadHDDSector ) ( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, 
        int iSectorCount, char* pcBuffer );
typedef int (* fWriteHDDSector ) ( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, 
        int iSectorCount, char* pcBuffer );

#define fopen       kOpenFile
#define fread       kReadFile
#define fwrite      kWriteFile
#define fseek       kSeekFile
#define fclose      kCloseFile
#define remove      kRemoveFile
#define opendir     kOpenDirectory
#define readdir     kReadDirectory
#define rewinddir   kRewindDirectory
#define closedir    kCloseDirectory
#define update      kUpdateDirectoryEntry
#define SEEK_SET    FILESYSTEM_SEEK_SET
#define SEEK_CUR    FILESYSTEM_SEEK_CUR
#define SEEK_END    FILESYSTEM_SEEK_END

#define size_t      DWORD       
#define dirent      kDirectoryEntryStruct
#define d_name      vcFileName


#pragma pack( push, 1 )

typedef struct kPartitionStruct
{
    	BYTE bBootableFlag;
    	BYTE vbStartingCHSAddress[ 3 ];
    	BYTE bPartitionType;
    	BYTE vbEndingCHSAddress[ 3 ];
   	DWORD dwStartingLBAAddress;
    	DWORD dwSizeInSector;
} PARTITION;


typedef struct kMBRStruct
{
    	BYTE vbBootCode[ 430 ];

    	DWORD dwSignature;
    	DWORD dwReservedSectorCount;
    	DWORD dwClusterLinkSectorCount;
    	DWORD dwTotalClusterCount;
    
   	PARTITION vstPartition[ 4 ];
    
    	BYTE vbBootLoaderSignature[ 2 ];
} MBR;


typedef struct kDirectoryEntryStruct
{
    	char vcFileName[ FILESYSTEM_MAXFILENAMELENGTH ];
	BYTE bType;
	DWORD dirClusterIndex;
    	DWORD dwFileSize;
    	DWORD dwStartClusterIndex;
} DIRECTORYENTRY;

typedef struct kFileHandleStruct
{
    	int iDirectoryEntryOffset;
    	DWORD dwFileSize;
    	DWORD dwStartClusterIndex;
    	DWORD dwCurrentClusterIndex;
    	DWORD dwPreviousClusterIndex;
    	DWORD dwCurrentOffset;
	DWORD dirClusterIndex;
} FILEHANDLE;

typedef struct kDirectoryHandleStruct
{
    	DIRECTORYENTRY* pstDirectoryBuffer;
	
    	int iCurrentOffset;
} DIRECTORYHANDLE;

typedef struct kFileDirectoryHandleStruct
{
    	BYTE bType;

    	union
    	{
        	FILEHANDLE stFileHandle;
        	DIRECTORYHANDLE stDirectoryHandle;
    	};    
} FILE, DIR;

typedef struct kFileSystemManagerStruct
{
    	BOOL bMounted;
    
	DWORD pstDirIndex;
    	DWORD dwReservedSectorCount;
    	DWORD dwClusterLinkAreaStartAddress;
    	DWORD dwClusterLinkAreaSize;
   	DWORD dwDataAreaStartAddress;   
    	DWORD dwTotalClusterCount;
    
    	DWORD dwLastAllocatedClusterLinkSectorOffset;
    
    	MUTEX stMutex;    
    	FILE* pstHandlePool;
	char dirRootName[100];
	
} FILESYSTEMMANAGER;



BOOL kInitializeFileSystem( void );
BOOL kFormat( void );
BOOL kMount( void );
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation);

static BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer );
static BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer );
static BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer );
static BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer );
static DWORD kFindFreeCluster( void );
static BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData );
static BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData );
int kFindFreeDirectoryEntry( void );
BOOL kSetDirectoryEntryData( int dirIndex ,int iIndex, DIRECTORYENTRY* pstEntry );
BOOL kGetDirectoryEntryData( int dirIndex, int iIndex, DIRECTORYENTRY* pstEntry );
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry );
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager );
int kFindDirectoryEntryByIndex(int  dirIndex, DIRECTORYENTRY* pstEntry);

FILE* kOpenFile( const char* pcFileName, const char* pcMode );
DWORD kReadFile( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile );
DWORD kWriteFile( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile );
int kSeekFile( FILE* pstFile, int iOffset, int iOrigin );
int kCloseFile( FILE* pstFile );
int kRemoveFile( const char* pcFileName );
int kRenameFile(const char* pcFileName, const char* pcNewFileName);
DIR* kOpenDirectory( void );
struct kDirectoryEntryStruct* kReadDirectory( DIR* pstDirectory, int* offset );
void kRewindDirectory( DIR* pstDirectory );
int kCloseDirectory( DIR* pstDirectory );
BOOL kWriteZero( FILE* pstFile, DWORD dwCount );
BOOL kIsFileOpened( const DIRECTORYENTRY* pstEntry );
int kOpenDir( const char* pcDirectoryName );
int kCloseDir( void );
int kRemoveAllFile( int dirIndex );

static void* kAllocateFileDirectoryHandle( int bType );
static void kFreeFileDirectoryHandle( FILE* pstFile );
static BOOL kCreateFile( const char* pcFileName, DIRECTORYENTRY* pstEntry, 
        int* piDirectoryEntryIndex );
static BOOL kFreeClusterUntilEnd( DWORD dwClusterIndex );
BOOL kUpdateDirectoryEntry( FILEHANDLE* pstFileHandle );
BOOL kCreateDir( const char* pcDirName );
int kRemoveDir( const char* pcDirectoryName );

FILESYSTEMMANAGER   gs_stFileSystemManager;

#endif/*__FILESYSTEM_H__*/
