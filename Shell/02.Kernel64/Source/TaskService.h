
#ifndef __TASKSERVICE_H__
#define __TASKSERVICE_H__

#include "Types.h"

#define HISTORYADDRESS     	0x800000
#define MAX_HTASKCOUNT		30
#define TLCPOOLADDRESS		( HISTORYADDRESS + sizeof(HISTORY) )

typedef struct taskLinkCandidate
{
	int fINum;
	int bINum;
	int weight;
} TLC;

typedef struct taskInfo
{
	int iNum;	
	char name[100];
} TASKINFO;

typedef struct history
{
	TASKINFO hTask[30];
	int count;
} HISTORY;

typedef struct tlcPool
{
	TLC tlc[1000];

} TLCPOOL;

void kInitializeTLCPool( void );
void TLCManager( void );
TLC* searchTLC( int a, int b, int level );
void createTLC( TLC* pstTLC, int a, int b );
void getTLCInfo( void );
void checkTLC( TASKINFO taskInfo );
TASKINFO getTaskInfo( int iNum );
void kInitializeHistory( void );
void putHistory( TASKINFO taskInfo );
void removeHistory( TASKINFO taskInfo );
void getHistoryInfo( int* count, int* hTaskBuff );
BOOL sendMessage( TASKINFO taskInfo );
BOOL searchTask( TASKINFO taskInfo );
void checkPreHistory( void );

#endif /*__TASKSERVICE_H__*/
