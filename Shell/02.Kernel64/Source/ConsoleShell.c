#include "FileSystem.h"
#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "TaskService.h"
#include "Synchronization.h"
#include "InterruptHandler.h"
#include "DynamicMemory.h"
#include "HardDisk.h"

SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
		{ "help", "Show Help", kHelp },
		{ "cls", "Clear Screen", kCls },
		{ "totalram", "Show Total RAM Size", kShowTotalRAMSize },
		{ "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
		{ "shutdown", "Shutdown And Reboot OS", kShutdown },
	{ "settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)",
				kSetTimer },
		{ "wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT },
		{ "rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter },
		{ "cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed },
		{ "date", "Show Date And Time", kShowDateAndTime },
		{ "createtask", "Create Task", kCreateTestTask },
		{ "time", "Show Command Time", kTimeCommand },
		{ "createtask", "Create Task, ex)createtask 1(type) 10(count)", kCreateTestTask },
	{ "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
				kChangeTaskPriority },
		{ "tasklist", "Show Task List", kShowTaskList },
		{ "killtask", "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)", kKillTask },
		{ "cpuload", "Show Processor Load", kCPULoad },
		{ "testmutex", "Test Mutex Function", kTestMutex },
		{ "createthread", "Create Threed, ex)createthreed 1(type) 10(count)", kCreateTestThread },
	{ "testthread", "Test Thread And Process Function", kTestThread },
		{ "showmatrix", "Show Matrix Screen", kShowMatrix },
		{ "testtask", "testTask", kTestTask },
		{ "testpie", "Test PIE Calculation", kTestPIE },
	{ "dynamicmeminfo", "Show Dyanmic Memory Information", kShowDyanmicMemoryInformation },
		{ "testseqalloc", "Test Sequential Allocation & Free", kTestSequentialAllocation },
		{ "testranalloc", "Test Random Allocation & Free", kTestRandomAllocation },
		{ "testmalloc", "createmalloc", kTestMalloc },
	{ "historyinfo", "historyListInfo", kHistoryInfo },
	{ "tlcinfo", "TLC information", kTLCInfo },
	{ "hddinfo", "Show HDD Information", kShowHDDInformation },
		{ "readsector", "Read HDD Sector, ex)readsector 0(LBA) 10(count)",
				kReadSector },
		{ "writesector", "Write HDD Sector, ex)writesector 0(LBA) 10(count)",
				kWriteSector },
	{ "mounthdd", "Mount HDD", kMountHDD },
		{ "formathdd", "Format HDD", kFormatHDD },
		{ "filesysteminfo", "Show File System Information", kShowFileSystemInformation },
		{ "createfile", "Create File, ex)createfile a.txt", kCreateFileInRootDirectory },
		{ "deletefile", "Delete File, ex)deletefile a.txt", kDeleteFileInRootDirectory },
		{ "dir", "Show Directory", kShowRootDirectory },
	{ "writefile", "Write Data To File, ex) writefile a.txt", kWriteDataToFile },
		{ "readfile", "Read Data From File, ex) readfile a.txt", kReadDataFromFile },
		{ "testfileio", "Test File I/O Function", kTestFileIO },
	{ "cd", "cd", kCdDir },
	{ "makedir","make dir", kMakeDir },
	{ "rmdir", "remove dir" , krmDir },
	{ "cp","copy",copy},   //cp command
	{ "mv","move",move},   //mv command
	{"rename","rename",rename},//rename command  follow function
};

void kStartConsoleShell(void)
{
	char vcCommandBuffer[CONSOLESHELL_MAXCOMMANDBUFFERCOUNT];
	int iCommandBufferIndex = 0;
	BYTE bKey;
	int iCursorX, iCursorY;
	PRECOMMANDS preCommand[10] = { 0, };
	int index = 0;
	int maxIndex = 0;
	int comPointer = 0;
	checkPreHistory();

	kPrintf("%s%s%c", CONSOLESHELL_PROMPTMESSAGE, gs_stFileSystemManager.dirRootName, '>');

	//kSetTaskBar();
	while (1)
	{
		kSetTaskBar();
		bKey = kGetCh();

		if (bKey == KEY_BACKSPACE)
		{
			if (iCommandBufferIndex > 0)
			{
				kGetCursor(&iCursorX, &iCursorY);
				kPrintStringXY(iCursorX - 1, iCursorY, " ");
				kSetCursor(iCursorX - 1, iCursorY);
				iCommandBufferIndex--;
			}
			comPointer = index - 1;
		}
		else if (bKey == KEY_ENTER)
		{
			kPrintf("\n");

			if (iCommandBufferIndex > 0)
			{
				vcCommandBuffer[iCommandBufferIndex] = '\0';
				index = kExecuteCommand(vcCommandBuffer, index, &preCommand[index]);
				if (maxIndex == 9)
				{
					maxIndex++;
				}

				if (index > maxIndex)
				{
					maxIndex = index;
				}
			}

			kPrintf("%s%s%c", CONSOLESHELL_PROMPTMESSAGE, gs_stFileSystemManager.dirRootName, '>');
			kMemSet(vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT);
			iCommandBufferIndex = 0;
			comPointer = index - 1;
		}
		else if (bKey == KEY_UP)
		{
			if (comPointer < 0)
			{
				comPointer = maxIndex - 1;
			}
			kGetCursor(&iCursorX, &iCursorY);
			kClearScreenLine(iCursorX, iCursorY, preCommand[comPointer].iLength);
			kPrintf("MINT64>%s", preCommand[comPointer].commandBuffer);
			for (iCommandBufferIndex = 0; iCommandBufferIndex < preCommand[comPointer].iLength; iCommandBufferIndex++)
			{
				vcCommandBuffer[iCommandBufferIndex] = ((char*)preCommand[comPointer].commandBuffer)[iCommandBufferIndex];
			}
			comPointer--;
		}
		else if (bKey == KEY_DOWN)
		{
			comPointer++;
			if (comPointer >= maxIndex)
			{
				comPointer = 0;
			}
			kGetCursor(&iCursorX, &iCursorY);
			kClearScreenLine(iCursorX, iCursorY, preCommand[comPointer].iLength);
			kPrintf("MINT64>%s", preCommand[comPointer].commandBuffer);
			for (iCommandBufferIndex = 0; iCommandBufferIndex < preCommand[comPointer].iLength; iCommandBufferIndex++)
			{
				vcCommandBuffer[iCommandBufferIndex] = ((char*)preCommand[comPointer].commandBuffer)[iCommandBufferIndex];
			}
		}
		else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) ||
			(bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
			(bKey == KEY_SCROLLLOCK))
		{
			comPointer = index - 1;
		}
		else
		{
			if (bKey == KEY_TAB)
			{
				bKey = ' ';
			}

			if (iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT)
			{
				vcCommandBuffer[iCommandBufferIndex++] = bKey;
				kPrintf("%c", bKey);
			}
			comPointer = index - 1;
		}
	}
}

int kExecuteCommand(const char* pcCommandBuffer, int index, PRECOMMANDS* preCommand)
{
	int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;
	int iCount;

	iCommandBufferLength = kStrLen(pcCommandBuffer);
	for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
	{
		if (pcCommandBuffer[iSpaceIndex] == ' ')
		{
			break;
		}
	}

	iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for (i = 0; i < iCount; i++)
	{
		iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);
		if ((iCommandLength == iSpaceIndex) &&
			(kMemCmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer,
				iSpaceIndex) == 0))
		{
			timeCounter = 1;
			timerON = 1;
			kInitializePIT(MSTOCOUNT(1), 1);
			gs_vstCommandTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
			timerON = 0;

			kMemCpy(preCommand->commandBuffer, pcCommandBuffer, 80);
			preCommand->iLength = iCommandBufferLength;
			preCommand->iIndex = index;
			if (index < 9)
			{
				index++;
				return index;
			}
			else
			{
				return 0;
			}
			break;
		}
	}

	if (i >= iCount)
	{
		kPrintf("'%s' is not found.\n", pcCommandBuffer);
		return index;
	}
}

