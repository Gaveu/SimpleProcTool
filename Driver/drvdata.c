#include"drvdef.h"

/*���̹��ܺ���*/
//����ö��
//���ڽ���˫����ı������Ͽ죬�����ڲ���Ӳ������м���������
VOID PsEnum1()
{
	PEPROCESS pEproc = NULL;		//��ȡ���̵�ַ
	PEPROCESS pFirstProc = NULL;	//����EPROCESS����˫����ͷ�ڵ�
	PLIST_ENTRY pLe = NULL;
	PCHAR pProcName = NULL;			//�������ַ���ָ��
	ULONG ulPid = 0;				//����ID
	PMYPROCESS pTmpMyProc = NULL;	//��ʱȫ�ֵ�����ָ��

	//��ȡ��ǰ����EPROCESS
	pEproc = PsGetCurrentProcess();
	if (!pEproc)
	{
		KdPrint(("PsEnum1:GetCurrentProcess Failed!\n"));
		return;
	}
	pFirstProc = pEproc;
	//KdBreakPoint();
	__try {
		EmptyGlProc();
		do
		{
			pProcName = PsGetProcessImageFileName(pEproc);
			ulPid = *(ULONG *)((PCHAR)pEproc + 0xb4);
			if(ulPid <= 30000)
			{
				//KdPrint(("PID:%d\tparentID:%d\t%s\n",
				//	ulPid,
				//	(ULONG)PsGetProcessInheritedFromUniqueProcessId(pEproc),
				//	pProcName));
				
				//��ʱȫ�ֵ�����Ľ�㿽������
				pTmpMyProc = (PMYPROCESS)MemAlloc(sizeof(MYPROCESS));
				if (!pTmpMyProc)
				{
					KdPrint(("PsEnum1:Memory Insufficient!\n"));
					return;
				}
				pTmpMyProc->ulPid = ulPid;
				pTmpMyProc->ulPpid = (ULONG)PsGetProcessInheritedFromUniqueProcessId(pEproc);
				strcpy_s(
					pTmpMyProc->ImageFileName,
					DE_MAX_FILE_NAME_LEN*sizeof(CHAR),
					pProcName
				);

				pTmpMyProc->ImageFileName[DE_MAX_FILE_NAME_LEN - 1] = '\0';
				//��ʱ���ͷ����ȫ�ֵ�������
				pTmpMyProc->next = glProcListEntry.next;
				glProcListEntry.next = pTmpMyProc;
			}
			//KdBreakPoint();
			pLe = (PLIST_ENTRY)((PCHAR)pEproc + 0xb8);
			pLe = pLe->Flink;
			pEproc = (PEPROCESS)((PCHAR)pLe - 0xb8);
		} while (pEproc != pFirstProc);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MemFree(pTmpMyProc);
		KdPrint(("PsEnum1:Process Enum Failed!0x%x\n", GetExceptionCode()));
	}
	//KdBreakPoint();
}

