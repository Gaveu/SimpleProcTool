#include"appDef.h"

//ȫ�ֵ��������,����ͷ�ڵ�
VOID EmptyGlProc()
{
	PMYPROCESS pTmp = glProcListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glProcListEntry.next);
		glProcListEntry.next = pTmp;
	}
}
VOID EmptyGlThd()
{
	PMYTHREAD pTmp = glThdListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glThdListEntry.next);
		glThdListEntry.next = pTmp;
	}
}
VOID EmptyGlMod()
{
	PMYMODINFO pTmp = glModListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		free(glModListEntry.next->DllPath.Buffer);
		free(glModListEntry.next);
		glModListEntry.next = pTmp;
	}
}




/*������ͨ�ź���*/
//�򿪲����ӵ�ָ�����豸����
NTSTATUS Init()
{
	FILETIME ftIdle;
	FILETIME ftKernel;
	FILETIME ftUser;

	//�������������豸����
	hDevice = CreateFile(
		TEXT("\\\\.\\ProcToolDev"),
		GENERIC_ALL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Init:File Create Failed!\n");
		return STATUS_INVALID_HANDLE;
	}

	//��ʼ��ͨ��ȫ�ֻ�����
	glBuf = (PCHAR)malloc(DE_MAX_BUF_LEN*sizeof(CHAR));
	if (!glBuf)
	{
		printf("Init:Memory Allocate Failed!\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//��ʼ��CPU�����ʵ����ȫ�ֱ���
	GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
	preFtIdle = FileTimeToDouble(&ftIdle);
	preFtUser = FileTimeToDouble(&ftUser);
	preFtKernel = FileTimeToDouble(&ftKernel);

	return STATUS_SUCCESS;
}

//�������ʱ��ִ����Դ�ͷŵȲ���
NTSTATUS End()
{
	CloseHandle(hDevice);
	free(glBuf);
	return STATUS_SUCCESS;
}

//����ö��ͨ�ź���
//���豸�����͹�����EnumProc
//���������㴫������Ϣ���Ͻ�����ʱȫ�ֽ�������
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS PsEnum()
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	ULONG cpuUsage = 0;
	SIZE_T memUsed = 0;
	SIZE_T vmemUsed = 0;

	NTSTATUS status;
	PMYPROCESS pMyProcTmp = NULL;


	//���豸�����͹�����EnumProc
	status = DeviceIoControl(
		hDevice,
		EnumProc,
		NULL,
		0,
		glBuf,
		DE_MAX_BUF_LEN * sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!status)
	{
		status = GetLastError();
		printf("PsEnum():EnumPs Failed!0x%x\n", status);
		EmptyGlProc();
		return status;
	}

	printf("PsEnum():wlen %d\n", wLen);

	//���������㴫������Ϣ���Ͻ�����ʱȫ�ֽ�������
	while (rLen < wLen)
	{
		pMyProcTmp = (PMYPROCESS)malloc(sizeof(MYPROCESS));
		if (!pMyProcTmp)
		{
			printf("PsEnum():Memory Insufficient!\n");
			EmptyGlProc();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		memcpy_s(
			pMyProcTmp,
			sizeof(MYPROCESS),
			glBuf + rLen,
			sizeof(MYPROCESS)
		);
		rLen += sizeof(MYPROCESS);

		pMyProcTmp->next = glProcListEntry.next;
		glProcListEntry.next = pMyProcTmp;
	}

	//������Ϻ����ȫ������������������Ϣ�����ٽ��
	pMyProcTmp = glProcListEntry.next;
	while (pMyProcTmp)
	{
		//status = PsGetUsage(
		//	(HANDLE)pMyProcTmp->ulPid,
		//	&cpuUsage,
		//	&memUsed,
		//	&vmemUsed
		//);
		//printf("%s\n", pMyProcTmp->ImageFileName);
		//printf("pid:%5d\tppid:%5d\t%u\t%u\\%u\n\n",
		//	pMyProcTmp->ulPid,
		//	pMyProcTmp->ulPpid,
		//	cpuUsage,
		//	memUsed,
		//	vmemUsed);

		printf("%s\n", pMyProcTmp->ImageFileName);
		printf("pid:%5u\tppid:%5u\n\n",
			pMyProcTmp->ulPid,
			pMyProcTmp->ulPpid
		);

		pMyProcTmp = pMyProcTmp->next;
		free(glProcListEntry.next);
		glProcListEntry.next = pMyProcTmp;
	}

	return STATUS_SUCCESS;
}

//�߳�ö��ͨ�ź���
//���豸�����͹�����EnumThd��ָ���Ľ���ID
//���������㴫������Ϣ���Ͻ�����ʱȫ���߳�����
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS ThdEnumByPid(IN HANDLE pid)
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	NTSTATUS status;
	PMYTHREAD pMyThdTmp = NULL;

	//���豸�����͹�����EnumThd��ָ���Ľ���ID
	status = DeviceIoControl(
		hDevice,
		EnumThd,
		&pid,
		sizeof(HANDLE),
		glBuf,
		DE_MAX_BUF_LEN * sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("ThdEnumByPid():EnumThd Failed!0x%x\n", status);
		EmptyGlThd();
		return status;
	}

	//���������㴫������Ϣ���Ͻ�����ʱȫ���߳�����
	while (rLen < wLen)
	{
		pMyThdTmp = (PMYTHREAD)malloc(sizeof(MYTHREAD));
		if (!pMyThdTmp)
		{
			printf("ThdEnumByPid():Memory Insufficient!\n");
			EmptyGlThd();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		memcpy_s(
			pMyThdTmp,
			sizeof(MYTHREAD),
			glBuf + rLen,
			sizeof(MYTHREAD)
		);
		rLen += sizeof(MYTHREAD);
		
		pMyThdTmp->next = glThdListEntry.next;
		glThdListEntry.next = pMyThdTmp;
	}

	//������Ϻ����ȫ������������������Ϣ�����ٽ��
	pMyThdTmp = glThdListEntry.next;
	while (pMyThdTmp)
	{
		printf("Tid:%5d\tPrioty:%3d\tState:%3d\n",
			pMyThdTmp->ulTid,
			pMyThdTmp->cPrioty,
			pMyThdTmp->ucState
		);
		pMyThdTmp = pMyThdTmp->next;
		free(glThdListEntry.next);
		glThdListEntry.next = pMyThdTmp;
	}
	return STATUS_SUCCESS;
}

//����ģ��ö��ͨ�ź���
//���豸�����͹�����EnumPMod��ָ���Ľ���ID
//���������㴫������Ϣ���Ͻ�����ʱȫ��ģ������
//������Ϻ����ȫ������������������Ϣ�����ٽ��
NTSTATUS ModEnumByPid(IN HANDLE pid)
{
	ULONG rLen = 0;
	ULONG wLen = 0;
	PMYMODINFO pMyModInfo = NULL;
	NTSTATUS status;

	//���豸�����͹�����EnumPMod��ָ���Ľ���ID
	status = DeviceIoControl(
		hDevice,
		EnumPMod,
		&pid,
		sizeof(HANDLE),
		glBuf,
		DE_MAX_BUF_LEN*sizeof(CHAR),
		&wLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("ModEnumByPid():EnumMod Failed!0x%x\n",status);
		EmptyGlMod();
		return status;
	}

	//���������㴫������Ϣ���Ͻ�����ʱȫ��ģ������
	while (rLen < wLen)
	{
		pMyModInfo = (PMYMODINFO)malloc(sizeof(MYMODINFO));
		if (!pMyModInfo)
		{
			printf("ModEnumByPid():Memory Insufficient!\n");
			EmptyGlMod();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		//������Ϣ�ṹ��
		memcpy_s(
			pMyModInfo,
			sizeof(MYMODINFO),
			glBuf + rLen,
			sizeof(MYMODINFO)
		);
		rLen += sizeof(MYMODINFO);
		pMyModInfo->DllPath.Buffer = (PWCH)malloc(sizeof(WCHAR)*DE_MAX_PATH_LEN);
		if (!pMyModInfo->DllPath.Buffer)
		{
			printf("ModEnumByPid():Memory Insufficient!\n");
			EmptyGlMod();
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		//�������ַ�����
		memcpy_s(
			pMyModInfo->DllPath.Buffer,
			sizeof(WCHAR)*DE_MAX_PATH_LEN,
			glBuf + rLen,
			sizeof(WCHAR)*DE_MAX_PATH_LEN
		);
		rLen += sizeof(WCHAR)*DE_MAX_PATH_LEN;

		pMyModInfo->next = glModListEntry.next;
		glModListEntry.next = pMyModInfo;
	}

	//������Ϻ����ȫ������������������Ϣ�����ٽ��
	pMyModInfo = glModListEntry.next;
	while (pMyModInfo)
	{
		wprintf(L"%ws\n\n", pMyModInfo->DllPath.Buffer);
		wprintf(L"Base:0x%p\tSize:0x%x\n",
			pMyModInfo->ulStartAddr,
			pMyModInfo->ulSize
		);

		free(pMyModInfo->DllPath.Buffer);
		pMyModInfo = pMyModInfo->next;
		free(glModListEntry.next);
		glModListEntry.next = pMyModInfo;
	}
	printf("wLen:0x%x\trLen:0x%x\n", wLen, rLen);
	return STATUS_SUCCESS;
}

//������ͣͨ�ź���
//���豸�����͹�����SuspendProc��ָ���Ľ���ID
NTSTATUS PsSuspendByPid(IN HANDLE pid)
{
	NTSTATUS status;
	ULONG retLen;
	status = DeviceIoControl(
		hDevice,
		SuspendProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsSuspendByPid():PsSuspend Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Suspend!\n", (ULONG)pid);
	}
	return status;
}

//���ָ̻�ͨ�ź���
//���豸�����͹�����ResumeProc��ָ���Ľ���ID
NTSTATUS PsResumeByPid(IN HANDLE pid)
{
	ULONG retLen;
	NTSTATUS status;
	status = DeviceIoControl(
		hDevice,
		ResumeProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsResumeByPid():PsResume Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Resume!\n", (ULONG)pid);
	}
	return status;
}

//���̽���ͨ�ź���
//���豸�����͹�����TerminateProc��ָ���Ľ���ID
NTSTATUS PsTerminateByPid(IN HANDLE pid)
{
	ULONG retLen;
	NTSTATUS status;
	status = DeviceIoControl(
		hDevice,
		TerminateProc,
		&pid,
		sizeof(HANDLE),
		NULL,
		0,
		&retLen,
		NULL
	);
	if (!(status))
	{
		status = GetLastError();
		printf("PsTerminateByPid():PsTerminate Failed!0x%x\n", status);
	}
	else
	{
		printf("pid:%d Terminate!\n", (ULONG)pid);
	}
	return status;
}



/*�û������ݺ���*/
//ת���ļ�ʱ��Ϊdouble����
DOUBLE FileTimeToDouble(FILETIME* pFiletime)
{
	return (double)((*pFiletime).dwHighDateTime * 4.294967296E9) + (double)(*pFiletime).dwLowDateTime;
}

/*
typedef struct _PROCESS_MEMORY_COUNTERS {
	DWORD cb;
	DWORD PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;
typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;
*/

//��CPUռ�������ڴ�ռ��
NTSTATUS PsGetUsage()
{
	FILETIME ftIdle;
	FILETIME ftKernel;
	FILETIME ftUser;
	DOUBLE nowFtIdle;
	DOUBLE nowFtKernel;
	DOUBLE nowFtUser;
	MEMORYSTATUSEX msex;
	int nCpuUsage = -1;
	DWORD dwMemUsage = -1;

	//��ȡcpu������
	GetSystemTimes(&ftIdle, &ftKernel, &ftUser);
	nowFtIdle = FileTimeToDouble(&ftIdle);
	nowFtKernel = FileTimeToDouble(&ftKernel);
	nowFtUser = FileTimeToDouble(&ftUser);
	nCpuUsage = (int)(100.0 - (nowFtIdle - preFtIdle) / (nowFtKernel - preFtKernel + nowFtUser - preFtUser)*100.0);
	preFtIdle = nowFtIdle;
	preFtKernel = nowFtKernel;
	preFtUser = nowFtUser;

	//��ȡ���ڴ�ռ��
	msex.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&msex);
	printf("CPUռ����: %d\n", nCpuUsage);
	printf("�ڴ�ռ����: %ld\n", msex.dwMemoryLoad);
	printf("���������ڴ�/�������ڴ�: %lluB/%lluB\t%d%%\n",
		msex.ullAvailPhys,
		msex.ullTotalPhys,
		(int)((double)msex.ullAvailPhys / (double)msex.ullTotalPhys*100.0)
	);
	printf("����ҳ���ڴ�/��ҳ���ڴ�: %lluB/%lluB\t%d%%\n",
		msex.ullAvailPageFile,
		msex.ullTotalPageFile,
		(int)((double)msex.ullAvailPageFile / (double)msex.ullTotalPageFile*100.0)
	);
	printf("�����û�̬�ڴ�/���û�̬�ڴ�: %lluB/%lluB\t%d%%\n",
		msex.ullAvailVirtual,
		msex.ullTotalVirtual,
		(int)((double)msex.ullAvailVirtual / (double)msex.ullTotalVirtual*100.0)
	);
	printf("�����û�̬��չ�ڴ�: %lluB\n\n", msex.ullAvailExtendedVirtual);

	return STATUS_SUCCESS;
}


//��ȡϵͳ��Ϣ
void getSysInfo()
{
	SYSTEM_INFO sysInfo; //�ýṹ������˵�ǰ���������Ϣ�����������ϵ�ṹ�����봦���������͡�ϵͳ�����봦������������ҳ��Ĵ�С�Լ�������Ϣ��
	OSVERSIONINFOEX osvi;
	ULONG cpuUsage;
	SIZE_T mem;
	SIZE_T vMem;
	HANDLE pid = (HANDLE)GetCurrentProcessId();
	GetSystemInfo(&sysInfo);
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	PsGetUsage(pid, &cpuUsage, &mem, &vMem);

	if (GetVersionEx((LPOSVERSIONINFOA)&osvi))
	{
		printf("����ϵͳ�汾 :  %u.%u.%u\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
		printf("Service Pack :  %u.%u\n", osvi.wServicePackMajor, osvi.wServicePackMinor);
	}

	if (STATUS_SUCCESS == PsGetUsage(pid, &cpuUsage, &mem, &vMem))
	{
		printf("CPUռ���� ��%u\n", cpuUsage);
		printf("�ڴ�ռ�� �� 0x%x\n", mem);
		printf("�����ڴ�ռ�� �� 0x%x\n", vMem);
	}

	printf("�������ܹ� :  %u\n", sysInfo.wProcessorArchitecture);
	printf("���������� :  %u\n", sysInfo.wProcessorLevel);
	printf("�������汾 :  %u\n", sysInfo.wProcessorRevision);
	printf("���������� :  %u\n", sysInfo.dwActiveProcessorMask);
	printf("���������� :  %u\n", sysInfo.dwNumberOfProcessors);
	printf("���������� :  %u\n", sysInfo.dwProcessorType);
	printf("ҳ���С :   %u\n", sysInfo.dwPageSize);
	printf("Ӧ�ó�����С��ַ : 0x%p\n", sysInfo.lpMinimumApplicationAddress);
	printf("Ӧ�ó�������ַ : 0x%p\n", sysInfo.lpMaximumApplicationAddress);
	printf("�����ڴ�������� : %u\n", sysInfo.dwAllocationGranularity);
	printf("OemId :    %u\n", sysInfo.dwOemId);
	printf("wReserved :   %u\n", sysInfo.wReserved);
}

/*
typedef struct _MEMORYSTATUSEX {
	DWORD dwLength;
	DWORD dwMemoryLoad;			//0~100֮���ֵ������ָʾ��ǰϵͳ�ڴ��ʹ����
	DWORDLONG ullTotalPhys;		//�������ڴ��С���ֽڵ�λ
	DWORDLONG ullAvailPhys;		//���������ڴ��С���ֽڵ�λ
	DWORDLONG ullTotalPageFile;	//ҳ���ļ����ֽ���
	DWORDLONG ullAvailPageFile;	//ҳ���ļ������ֽ���
	DWORDLONG ullTotalVirtual;	//�û�̬�����ַ�ռ��С���ֽڵ�λ
	DWORDLONG ullAvailVirtual;	//�û�̬�����ַ�ռ��ʵ�ʿ��ô�С���ֽڵ�λ
	DWORDLONG ullAvailExtendedVirtual;	//�û�̬�����ַ��չ�ռ��ʵ�ʿ��ô�С���ֽڵ�λ
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;
*/

/*
typedef struct _SYSTEM_INFO {
	union {
		DWORD dwOemId;          // Obsolete field...do not use
		struct {
			WORD wProcessorArchitecture;
			WORD wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	DWORD dwPageSize;
	LPVOID lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress;
	DWORD_PTR dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;
*/

/*
typedef struct _OSVERSIONINFOEXA {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
	WORD   wServicePackMajor;
	WORD   wServicePackMinor;
	WORD   wSuiteMask;
	BYTE  wProductType;
	BYTE  wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
*/