void kInitializeParameter(PARAMETERLIST* pstList, const char* pcParameter)
{
	pstList->pcBuffer = pcParameter;
	pstList->iLength = kStrLen(pcParameter);
	pstList->iCurrentPosition = 0;
}

int kGetNextParameter(PARAMETERLIST* pstList, char* pcParameter)
{
	int i;
	int iLength;

	if (pstList->iLength <= pstList->iCurrentPosition)
	{
		return 0;
	}

	for (i = pstList->iCurrentPosition; i < pstList->iLength; i++)
	{
		if (pstList->pcBuffer[i] == ' ')
		{
			break;
		}
	}

	kMemCpy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
	iLength = i - pstList->iCurrentPosition;
	pcParameter[iLength] = '\0';
	pstList->iCurrentPosition += iLength + 1;

	return iLength;
}

static void kHelp(const char* pcCommandBuffer)
{
	int i;
	int iCount;
	int iCursorX, iCursorY;
	int iLength, iMaxCommandLength = 0;

	kPrintf("=========================================================\n");
	kPrintf("                    MINT64 Shell Help                    \n");
	kPrintf("=========================================================\n");

	iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	for (i = 0; i < iCount; i++)
	{
		iLength = kStrLen(gs_vstCommandTable[i].pcCommand);
		if (iLength > iMaxCommandLength)
		{
			iMaxCommandLength = iLength;
		}
	}

	for (i = 0; i < iCount; i++)
	{
		kPrintf("%s", gs_vstCommandTable[i].pcCommand);
		kGetCursor(&iCursorX, &iCursorY);
		kSetCursor(iMaxCommandLength, iCursorY);
		kPrintf("  - %s\n", gs_vstCommandTable[i].pcHelp);

		if ((i != 0) && ((i % 18) == 0))
		{
			kPrintf("Press any key to continue... ('q' is exit) : ");
			if (kGetCh() == 'q')
			{
				kPrintf("\n");
				break;
			}
			kPrintf("\n");
		}
	}
}

static void kCls(const char* pcParameterBuffer)
{
	kClearScreen();
	kSetCursor(0, 1);
}

static void kShowTotalRAMSize(const char* pcParameterBuffer)
{
	kPrintf("Total RAM Size = %d MB\n", kGetTotalRAMSize());
}

static void kStringToDecimalHexTest(const char* pcParameterBuffer)
{
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	kInitializeParameter(&stList, pcParameterBuffer);

	while (1)
	{
		iLength = kGetNextParameter(&stList, vcParameter);
		if (iLength == 0)
		{
			break;
		}

		kPrintf("Param %d = '%s', Length = %d, ", iCount + 1,
			vcParameter, iLength);

		if (kMemCmp(vcParameter, "0x", 2) == 0)
		{
			lValue = kAToI(vcParameter + 2, 16);
			kPrintf("HEX Value = %q\n", lValue);
		}
		else
		{
			lValue = kAToI(vcParameter, 10);
			kPrintf("Decimal Value = %d\n", lValue);
		}

		iCount++;
	}
}

static void kShutdown(const char* pcParamegerBuffer)
{
	kPrintf("System Shutdown Start...\n");

	kPrintf("Press Any Key To Reboot PC...");
	kGetCh();
	kReboot();
}

static void kSetTimer(const char* pcParameterBuffer)
{
	char vcParameter[100];
	PARAMETERLIST stList;
	long lValue;
	BOOL bPeriodic;

	kInitializeParameter(&stList, pcParameterBuffer);

	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}

	lValue = kAToI(vcParameter, 10);

	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)settimer 10(ms) 1(periodic)\n");
		return;
	}
	bPeriodic = kAToI(vcParameter, 10);

	kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
	kPrintf("Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic);
}

static void kWaitUsingPIT(const char* pcParameterBuffer)
{
	char vcParameter[100];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;

	kInitializeParameter(&stList, pcParameterBuffer);
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)wait 100\n");
		return;
	}

	lMillisecond = kAToI(pcParameterBuffer, 10);
	kPrintf("%d ms Sleep Start...\n", lMillisecond);

	for (i = 0; i < lMillisecond / 50; i++)
	{
		kWaitUsingDirectPIT(MSTOCOUNT(50));
	}
	kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 50));

	kPrintf("%d ms Sleep Complete\n", lMillisecond);

	kInitializePIT(MSTOCOUNT(1), 1);
}

static void kReadTimeStampCounter(const char* pcParameterBuffer)
{
	QWORD qwTSC;

	qwTSC = kReadTSC();
	kPrintf("Time Stamp Counter = %q\n", qwTSC);
}

static void kMeasureProcessorSpeed(const char* pcParameterBuffer)
{
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;

	kPrintf("Now Measuring.");

	for (i = 0; i < 200; i++)
	{
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT(MSTOCOUNT(50));
		qwTotalTSC += kReadTSC() - qwLastTSC;

		kPrintf(".");
	}
	kInitializePIT(MSTOCOUNT(1), 1);

	kPrintf("\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000);
}

static void kShowDateAndTime(const char* pcParameterBuffer)
{
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	kReadRTCTime(&bHour, &bMinute, &bSecond);
	kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

	kPrintf("Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
		kConvertDayOfWeekToString(bDayOfWeek));
	kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}
static void kTimeCommand(const char* pcParameterBuffer)
{
	char vcParameter[100];
	PARAMETERLIST stList;
	QWORD qwLastTSC, qwTotalTSC = 0;
	int i;

	kInitializeParameter(&stList, pcParameterBuffer);
	if (kGetNextParameter(&stList, vcParameter) == 0)
	{
		kPrintf("ex)time 'command'\n");
		return;
	}

	int min, sec, ms, mic, nano;
	int iSpaceIndex;
	int iCommandBufferLength, iCommandLength;
	int iCount;

	iCommandBufferLength = kStrLen(vcParameter);
	for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
	{
		if (vcParameter[iSpaceIndex] == ' ')
		{
			break;
		}
	}

	iCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);
	for (i = 0; i < iCount; i++)
	{
		iCommandLength = kStrLen(gs_vstCommandTable[i].pcCommand);
		if ((iCommandLength == iSpaceIndex) &&
			(kMemCmp(gs_vstCommandTable[i].pcCommand, vcParameter,
				iSpaceIndex) == 0))
		{
			qwLastTSC = kReadTSC();
			gs_vstCommandTable[i].pfFunction(pcParameterBuffer + iSpaceIndex + 1);
			qwTotalTSC += kReadTSC() - qwLastTSC;
			nano = (qwTotalTSC % 2700) / 3;
			mic = (qwTotalTSC % 2700000) / 2700;
			ms = (qwTotalTSC % 2700000000) / 2700000;
			sec = qwTotalTSC / 2700000000;

			kPrintf("real = %d:%d:%d:%d \n", sec, ms, mic, nano);
			break;
		}
	}

	if (i >= iCount)
	{
		kPrintf("'%s' is not found.\n", vcParameter);
	}
}

void kSetTaskBar(void)
{
	int j;
	int iX, iY;

	for (j = 0; j < 80; j++)
	{
		kPrintStringXY(j, 23, "=");
	}
}

