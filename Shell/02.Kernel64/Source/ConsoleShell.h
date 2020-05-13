
#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"
#include "FileSystem.h"
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "MINT64:"

typedef void ( * CommandFunction ) ( const char* pcParameter );


#pragma pack( push, 1 )

typedef struct kShellCommandEntryStruct
{
    	char* pcCommand;
    	char* pcHelp;
    	CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

typedef struct kParameterListStruct
{
	const char* pcBuffer;
    	int iLength;
    	int iCurrentPosition;
} PARAMETERLIST;

typedef struct kPreCommandStruct
{
	const char commandBuffer[80];
	int iLength;
	int iIndex;
} PRECOMMANDS;


#pragma pack( pop )

void kStartConsoleShell( void );
int kExecuteCommand( const char* pcCommandBuffer, int index, PRECOMMANDS* pstPreCommand );
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter );
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter );
void kSetTaskBar( void );

static void kHelp( const char* pcParameterBuffer );
static void kCls( const char* pcParameterBuffer );
static void kShowTotalRAMSize( const char* pcParameterBuffer );
static void kStringToDecimalHexTest( const char* pcParameterBuffer );
static void kShutdown( const char* pcParamegerBuffer );
static void kSetTimer( const char* pcParameterBuffer );
static void kWaitUsingPIT( const char* pcParameterBuffer );
static void kReadTimeStampCounter( const char* pcParameterBuffer );
static void kMeasureProcessorSpeed( const char* pcParameterBuffer );
static void kTimeCommand( const char* pcParameterBuffer );



static void kShowDateAndTime( const char* pcParameterBuffer );
static void kCreateTestTask( const char* pcParameterBuffer );
static void kTestTask( const char* pcParameterBuffer );	//
static void kCreateTestThread( const char* pcParameterBuffer );
static void kChangeTaskPriority( const char* pcParameterBuffer );
static void kShowTaskList( const char* pcParameterBuffer );
static void kKillTask( const char* pcParameterBuffer );
static void kCPULoad( const char* pcParameterBuffer );
static void kPrintNumberTask( void );
static void kTestMutex( const char* pcParameterBuffer );
static void kCreateThreadTask( void );
static void kTestThread( const char* pcParameterBuffer );
static void kShowMatrix( const char* pcParameterBuffer );
static void kTestPIE( const char* pcParameterBuffer );
static void kShowDyanmicMemoryInformation( const char* pcParameterBuffer );
static void kTestSequentialAllocation( const char* pcParameterBuffer );
static void kTestRandomAllocation( const char* pcParameterBuffer );
static void kRandomAllocationTask( void );
static void kTestMalloc(const char* pcParameterBuffer);
static void kHistoryInfo( void );
static void kTLCInfo( void );
static void kShowHDDInformation( const char* pcParameterBuffer );
static void kReadSector( const char* pcParameterBuffer );
static void kWriteSector( const char* pcParameterBuffer );
static void kMountHDD( const char* pcParameterBuffer );
static void kFormatHDD( const char* pcParameterBuffer );
static void kShowFileSystemInformation( const char* pcParameterBuffer );
static void kCreateFileInRootDirectory( const char* pcParameterBuffer );
static void kDeleteFileInRootDirectory( const char* pcParameterBuffer );
static void kShowRootDirectory( const char* pcParameterBuffer );
static void kWriteDataToFile( const char* pcParameterBuffer );
static void kReadDataFromFile( const char* pcParameterBuffer );
static void kCdDir( const char* pcParameterBuffer );
static void kTestFileIO( const char* pcParameterBuffer );
static void kMakeDir( const char* pcParameterBuffer );
static void krmDir( const char* pcParameterBuffer );
static void move(const char* pcParameterBuffer);
static void copy(const char* pcParameterBuffer);
static void rename(const char* pcParameterBuffer);
#endif /*__CONSOLESHELL_H__*/
