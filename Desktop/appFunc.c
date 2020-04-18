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
	glBuf = (PCHAR)malloc(DE_MAX_BUF_LEN*sizeof(CHAR));
	if (!glBuf)
	{
		printf("Init:Memory Allocate Failed!\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
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
		printf("pid:%5d\tppid:%5d\t%s\n",
			pMyProcTmp->ulPid,
			pMyProcTmp->ulPpid,
			pMyProcTmp->ImageFileName);
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