static void kCreateTestTask(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcType[30];
	char vcCount[30];
	int i;

	// 파라미터를 추출
	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcType);
	kGetNextParameter(&stList, vcCount);

	switch (kAToI(vcType, 10))
	{
		// 타입 1 태스크 생성
	case 1:
		for (i = 0; i < kAToI(vcCount, 10); i++)
		{
			if (kCreateTask(TASK_FLAGS_MEDIUM | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask1, "testTask1", 1) == NULL)	//
			{
				break;
			}
		}

		kPrintf("Task1 %d Created\n", i);
		break;

		// 타입 2 태스크 생성
	case 2:
		for (i = 0; i < kAToI(vcCount, 10); i++)
		{
			if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask2, "testTask2", 2) == NULL)
			{
				break;
			}
		}
		kPrintf("Task2 %d Created\n", i);
		break;
	case 3:
	default:
		for (i = 0; i < kAToI(vcCount, 10); i++)
		{
			if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask3, "testTask3", 3) == NULL)
			{
				break;
			}
		}
		kPrintf("Task3 %d Created\n", i);
		break;
	}
}

static void kTestTask(const char* pcParameterBuffer)
{
	int i;

	for (i = 0; i < 1; i++)
	{
		if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask1, "testTask1", 1) == NULL)
		{
			break;
		}
	}
	for (i = 0; i < 1; i++)
	{
		if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask2, "testTask2", 2) == NULL)
		{
			break;
		}
	}
	for (i = 0; i < 1; i++)
	{
		if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, (QWORD)kTestTask3, "testTask3", 3) == NULL)
		{
			break;
		}
	}
	/*for( i = 0 ; i < 1 ; i++)
	{
		if( kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, 0, 0, ( QWORD ) kTestTask2 ) == NULL )
		{
			break;
		}
	}*/
}

static void kCreateTestThread(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcType[30];
	char vcCount[30];
	int i;

	// 파라미터를 추출
	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcType);
	kGetNextParameter(&stList, vcCount);

	switch (kAToI(vcType, 10))
	{
		// 타입 1 태스크 생성
	case 1:
		for (i = 0; i < kAToI(vcCount, 10); i++)
		{
			if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask1, 0, 0) == NULL)
			{
				break;
			}
		}

		kPrintf("Task1(Thread) %d Created\n", i);
		break;

		// 타입 2 태스크 생성
	case 2:
	default:
		for (i = 0; i < kAToI(vcCount, 10); i++)
		{
			if (kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask2, 0, 0) == NULL)
			{
				break;
			}
		}
		kPrintf("Task2(Thread) %d Created\n", i);
		break;
	}
}

static void kChangeTaskPriority(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcID[30];
	char vcPriority[30];
	QWORD qwID;
	BYTE bPriority;

	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcID);
	kGetNextParameter(&stList, vcPriority);

	if (kMemCmp(vcID, "0x", 2) == 0)
	{
		qwID = kAToI(vcID + 2, 16);
	}
	else
	{
		qwID = kAToI(vcID, 10);
	}

	bPriority = kAToI(vcPriority, 10);

	kPrintf("Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority);
	if (kChangePriority(qwID, bPriority) == TRUE)
	{
		kPrintf("Success\n");
	}
	else
	{
		kPrintf("Fail\n");
	}
}

static void kShowTaskList(const char* pcParameterBuffer)
{
	int i;
	TCB* pstTCB;
	int iCount = 0;
	kPrintf("=========== Task Total Task [%d] ===========\n", kGetTotalTask());
	kPrintf("=========== Task Total Tickets [%d] ===========\n", kGetTotalTicket());
	kPrintf("=========== Task Total Count [%d] ===========\n", kGetTaskCount());
	for (i = 0; i < TASK_MAXCOUNT; i++)
	{
		pstTCB = kGetTCBInTCBPool(i);
		if ((pstTCB->stLink.qwID >> 32) != 0)
		{
			if ((iCount != 0) && ((iCount % 10) == 0))
			{
				kPrintf("Press any key to continue... ('q' is exit) : ");
				if (kGetCh() == 'q')
				{
					kPrintf("\n");
					break;
				}
				kPrintf("\n");
			}

			/*kPrintf( "[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n", 1 + iCount++,
					pstTCB->stLink.qwID, GETPRIORITY( pstTCB->qwFlags ),
					pstTCB->qwFlags);
	*/
			kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d], Ticket[%d], UseCount[%d]\n", 1 + iCount++,
				pstTCB->stLink.qwID, GETPRIORITY(pstTCB->qwFlags),
				pstTCB->qwFlags, kGetListCount(&(pstTCB->stChildThreadList)), pstTCB->tickets, pstTCB->useCount);
			kPrintf("    Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n",
				pstTCB->qwParentProcessID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize);
		}
	}
}

static void kKillTask(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcID[30];
	QWORD qwID;
	TCB* pstTCB;
	int i;

	kInitializeParameter(&stList, pcParameterBuffer);
	kGetNextParameter(&stList, vcID);

	if (kMemCmp(vcID, "0x", 2) == 0)
	{
		qwID = kAToI(vcID + 2, 16);
	}
	else
	{
		qwID = kAToI(vcID, 10);
	}

	if (qwID != 0xFFFFFFFF)
	{
		pstTCB = kGetTCBInTCBPool(GETTCBOFFSET(qwID));
		qwID = pstTCB->stLink.qwID;

		if (((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
		{
			kPrintf("Kill Task ID [0x%q] ", qwID);
			if (kEndTask(qwID) == TRUE)
			{
				kPrintf("Success\n");
			}
			else
			{
				kPrintf("Fail\n");
			}
		}
		else
		{
			kPrintf("Task does not exist or task is system task\n");
		}
	}
	else
	{
		for (i = 0; i < TASK_MAXCOUNT; i++)
		{
			pstTCB = kGetTCBInTCBPool(i);
			qwID = pstTCB->stLink.qwID;

			if (((qwID >> 32) != 0) && ((pstTCB->qwFlags & TASK_FLAGS_SYSTEM) == 0x00))
			{
				kPrintf("Kill Task ID [0x%q] ", qwID);
				if (kEndTask(qwID) == TRUE)
				{
					kPrintf("Success\n");
				}
				else
				{
					kPrintf("Fail\n");
				}
			}
		}
	}
}

static void kCPULoad(const char* pcParameterBuffer)
{
	kPrintf("Processor Load : %d%%\n", kGetProcessorLoad());
}

static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

static void kPrintNumberTask(void)
{
	int i;
	int j;
	QWORD qwTickCount;

	qwTickCount = kGetTickCount();
	while ((kGetTickCount() - qwTickCount) < 50)
	{
		kSchedule();
	}

	// 루프를 돌면서 숫자를 출력
	for (i = 0; i < 5; i++)
	{
		kLock(&(gs_stMutex));
		kPrintf("Task ID [0x%Q] Value[%d]\n", kGetRunningTask()->stLink.qwID,
			gs_qwAdder);

		gs_qwAdder += 1;
		kUnlock(&(gs_stMutex));

		// 프로세서 소모를 늘리려고 추가한 코드
		for (j = 0; j < 30000; j++);
	}

	// 모든 태스크가 종료할 때까지 1초(100ms) 정도 대기
	qwTickCount = kGetTickCount();
	while ((kGetTickCount() - qwTickCount) < 1000)
	{
		kSchedule();
	}

	// 태스크 종료
	kExitTask();
}

static void kTestMutex(const char* pcParameterBuffer)
{
	int i;

	gs_qwAdder = 1;

	kInitializeMutex(&gs_stMutex);

	for (i = 0; i < 3; i++)
	{
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kPrintNumberTask, 0, 0);
	}
	kPrintf("Wait Util %d Task End...\n", i);
	kGetCh();
}
static void kCreateThreadTask(void)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kTestTask2, 0, 0);
	}

	while (1)
	{
		kSleep(1);
	}
}