//��ͨ�ã��޼��������⣬��Ϊ���Ա���
VOID PsEnum2()
{
	ULONG ulPid = 0;
	PEPROCESS pEproc = NULL;
	PUCHAR pProcName = NULL;
	PMYPROCESS pTmpMyProc = NULL;	//��ʱȫ�ֵ�����ָ��
	NTSTATUS status;

	__try {
		EmptyGlProc();
		for (ulPid = 4; ulPid < 20000; ulPid += 4)
		{
			status = PsLookupProcessByProcessId((HANDLE)ulPid, &pEproc);
			if (NT_SUCCESS(status))
			{
				pProcName = PsGetProcessImageFileName(pEproc);

				//��ʱȫ�ֵ�����Ľ�㿽������
				pTmpMyProc = (PMYPROCESS)MemAlloc(sizeof(MYPROCESS));
				if (!pTmpMyProc)
				{
					KdPrint(("PsEnum2:Memory Insufficient!\n"));
					return;
				}
				pTmpMyProc->ulPid = ulPid;
				pTmpMyProc->ulPpid = (ULONG)PsGetProcessInheritedFromUniqueProcessId(pEproc);
				strcpy_s(
					pTmpMyProc->ImageFileName,
					DE_MAX_FILE_NAME_LEN * sizeof(CHAR),
					pProcName
				);
				
				pTmpMyProc->ImageFileName[DE_MAX_FILE_NAME_LEN - 1] = '\0';
				//��ʱ���ͷ����ȫ�ֵ�������
				pTmpMyProc->next = glProcListEntry.next;
				glProcListEntry.next = pTmpMyProc;

				//ThEnum3(pEproc);
			}
			ObDereferenceObject(pEproc);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MemFree(pTmpMyProc);
		ObDereferenceObject(pEproc);
		KdPrint(("PsEnum2:Process Enum Failed!0x%x\n", GetExceptionCode()));
	}
	
	//KdPrint(("---------------------\n"));
}

//������ͣ
//��������pid����ͣ��Ӧ���̵�ִ��
NTSTATUS PsSuspend(IN HANDLE hPid)
{
	NTSTATUS status;
	PEPROCESS pEproc = NULL;
	status = PsLookupProcessByProcessId(hPid, &pEproc);
	//KdBreakPoint();
	if (NT_SUCCESS(status))
	{
		//xp->Nt.5�汾����Ӳ�������뷽ʽִ�к���
		if (SharedUserData->NtMajorVersion == 5)
		{
			status = pXpPsSuspendProcess(pEproc);
		}
		else
		{
			status = PsSuspendProcess(pEproc);
		}
		if (NT_SUCCESS(status))
		{
			KdPrint(("pid:%d\t%s\t Suspend!\n", (ULONG)hPid, PsGetProcessImageFileName(pEproc)));
		}
	}
	return status;
}

//���ָ̻�
//��������pid���ָ���Ӧ���̵�ִ��
NTSTATUS PsResume(IN HANDLE hPid)
{
	NTSTATUS status;
	PEPROCESS pEproc = NULL;
	status = PsLookupProcessByProcessId(hPid, &pEproc);
	//KdBreakPoint();
	if (NT_SUCCESS(status))
	{
		//xp->Nt.5�汾����Ӳ�������뷽ʽִ�к���
		if (SharedUserData->NtMajorVersion == 5)
		{
			status = pXpPsResumeProcess(pEproc);
		}
		else
		{
			status = PsResumeProcess(pEproc);
		}
		if (NT_SUCCESS(status))
		{
			KdPrint(("pid:%d\t%s\t Resume!\n", (ULONG)hPid, PsGetProcessImageFileName(pEproc)));
		}
	}

	return status;
}

//������ֹ
//��������pid����ֹ��Ӧ���̵�ִ��
NTSTATUS PsTerminate(IN HANDLE hPid)
{
	HANDLE hProc = NULL;
	CLIENT_ID clientId = { 0 };
	NTSTATUS status;
	OBJECT_ATTRIBUTES oa;
	clientId.UniqueProcess = hPid;
	InitializeObjectAttributes(
		&oa,
		NULL,
		OBJ_KERNEL_HANDLE,
		NULL,
		NULL
	);
	//KdBreakPoint();
	status = ZwOpenProcess(
		&hProc,
		PROCESS_ALL_ACCESS,
		&oa,
		&clientId
	);
	if (NT_SUCCESS(status))
	{
		status = ZwTerminateProcess(hProc, 0);
		if (NT_SUCCESS(status))
		{
			KdPrint(("PID:%d\tTerminate Success!\n", hPid));
		}
		else
		{
			KdPrint(("PsTerminate:TerminateProcess Failed!0x%x\n", status));
		}
	}
	else
	{
		KdPrint(("OpenProcess Failed!0x%x\n", status));
	}
	ZwClose(hProc);
	return status;
}




/*�̹߳��ܺ���*/
//�߳�ö��
VOID ThEnum1()
{
	PETHREAD pEth = NULL;
	//PEPROCESS pEproc = NULL;
	NTSTATUS status;
	CHAR cPriority;
	UCHAR ucState;
	ULONG ulThreadId = 0;
	//ULONG ulPid = 0;
	//PCHAR pProcName = NULL;
	PMYTHREAD pMyThdTmp = NULL;

	//KdPrint(("ThEnum1\n"));
	EmptyGlThd();
	for (ulThreadId = 0; ulThreadId < 80000; ulThreadId += 4)
	{
		pMyThdTmp = (PMYTHREAD)MemAlloc(sizeof(MYTHREAD));
		if (!pMyThdTmp)
		{
			KdPrint(("ThEnum1:Memory Insufficient!\n"));
			return;
		}

		status = PsLookupThreadByThreadId((HANDLE)ulThreadId, &pEth);
		if (NT_SUCCESS(status))
		{
			//pEproc = (PEPROCESS)(*(PEPROCESS *)((PCHAR)pEth + 0x150));
			//ulPid = (ULONG)PsGetProcessId(pEproc);
			//pProcName = PsGetProcessImageFileName(pEproc);
			__try
			{
				cPriority = *(CHAR *)((PCHAR)pEth + 0x57);
				ucState = *(UCHAR *)((PCHAR)pEth + 0x68);

				pMyThdTmp->cPrioty = cPriority;
				pMyThdTmp->ulTid = ulThreadId;
				pMyThdTmp->ucState = ucState;
				pMyThdTmp->next = glThdListEntry.next;
				glThdListEntry.next = pMyThdTmp;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				MemFree(pMyThdTmp);
				ObDereferenceObject(pEth);
				KdPrint(("ThEnum1:Thread Enum failed!0x%x\n", GetExceptionCode()));
			}
			//KdPrint(("Tid:%d\tPid:%d\t%s\n", ulThreadId, ulPid, pProcName));
		}
		ObDereferenceObject(pEth);
	}



}

VOID ThEnum2(IN PEPROCESS process)
{
	PETHREAD pEth;
	PETHREAD pEthFirst;
	PEPROCESS pEproc = process;
	PLIST_ENTRY pLe = NULL;
	PMYTHREAD pMyThdTmp = NULL;
	NTSTATUS status;
	ULONG ulThreadId;
	CHAR cPrioty;
	UCHAR ucState;


	//KdPrint(("ThEnum2\n"));
	//KdBreakPoint();
	pLe = (PLIST_ENTRY)((PCHAR)pEproc + 0x2c);
	pLe = pLe->Flink;
	pEth = (PETHREAD)((PCHAR)pLe - 0x1e0);
	pEthFirst = pEth;
	__try 
	{
		EmptyGlThd();
		do
		{
			ulThreadId = *(PULONG)((PCHAR)pEth + 0x230);
			if (ulThreadId)
			{
				//KdPrint(("\tTid:%d\n", ulThreadId));
				pMyThdTmp = (PMYTHREAD)MemAlloc(sizeof(MYTHREAD));
				if (!pMyThdTmp)
				{
					KdPrint(("ThEnum2:Memory Insufficient!\n"));
					return;
				}

				cPrioty = *(CHAR *)((PCHAR)pEth + 0x57);
				ucState = *(UCHAR *)((PCHAR)pEth + 0x68);

				pMyThdTmp->cPrioty = cPrioty;
				pMyThdTmp->ucState = ucState;
				pMyThdTmp->ulTid = ulThreadId;
				pMyThdTmp->next = glThdListEntry.next;
				glThdListEntry.next = pMyThdTmp;
			}


			pLe = (PLIST_ENTRY)((PCHAR)pEth + 0x1e0);
			pLe = pLe->Flink;
			pEth = (PETHREAD)((PCHAR)pLe - 0x1e0);
		} while (pEth != pEthFirst);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MemFree(pMyThdTmp);
		KdPrint(("ThEnum2:Thread Enum Failed!0x%x", GetExceptionCode()));
	}

}

VOID ThEnum3(IN PEPROCESS process)
{
	PETHREAD pThd = NULL;
	NTSTATUS status;
	ULONG tid;
	PMYTHREAD pMyThdTmp = NULL;
	//KdPrint(("ThEnum2\n"));

	__try
	{
		EmptyGlThd();
		for (tid = 0; tid < 20000; tid += 4)
		{
			status = PsLookupThreadByThreadId((HANDLE)tid, &pThd);
			if (NT_SUCCESS(status))
			{
				if (IoThreadToProcess(pThd) == process)
				{
					pMyThdTmp = (PMYTHREAD)MemAlloc(sizeof(MYTHREAD));
					if (!pMyThdTmp)
					{
						KdPrint(("ThEnum3:Memory Insufficient!\n"));
						return;
					}

					pMyThdTmp->cPrioty = *(CHAR *)((PCHAR)pThd + 0x57);
					pMyThdTmp->ucState = *(UCHAR *)((PCHAR)pThd + 0x68);
					pMyThdTmp->ulTid = tid;
					pMyThdTmp->next = glThdListEntry.next;
					glThdListEntry.next = pMyThdTmp;
					//KdPrint(("\tTid:%d\n", tid));
				}
			}
			ObDereferenceObject(pThd);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		MemFree(pMyThdTmp);
		ObDereferenceObject(pThd);
		KdPrint(("ThEnum3:Thread Enum Failed!0x%x\n", GetExceptionCode()));
	}

}




/*ģ�鹦�ܺ���*/
//����ģ��ö��
//�������̵�_EPROCESS�ṹָ�룬ö������װ�ص�ģ��
VOID PsEnumModule(IN PEPROCESS process)
{
	if (!process)
	{
		return;
	}
	//KdBreakPoint();
	KAPC_STATE kApcState;
	NTSTATUS status;
	PPEB pPeb = PsGetProcessPeb(process);
	PLIST_ENTRY pLe;
	PLIST_ENTRY pFirLe;
	PLDR_DATA_TABLE_ENTRY pLdrDataEntry;
	
	PMYMODINFO pMyModInfoTmp = NULL;
	PUNICODE_STRING pDllPath = NULL;
	PVOID ulStartAddr = NULL;
	ULONG ulSize;
	
	if (!pPeb)
	{
		return;
	}
	//��֤��ǰ���̿��Է���Ŀ����̵�ַ�ռ�
	KeStackAttachProcess(process, &kApcState);
	__try
	{
		EmptyGlMod();
		pFirLe = pLe = pPeb->Ldr->InMemoryOrderModuleList.Flink;
		do
		{
			//���ݳ�Ա����InMemoryOrderLinks��LDR_DATA_TABLE_ENTRY�ṹ���ڵ�ƫ�ƣ�����pLe��Ӧ��LDR_DATA_TABLE_ENTRY�ṹ���ڴ��׵�ַ
			pLdrDataEntry = CONTAINING_RECORD(
				pLe,
				LDR_DATA_TABLE_ENTRY,
				InMemoryOrderLinks
			);
			if (pLdrDataEntry->DllBase)
			{
				pMyModInfoTmp = (PMYMODINFO)MemAlloc(sizeof(MYMODINFO));
				if (!pMyModInfoTmp)
				{
					KdPrint(("PsEnumModule:Memory Insufficient!\n"));
					return;
				}
				pDllPath = &pLdrDataEntry->FullDllName;
				ulStartAddr = pLdrDataEntry->DllBase;
				ulSize = *(ULONG *)((PCHAR)pLdrDataEntry + 0x20);

				//��ʼ����UNICODE_STRING
				pMyModInfoTmp->DllPath.Buffer = (PWCH)MemAlloc(DE_MAX_PATH_LEN * sizeof(WCHAR));
				if (!pMyModInfoTmp->DllPath.Buffer)
				{
					KdPrint(("PsEnumModule:Memory Insufficient!\n"));
					MemFree(pMyModInfoTmp);
					return;
				}
				pMyModInfoTmp->DllPath.MaximumLength = DE_MAX_PATH_LEN * sizeof(WCHAR);
				RtlCopyMemory(
					pMyModInfoTmp->DllPath.Buffer,
					pLdrDataEntry->FullDllName.Buffer,
					pLdrDataEntry->FullDllName.Length
				);
				pMyModInfoTmp->DllPath.Length = pLdrDataEntry->FullDllName.Length;
				pMyModInfoTmp->DllPath.Buffer[pMyModInfoTmp->DllPath.Length / 2] = 0x0000;
				pMyModInfoTmp->ulSize = ulSize;
				pMyModInfoTmp->ulStartAddr = ulStartAddr;
				pMyModInfoTmp->next = glModListEntry.next;
				glModListEntry.next = pMyModInfoTmp;

				//KdPrint(("--%wZ\tBase:%p\n",
				//	&pLdrDataEntry->FullDllName,
				//	pLdrDataEntry->DllBase));
			}
			pLe = pLe->Flink;
		} while (pLe->Flink != pFirLe);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("PsEnumModule:Module Enum Failed!0x%x\n", GetExceptionCode()));
	}
	KeUnstackDetachProcess(&kApcState);

}