static void kTestThread(const char* pcParameterBuffer)
{
	TCB* pstProcess;

	pstProcess = kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, (void*)0xEEEEEEEE, 0x1000,
		(QWORD)kCreateThreadTask, 0, 0);
	if (pstProcess != NULL)
	{
		kPrintf("Process [0x%Q] Create Success\n", pstProcess->stLink.qwID, 0, 0);
	}
	else
	{
		kPrintf("Process Create Fail\n");
	}
}

static volatile QWORD gs_qwRandomValue = 0;

QWORD kRandom(void)
{
	gs_qwRandomValue = (gs_qwRandomValue * 412153 + 5571031) >> 16;
	return gs_qwRandomValue;
}

static void kDropCharactorThread(void)
{
	int iX, iY;
	int i;
	char vcText[2] = { 0, };

	iX = kRandom() % CONSOLE_WIDTH;

	while (1)
	{
		// 잠시 대기함
		kSleep(kRandom() % 20);

		if ((kRandom() % 20) < 16)
		{
			vcText[0] = ' ';
			for (i = 0; i < CONSOLE_HEIGHT - 1; i++)
			{
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}
		}
		else
		{
			for (i = 0; i < CONSOLE_HEIGHT - 1; i++)
			{
				vcText[0] = i + kRandom();
				kPrintStringXY(iX, i, vcText);
				kSleep(50);
			}
		}
	}
}

static void kMatrixProcess(void)
{
	int i;

	for (i = 0; i < 300; i++)
	{
		if (kCreateTask(TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0,
			(QWORD)kDropCharactorThread, 0, 0) == NULL)
		{
			break;
		}

		kSleep(kRandom() % 5 + 5);
	}

	kPrintf("%d Thread is created\n", i);

	// 키가 입력되면 프로세스 종료
	kGetCh();
}

static void kShowMatrix(const char* pcParameterBuffer)
{
	TCB* pstProcess;

	pstProcess = kCreateTask(TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, (void*)0xE00000, 0xE00000,
		(QWORD)kMatrixProcess, 0, 0);
	if (pstProcess != NULL)
	{
		kPrintf("Matrix Process [0x%Q] Create Success\n");

		// 태스크가 종료 될 때까지 대기
		while ((pstProcess->stLink.qwID >> 32) != 0)
		{
			kSleep(100);
		}
	}
	else
	{
		kPrintf("Matrix Process Create Fail\n");
	}
}

static void kFPUTestTask(void)
{
	double dValue1;
	double dValue2;
	TCB* pstRunningTask;
	QWORD qwCount = 0;
	QWORD qwRandomValue;
	int i;
	int iOffset;
	char vcData[4] = { '-', '\\', '|', '/' };
	CHARACTER* pstScreen = (CHARACTER*)CONSOLE_VIDEOMEMORYADDRESS;

	pstRunningTask = kGetRunningTask();

	iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
	iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT -
		(iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

	while (1)
	{
		dValue1 = 1;
		dValue2 = 1;

		for (i = 0; i < 10; i++)
		{
			qwRandomValue = kRandom();
			dValue1 *= (double)qwRandomValue;
			dValue2 *= (double)qwRandomValue;

			kSleep(1);

			qwRandomValue = kRandom();
			dValue1 /= (double)qwRandomValue;
			dValue2 /= (double)qwRandomValue;
		}

		if (dValue1 != dValue2)
		{
			kPrintf("Value Is Not Same~!!! [%f] != [%f]\n", dValue1, dValue2);
			break;
		}
		qwCount++;

		pstScreen[iOffset].bCharactor = vcData[qwCount % 4];

		pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
	}
}

static void kTestPIE(const char* pcParameterBuffer)
{
	double dResult;
	int i;

	kPrintf("PIE Cacluation Test\n");
	kPrintf("Result: 355 / 113 = ");
	dResult = (double)355 / 113;
	kPrintf("%d.%d%d\n", (QWORD)dResult, ((QWORD)(dResult * 10) % 10),
		((QWORD)(dResult * 100) % 10));

	for (i = 0; i < 100; i++)
	{
		kCreateTask(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)kFPUTestTask, 0, 0);
	}
}

static void kShowDyanmicMemoryInformation(const char* pcParameterBuffer)
{
	QWORD qwStartAddress, qwTotalSize, qwMetaSize, qwUsedSize;
	int useNum, listCount;
	long lefSize;

	kGetDynamicMemoryInformation(&qwStartAddress, &qwTotalSize, &qwMetaSize,
		&qwUsedSize);

	getFreeListInfo(&useNum, &lefSize);
	listCount = getCountFreeList();

	kPrintf("============ Dynamic Memory Information ============\n");
	kPrintf("Start Address: [0x%Q]\n", qwStartAddress);
	kPrintf("Total Size:    [0x%Q]byte, [%d]MB\n", qwTotalSize,
		qwTotalSize / 1024 / 1024);
	kPrintf("Meta Size:     [0x%Q]byte, [%d]KB\n", qwMetaSize,
		qwMetaSize / 1024);
	kPrintf("Used Size:     [0x%Q]byte, [%d]KB\n", qwUsedSize, qwUsedSize / 1024);
	kPrintf("Used FreeList:     [%d]use, [%d]left, [%d]count\n", useNum, lefSize, listCount);
}

static void kTestSequentialAllocation(const char* pcParameterBuffer)
{
	DYNAMICMEMORY* pstMemory;
	long i, j, k;
	QWORD* pqwBuffer;

	kPrintf("============ Dynamic Memory Test ============\n");
	pstMemory = kGetDynamicMemoryManager();

	for (i = 0; i < pstMemory->iMaxLevelCount; i++)
	{
		kPrintf("Block List [%d] Test Start\n", i);
		kPrintf("Allocation And Compare: ");

		// 모든 블록을 할당 받아서 값을 채운 후 검사
		for (j = 0; j < (pstMemory->iBlockCountOfSmallestBlock >> i); j++)
		{
			pqwBuffer = kMalloc(DYNAMICMEMORY_MIN_SIZE << i);
			if (pqwBuffer == NULL)
			{
				kPrintf("\nAllocation Fail\n");
				return;
			}

			// 값을 채운 후 다시 검사
			for (k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
			{
				pqwBuffer[k] = k;
			}

			for (k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; k++)
			{
				if (pqwBuffer[k] != k)
				{
					kPrintf("\nCompare Fail\n");
					return;
				}
			}
			// 진행 과정을 . 으로 표시
			kPrintf(".");
		}

		kPrintf("\nFree: ");
		// 할당 받은 블록을 모두 반환
		for (j = 0; j < (pstMemory->iBlockCountOfSmallestBlock >> i); j++)
		{
			if (kFreeMemory((void*)(pstMemory->qwStartAddress +
				(DYNAMICMEMORY_MIN_SIZE << i) * j)) == FALSE)
			{
				kPrintf("\nFree Fail\n");
				return;
			}
			// 진행 과정을 . 으로 표시
			kPrintf(".");
		}
		kPrintf("\n");
	}
	kPrintf("Test Complete~!!!\n");
}

static void kRandomAllocationTask(void)
{
	TCB* pstTask;
	QWORD qwMemorySize;
	char vcBuffer[200];
	BYTE* pbAllocationBuffer;
	int i, j;
	int iY;

	pstTask = kGetRunningTask();
	iY = (pstTask->stLink.qwID) % 15 + 9;

	for (j = 0; j < 10; j++)
	{
		// 1KB ~ 32M까지 할당하도록 함
		do
		{
			qwMemorySize = ((kRandom() % (32 * 1024)) + 1) * 1024;
			pbAllocationBuffer = kMalloc(qwMemorySize);

			// 만일 버퍼를 할당 받지 못하면 다른 태스크가 메모리를 사용하고
			// 있을 수 있으므로 잠시 대기한 후 다시 시도
			if (pbAllocationBuffer == 0)
			{
				kSleep(1);
			}
		} while (pbAllocationBuffer == 0);

		kSPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Allocation Success",
			pbAllocationBuffer, qwMemorySize);
		// 자신의 ID를 Y 좌표로 하여 데이터를 출력
		kPrintStringXY(20, iY, vcBuffer);
		kSleep(200);

		// 버퍼를 반으로 나눠서 랜덤한 데이터를 똑같이 채움
		kSPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Write...     ",
			pbAllocationBuffer, qwMemorySize);
		kPrintStringXY(20, iY, vcBuffer);
		for (i = 0; i < qwMemorySize / 2; i++)
		{
			pbAllocationBuffer[i] = kRandom() & 0xFF;
			pbAllocationBuffer[i + (qwMemorySize / 2)] = pbAllocationBuffer[i];
		}
		kSleep(200);

		// 채운 데이터가 정상적인지 다시 확인
		kSPrintf(vcBuffer, "|Address: [0x%Q] Size: [0x%Q] Data Verify...   ",
			pbAllocationBuffer, qwMemorySize);
		kPrintStringXY(20, iY, vcBuffer);
		for (i = 0; i < qwMemorySize / 2; i++)
		{
			if (pbAllocationBuffer[i] != pbAllocationBuffer[i + (qwMemorySize / 2)])
			{
				kPrintf("Task ID[0x%Q] Verify Fail\n", pstTask->stLink.qwID);
				kExitTask();
			}
		}
		kFreeMemory(pbAllocationBuffer);
		kSleep(200);
	}

	kExitTask();
}

/**
 *  태스크를 여러 개 생성하여 임의의 메모리를 할당하고 해제하는 것을 반복하는 테스트
 */
static void kTestRandomAllocation(const char* pcParameterBuffer)
{
	int i;

	for (i = 0; i < 100; i++)
	{
		kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, (QWORD)kRandomAllocationTask, 0, 0);
	}
}

static void kTestMalloc(const char* pcParameterBuffer)
{
	int* a = (int*)kMalloc(32);
	*a = 5;
	kPrintf("a =[%d]\n", *a);

	int* b = (int*)kMalloc(4);
	*b = 5;
	kPrintf("a =[%d]\n", *b);
	kFree(a);
}

static void kTestFree(const char* pcParameterBuffer)
{
}

static void kHistoryInfo(void)
{
	int hTaskList[30];
	int i, count;

	getHistoryInfo(&count, &hTaskList);

	kPrintf("============ History Information ============\n");
	kPrintf("inHistoryCount: [%d]\n", count);
	for (i = 0; i < count; i++)
	{
		kPrintf("List[%d], inHistoryInum: [%d]\n", i, hTaskList[i]);
	}
}

static void kTLCInfo(void)
{
	kPrintf("============ TLC Information ============\n");
	getTLCInfo();
}

static void kShowHDDInformation(const char* pcParameterBuffer)
{
	HDDINFORMATION stHDD;
	char vcBuffer[100];

	// 하드 디스크의 정보를 읽음
	if (kReadHDDInformation(TRUE, TRUE, &stHDD) == FALSE)
	{
		kPrintf("HDD Information Read Fail\n");
		return;
	}

	kPrintf("============ Primary Master HDD Information ============\n");

	// 모델 번호 출력
	kMemCpy(vcBuffer, stHDD.vwModelNumber, sizeof(stHDD.vwModelNumber));
	vcBuffer[sizeof(stHDD.vwModelNumber) - 1] = '\0';
	kPrintf("Model Number:\t %s\n", vcBuffer);

	// 시리얼 번호 출력
	kMemCpy(vcBuffer, stHDD.vwSerialNumber, sizeof(stHDD.vwSerialNumber));
	vcBuffer[sizeof(stHDD.vwSerialNumber) - 1] = '\0';
	kPrintf("Serial Number:\t %s\n", vcBuffer);

	// 헤드, 실린더, 실린더 당 섹터 수를 출력
	kPrintf("Head Count:\t %d\n", stHDD.wNumberOfHead);
	kPrintf("Cylinder Count:\t %d\n", stHDD.wNumberOfCylinder);
	kPrintf("Sector Count:\t %d\n", stHDD.wNumberOfSectorPerCylinder);

	// 총 섹터 수 출력
	kPrintf("Total Sector:\t %d Sector, %dMB\n", stHDD.dwTotalSectors,
		stHDD.dwTotalSectors / 2 / 1024);
}

static void kReadSector(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcLBA[50], vcSectorCount[50];
	DWORD dwLBA;
	int iSectorCount;
	char* pcBuffer;
	int i, j;
	BYTE bData;
	BOOL bExit = FALSE;

	kInitializeParameter(&stList, pcParameterBuffer);
	if ((kGetNextParameter(&stList, vcLBA) == 0) ||
		(kGetNextParameter(&stList, vcSectorCount) == 0))
	{
		kPrintf("ex) readsector 0(LBA) 10(count)\n");
		return;
	}
	dwLBA = kAToI(vcLBA, 10);
	iSectorCount = kAToI(vcSectorCount, 10);

	pcBuffer = kMalloc(iSectorCount * 512);
	if (kReadHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) == iSectorCount)
	{
		kPrintf("LBA [%d], [%d] Sector Read Success~!!", dwLBA, iSectorCount);
		for (j = 0; j < iSectorCount; j++)
		{
			for (i = 0; i < 512; i++)
			{
				if (!((j == 0) && (i == 0)) && ((i % 256) == 0))
				{
					kPrintf("\nPress any key to continue... ('q' is exit) : ");
					if (kGetCh() == 'q')
					{
						bExit = TRUE;
						break;
					}
				}

				if ((i % 16) == 0)
				{
					kPrintf("\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i);
				}

				bData = pcBuffer[j * 512 + i] & 0xFF;
				if (bData < 16)
				{
					kPrintf("0");
				}
				kPrintf("%X ", bData);
			}

			if (bExit == TRUE)
			{
				break;
			}
		}
		kPrintf("\n");
	}
	else
	{
		kPrintf("Read Fail\n");
	}

	kFree(pcBuffer);
}

static void kWriteSector(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcLBA[50], vcSectorCount[50];
	DWORD dwLBA;
	int iSectorCount;
	char* pcBuffer;
	int i, j;
	BOOL bExit = FALSE;
	BYTE bData;
	static DWORD s_dwWriteCount = 0;

	kInitializeParameter(&stList, pcParameterBuffer);
	if ((kGetNextParameter(&stList, vcLBA) == 0) ||
		(kGetNextParameter(&stList, vcSectorCount) == 0))
	{
		kPrintf("ex) writesector 0(LBA) 10(count)\n");
		return;
	}
	dwLBA = kAToI(vcLBA, 10);
	iSectorCount = kAToI(vcSectorCount, 10);

	s_dwWriteCount++;

	pcBuffer = kMalloc(iSectorCount * 512);
	for (j = 0; j < iSectorCount; j++)
	{
		for (i = 0; i < 512; i += 8)
		{
			*(DWORD*)& (pcBuffer[j * 512 + i]) = dwLBA + j;
			*(DWORD*)& (pcBuffer[j * 512 + i + 4]) = s_dwWriteCount;
		}
	}

	if (kWriteHDDSector(TRUE, TRUE, dwLBA, iSectorCount, pcBuffer) != iSectorCount)
	{
		kPrintf("Write Fail\n");
		return;
	}
	kPrintf("LBA [%d], [%d] Sector Write Success~!!", dwLBA, iSectorCount);

	for (j = 0; j < iSectorCount; j++)
	{
		for (i = 0; i < 512; i++)
		{
			if (!((j == 0) && (i == 0)) && ((i % 256) == 0))
			{
				kPrintf("\nPress any key to continue... ('q' is exit) : ");
				if (kGetCh() == 'q')
				{
					bExit = TRUE;
					break;
				}
			}

			if ((i % 16) == 0)
			{
				kPrintf("\n[LBA:%d, Offset:%d]\t| ", dwLBA + j, i);
			}

			bData = pcBuffer[j * 512 + i] & 0xFF;
			if (bData < 16)
			{
				kPrintf("0");
			}
			kPrintf("%X ", bData);
		}

		if (bExit == TRUE)
		{
			break;
		}
	}
	kPrintf("\n");
	kFree(pcBuffer);
}

static void kMountHDD(const char* pcParameterBuffer)
{
	if (kMount() == FALSE)
	{
		kPrintf("HDD Mount Fail\n");
		return;
	}
	kPrintf("HDD Mount Success\n");
}

static void kFormatHDD(const char* pcParameterBuffer)
{
	if (kFormat() == FALSE)
	{
		kPrintf("HDD Format Fail\n");
		return;
	}
	kPrintf("HDD Format Success\n");
}

static void kShowFileSystemInformation(const char* pcParameterBuffer)
{
	FILESYSTEMMANAGER stManager;

	kGetFileSystemInformation(&stManager);

	kPrintf("================== File System Information ==================\n");
	kPrintf("Mouted:\t\t\t\t\t %d\n", stManager.bMounted);
	kPrintf("Reserved Sector Count:\t\t\t %d Sector\n", stManager.dwReservedSectorCount);
	kPrintf("Cluster Link Table Start Address:\t %d Sector\n",
		stManager.dwClusterLinkAreaStartAddress);
	kPrintf("Cluster Link Table Size:\t\t %d Sector\n", stManager.dwClusterLinkAreaSize);
	kPrintf("Data Area Start Address:\t\t %d Sector\n", stManager.dwDataAreaStartAddress);
	kPrintf("Total Cluster Count:\t\t\t %d Cluster\n", stManager.dwTotalClusterCount);
}

static void kCreateFileInRootDirectory(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	int i;
	FILE* pstFile;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	pstFile = fopen(vcFileName, "w");
	if (pstFile == NULL)
	{
		kPrintf("File Create Fail\n");
		return;
	}
	fclose(pstFile);
	kPrintf("File Create Success\n");
}

static void kDeleteFileInRootDirectory(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	DIRECTORYENTRY stEntry;
	int iOffset;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	if (remove(vcFileName) != 0)
	{
		kPrintf("File Not Found or File Opened\n");
		return;
	}
	kPrintf("File Delete Success\n");
}

static void kShowRootDirectory(const char* pcParameterBuffer)
{
	DIR* pstDirectory;
	int i, iCount, iTotalCount;
	int* offset = 0;
	struct dirent* pstEntry;
	char vcBuffer[400];
	char vcTempValue[50];
	DWORD dwTotalByte;
	DWORD dwUsedClusterCount;
	FILESYSTEMMANAGER stManager;

	kGetFileSystemInformation(&stManager);

	pstDirectory = opendir();
	if (pstDirectory == NULL)
	{
		kPrintf("Root Directory Open Fail\n");
		return;
	}

	iTotalCount = 0;
	dwTotalByte = 0;
	dwUsedClusterCount = 0;
	while (1)
	{
		pstEntry = readdir(pstDirectory, offset);
		if (pstEntry == NULL)
		{
			break;
		}
		iTotalCount++;
		dwTotalByte += pstEntry->dwFileSize;

		if (pstEntry->dwFileSize == 0)
		{
			dwUsedClusterCount++;
		}
		else
		{
			dwUsedClusterCount += (pstEntry->dwFileSize +
				(FILESYSTEM_CLUSTERSIZE - 1)) / FILESYSTEM_CLUSTERSIZE;
		}
	}

	rewinddir(pstDirectory);
	iCount = 0;
	while (1)
	{
		pstEntry = readdir(pstDirectory, offset);
		if (pstEntry == NULL)
		{
			break;
		}
		if (pstEntry->bType == FILESYSTEM_TYPE_DIRECTORY)
		{
			kMemSet(vcBuffer, ' ', sizeof(vcBuffer) - 1);
			vcBuffer[sizeof(vcBuffer) - 1] = '\0';
			if (*offset == 0)
			{
				kMemCpy(vcBuffer, ".", 1);
			}
			else if (*offset == 1)
			{
				kMemCpy(vcBuffer, "..", 2);
			}
			else
			{
				kMemCpy(vcBuffer, pstEntry->d_name,
					kStrLen(pstEntry->d_name));
			}
			//kMemCpy( vcTempValue, "Directory" ,sizeof("Directory") );
			kMemCpy(vcBuffer + 30, "Directory", sizeof("Directory"));

			kPrintf("    %s\n", vcBuffer);
		}
		else
		{
			kMemSet(vcBuffer, ' ', sizeof(vcBuffer) - 1);
			vcBuffer[sizeof(vcBuffer) - 1] = '\0';

			kMemCpy(vcBuffer, pstEntry->d_name,
				kStrLen(pstEntry->d_name));

			kSPrintf(vcTempValue, "%d Byte", pstEntry->dwFileSize);
			kMemCpy(vcBuffer + 30, vcTempValue, kStrLen(vcTempValue));

			kSPrintf(vcTempValue, "0x%X Cluster", pstEntry->dwStartClusterIndex);
			kMemCpy(vcBuffer + 55, vcTempValue, kStrLen(vcTempValue) + 1);
			kPrintf("    %s\n", vcBuffer);
		}
		if ((iCount != 0) && ((iCount % 20) == 0))
		{
			kPrintf("Press any key to continue... ('q' is exit) : ");
			if (kGetCh() == 'q')
			{
				kPrintf("\n");
				break;
			}
		}
		iCount++;
	}

	kPrintf("\t\tTotal File Count: %d\n", iTotalCount);
	kPrintf("\t\tTotal File Size: %d KByte (%d Cluster)\n", dwTotalByte,
		dwUsedClusterCount);

	kPrintf("\t\tFree Space: %d KByte (%d Cluster)\n",
		(stManager.dwTotalClusterCount - dwUsedClusterCount) *
		FILESYSTEM_CLUSTERSIZE / 1024, stManager.dwTotalClusterCount -
		dwUsedClusterCount);

	closedir(pstDirectory);
}

static void kWriteDataToFile(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	FILE* fp;
	int iEnterCount;
	BYTE bKey;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	fp = fopen(vcFileName, "w");
	if (fp == NULL)
	{
		kPrintf("%s File Open Fail\n", vcFileName);
		return;
	}

	iEnterCount = 0;
	while (1)
	{
		bKey = kGetCh();
		if (bKey == KEY_ENTER)
		{
			iEnterCount++;
			if (iEnterCount >= 3)
			{
				break;
			}
		}
		else
		{
			iEnterCount = 0;
		}

		kPrintf("%c", bKey);
		if (fwrite(&bKey, 1, 1, fp) != 1)
		{
			kPrintf("File Wirte Fail\n");
			break;
		}
	}

	kPrintf("File Create Success\n");
	fclose(fp);
}

static void kReadDataFromFile(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	FILE* fp;
	int iEnterCount;
	BYTE bKey;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	fp = fopen(vcFileName, "r");
	if (fp == NULL)
	{
		kPrintf("%s File Open Fail\n", vcFileName);
		return;
	}

	iEnterCount = 0;
	while (1)
	{
		if (fread(&bKey, 1, 1, fp) != 1)
		{
			break;
		}
		kPrintf("%c", bKey);

		if (bKey == KEY_ENTER)
		{
			iEnterCount++;

			if ((iEnterCount != 0) && ((iEnterCount % 20) == 0))
			{
				kPrintf("Press any key to continue... ('q' is exit) : ");
				if (kGetCh() == 'q')
				{
					kPrintf("\n");
					break;
				}
				kPrintf("\n");
				iEnterCount = 0;
			}
		}
	}
	fclose(fp);
}

static void kCdDir(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	int i;
	FILE* pstFile;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	if (kMemCmp(vcFileName, "..", 2) == 0)
	{
		if (kCloseDir() == 0)
		{
			kPrintf("RootDir Open Success\n");
			return;
		}
		kPrintf("Dir Fail\n");
	}
	if (kMemCmp(vcFileName, ".", 1) == 0)
	{
		kPrintf("This Dir\n");
		return;
	}

	if (kOpenDir(vcFileName) == -1)
	{
		kPrintf("Dir Fail\n");
		return;
	}

	kPrintf("Dir Open Success\n");
}

static void kMakeDir(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	int i;
	FILE* pstFile;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	if (kCreateDir(vcFileName) == FALSE)
	{
		kPrintf("Dir Fail\n");
		return;
	}

	kPrintf("Dir make Success\n");
}

static void krmDir(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	int i;
	FILE* pstFile;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	if (kRemoveDir(vcFileName) == -1)
	{
		kPrintf("Dir delete Fail\n");
		return;
	}

	kPrintf("Dir delete Success\n");
}


static void move(const char* pcParameterBuffer) {
	copy(pcParameterBuffer);
	PARAMETERLIST stList;
	char vcFileName[50];
	int iLength;
	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	krmDir(vcFileName);
}

static void copy(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	char vcdir[50];
	char vcOlddir[50];
	int iLength;
	int i, dirIndex;
	DWORD dwCluster;
	DIRECTORYENTRY stEntry;
	DIRECTORYENTRY* pstEmptyEntry;
	int iFileOffset, iFreeIndex;
	FILE* pstFile;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';
	iLength = kGetNextParameter(&stList, vcdir);
	vcdir[iLength] = '\0';

	if ((iLength > (FILESYSTEM_MAXFILENAMELENGTH - 1)) || (iLength == 0))
	{
		kPrintf("Too Long or Too Short File Name\n");
		return;
	}

	iFileOffset = kFindDirectoryEntry(vcFileName, &stEntry);
	

	if (kMemCmp(vcdir, "..", 2) == 0)
	{
		dirIndex = gs_stFileSystemManager.pstDirIndex;
		if (kCloseDir() == 0)
		{
			iFreeIndex = kFindFreeDirectoryEntry();
			kPrintf("kGetDirectoryEntryData complete\n");

			if (kSetDirectoryEntryData(gs_stFileSystemManager.pstDirIndex, iFreeIndex, &stEntry) == FALSE) {
				kPrintf("kSetDirectoryEntryData fail\n");
				return;
			}
			kPrintf("kSetDirectoryEntryData complete\n");
			int destination = kFindDirectoryEntryByIndex(dirIndex, &stEntry);
			kGetDirectoryEntryData(gs_stFileSystemManager.pstDirIndex, destination, &stEntry);
			kCdDir(stEntry.vcFileName);
			kPrintf("Copy Success\n");
			return;
		}
		kPrintf("Copy fail\n");
		return;
	}
	else
	{
		kCdDir(vcdir);
		kPrintf("cdDir complete\n");

		iFreeIndex = kFindFreeDirectoryEntry();
		kPrintf("kGetDirectoryEntryData complete\n");

		if (kSetDirectoryEntryData(gs_stFileSystemManager.pstDirIndex, iFreeIndex, &stEntry) == FALSE) {
			kPrintf("kSetDirectoryEntryData fail\n");
			return;
		}
		kPrintf("kSetDirectoryEntryData complete\n");

		kCdDir("..");
		kPrintf("kCloseDir complete\n");

		return;
	}
	kPrintf("Copy fail\n");

	return;
}

static void rename(const char* pcParameterBuffer)
{
	PARAMETERLIST stList;
	char vcFileName[50];
	char vcNewFileName[50];
	int iLength, iNewNameLength;

	kInitializeParameter(&stList, pcParameterBuffer);
	iLength = kGetNextParameter(&stList, vcFileName);
	vcFileName[iLength] = '\0';

	iNewNameLength = kGetNextParameter(&stList, vcNewFileName);
	vcNewFileName[iNewNameLength] = '\0';

	if (kRenameFile(vcFileName, vcNewFileName) == 0) {
		kPrintf("Success rename\n");
	}
	else {
		kPrintf("Rename fail\n");
	}
	return;
}

static void kTestFileIO(const char* pcParameterBuffer)
{
	FILE* pstFile;
	BYTE* pbBuffer;
	int i;
	int j;
	DWORD dwRandomOffset;
	DWORD dwByteCount;
	BYTE vbTempBuffer[1024];
	DWORD dwMaxFileSize;

	kPrintf("================== File I/O Function Test ==================\n");

	// 4Mbyte의 버퍼 할당
	dwMaxFileSize = 4 * 1024 * 1024;
	pbBuffer = kMalloc(dwMaxFileSize);
	if (pbBuffer == NULL)
	{
		kPrintf("Memory Allocation Fail\n");
		return;
	}
	// 테스트용 파일을 삭제
	remove("testfileio.bin");

	//==========================================================================
	// 파일 열기 테스트
	//==========================================================================
	kPrintf("1. File Open Fail Test...");
	// r 옵션은 파일을 생성하지 않으므로, 테스트 파일이 없는 경우 NULL이 되어야 함
	pstFile = fopen("testfileio.bin", "r");
	if (pstFile == NULL)
	{
		kPrintf("[Pass]\n");
	}
	else
	{
		kPrintf("[Fail]\n");
		fclose(pstFile);
	}

	//==========================================================================
	// 파일 생성 테스트
	//==========================================================================
	kPrintf("2. File Create Test...");
	// w 옵션은 파일을 생성하므로, 정상적으로 핸들이 반환되어야함
	pstFile = fopen("testfileio.bin", "w");
	if (pstFile != NULL)
	{
		kPrintf("[Pass]\n");
		kPrintf("    File Handle [0x%Q]\n", pstFile);
	}
	else
	{
		kPrintf("[Fail]\n");
	}

	//==========================================================================
	// 순차적인 영역 쓰기 테스트
	//==========================================================================
	kPrintf("3. Sequential Write Test(Cluster Size)...");
	// 열린 핸들을 가지고 쓰기 수행
	for (i = 0; i < 100; i++)
	{
		kMemSet(pbBuffer, i, FILESYSTEM_CLUSTERSIZE);
		if (fwrite(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
			FILESYSTEM_CLUSTERSIZE)
		{
			kPrintf("[Fail]\n");
			kPrintf("    %d Cluster Error\n", i);
			break;
		}
	}
	if (i >= 100)
	{
		kPrintf("[Pass]\n");
	}

	//==========================================================================
	// 순차적인 영역 읽기 테스트
	//==========================================================================
	kPrintf("4. Sequential Read And Verify Test(Cluster Size)...");
	// 파일의 처음으로 이동
	fseek(pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_END);

	// 열린 핸들을 가지고 읽기 수행 후, 데이터 검증
	for (i = 0; i < 100; i++)
	{
		// 파일을 읽음
		if (fread(pbBuffer, 1, FILESYSTEM_CLUSTERSIZE, pstFile) !=
			FILESYSTEM_CLUSTERSIZE)
		{
			kPrintf("[Fail]\n");
			return;
		}

		// 데이터 검사
		for (j = 0; j < FILESYSTEM_CLUSTERSIZE; j++)
		{
			if (pbBuffer[j] != (BYTE)i)
			{
				kPrintf("[Fail]\n");
				kPrintf("    %d Cluster Error. [%X] != [%X]\n", i, pbBuffer[j],
					(BYTE)i);
				break;
			}
		}
	}
	if (i >= 100)
	{
		kPrintf("[Pass]\n");
	}

	//==========================================================================
	// 임의의 영역 쓰기 테스트
	//==========================================================================
	kPrintf("5. Random Write Test...\n");

	// 버퍼를 모두 0으로 채움
	kMemSet(pbBuffer, 0, dwMaxFileSize);
	// 여기 저기에 옮겨다니면서 데이터를 쓰고 검증
	// 파일의 내용을 읽어서 버퍼로 복사
	fseek(pstFile, -100 * FILESYSTEM_CLUSTERSIZE, SEEK_CUR);
	fread(pbBuffer, 1, dwMaxFileSize, pstFile);

	// 임의의 위치로 옮기면서 데이터를 파일과 버퍼에 동시에 씀
	for (i = 0; i < 100; i++)
	{
		dwByteCount = (kRandom() % (sizeof(vbTempBuffer) - 1)) + 1;
		dwRandomOffset = kRandom() % (dwMaxFileSize - dwByteCount);

		kPrintf("    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
			dwByteCount);

		// 파일 포인터를 이동
		fseek(pstFile, dwRandomOffset, SEEK_SET);
		kMemSet(vbTempBuffer, i, dwByteCount);

		// 데이터를 씀
		if (fwrite(vbTempBuffer, 1, dwByteCount, pstFile) != dwByteCount)
		{
			kPrintf("[Fail]\n");
			break;
		}
		else
		{
			kPrintf("[Pass]\n");
		}

		kMemSet(pbBuffer + dwRandomOffset, i, dwByteCount);
	}

	// 맨 마지막으로 이동하여 1바이트를 써서 파일의 크기를 4Mbyte로 만듦
	fseek(pstFile, dwMaxFileSize - 1, SEEK_SET);
	fwrite(&i, 1, 1, pstFile);
	pbBuffer[dwMaxFileSize - 1] = (BYTE)i;

	//==========================================================================
	// 임의의 영역 읽기 테스트
	//==========================================================================
	kPrintf("6. Random Read And Verify Test...\n");
	// 임의의 위치로 옮기면서 파일에서 데이터를 읽어 버퍼의 내용과 비교
	for (i = 0; i < 100; i++)
	{
		dwByteCount = (kRandom() % (sizeof(vbTempBuffer) - 1)) + 1;
		dwRandomOffset = kRandom() % ((dwMaxFileSize)-dwByteCount);

		kPrintf("    [%d] Offset [%d] Byte [%d]...", i, dwRandomOffset,
			dwByteCount);

		// 파일 포인터를 이동
		fseek(pstFile, dwRandomOffset, SEEK_SET);

		// 데이터 읽음
		if (fread(vbTempBuffer, 1, dwByteCount, pstFile) != dwByteCount)
		{
			kPrintf("[Fail]\n");
			kPrintf("    Read Fail\n", dwRandomOffset);
			break;
		}

		// 버퍼와 비교
		if (kMemCmp(pbBuffer + dwRandomOffset, vbTempBuffer, dwByteCount)
			!= 0)
		{
			kPrintf("[Fail]\n");
			kPrintf("    Compare Fail\n", dwRandomOffset);
			break;
		}

		kPrintf("[Pass]\n");
	}

	//==========================================================================
	// 다시 순차적인 영역 읽기 테스트
	//==========================================================================
	kPrintf("7. Sequential Write, Read And Verify Test(1024 Byte)...\n");
	// 파일의 처음으로 이동
	fseek(pstFile, -dwMaxFileSize, SEEK_CUR);

	// 열린 핸들을 가지고 쓰기 수행. 앞부분에서 2Mbyte만 씀
	for (i = 0; i < (2 * 1024 * 1024 / 1024); i++)
	{
		kPrintf("    [%d] Offset [%d] Byte [%d] Write...", i, i * 1024, 1024);

		// 1024 바이트씩 파일을 씀
		if (fwrite(pbBuffer + (i * 1024), 1, 1024, pstFile) != 1024)
		{
			kPrintf("[Fail]\n");
			return;
		}
		else
		{
			kPrintf("[Pass]\n");
		}
	}

	// 파일의 처음으로 이동
	fseek(pstFile, -dwMaxFileSize, SEEK_SET);

	// 열린 핸들을 가지고 읽기 수행 후 데이터 검증. Random Write로 데이터가 잘못
	// 저장될 수 있으므로 검증은 4Mbyte 전체를 대상으로 함
	for (i = 0; i < (dwMaxFileSize / 1024); i++)
	{
		// 데이터 검사
		kPrintf("    [%d] Offset [%d] Byte [%d] Read And Verify...", i,
			i * 1024, 1024);

		// 1024 바이트씩 파일을 읽음
		if (fread(vbTempBuffer, 1, 1024, pstFile) != 1024)
		{
			kPrintf("[Fail]\n");
			return;
		}

		if (kMemCmp(pbBuffer + (i * 1024), vbTempBuffer, 1024) != 0)
		{
			kPrintf("[Fail]\n");
			break;
		}
		else
		{
			kPrintf("[Pass]\n");
		}
	}

	//==========================================================================
	// 파일 삭제 실패 테스트
	//==========================================================================
	kPrintf("8. File Delete Fail Test...");
	// 파일이 열려있는 상태이므로 파일을 지우려고 하면 실패해야 함
	if (remove("testfileio.bin") != 0)
	{
		kPrintf("[Pass]\n");
	}
	else
	{
		kPrintf("[Fail]\n");
	}

	//==========================================================================
	// 파일 닫기 테스트
	//==========================================================================
	kPrintf("9. File Close Test...");
	// 파일이 정상적으로 닫혀야 함
	if (fclose(pstFile) == 0)
	{
		kPrintf("[Pass]\n");
	}
	else
	{
		kPrintf("[Fail]\n");
	}

	//==========================================================================
	// 파일 삭제 테스트
	//==========================================================================
	kPrintf("10. File Delete Test...");
	// 파일이 닫혔으므로 정상적으로 지워져야 함
	if (remove("testfileio.bin") == 0)
	{
		kPrintf("[Pass]\n");
	}
	else
	{
		kPrintf("[Fail]\n");
	}

	// 메모리를 해제
	kFree(pbBuffer);
}