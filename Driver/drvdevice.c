#include"drvdef.h"

/*������ʼ��*/
//��ʼ����ʱȫ�ֵ�����
VOID init()
{
	glProcListEntry.next = NULL;
	glThdListEntry.next = NULL;
	glThdListEntry.next = NULL;
}

/*ȫ�ֵ��������*/
//����(��ҳ)�ڴ�
PVOID MemAlloc(IN size_t size)
{
	return ExAllocatePool(PagedPool, size);
}
//�ͷ��ڴ�
VOID MemFree(IN PVOID p)
{
	ExFreePool(p);
}
//ȫ�ֵ��������,����ͷ�ڵ�
VOID EmptyGlProc()
{
	PMYPROCESS pTmp = glProcListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		MemFree(glProcListEntry.next);
		glProcListEntry.next = pTmp;
	}
}
VOID EmptyGlThd()
{
	PMYTHREAD pTmp = glThdListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		MemFree(glThdListEntry.next);
		glThdListEntry.next = pTmp;
	}
}
VOID EmptyGlMod()
{
	PMYMODINFO pTmp = glModListEntry.next;
	while (pTmp)
	{
		pTmp = pTmp->next;
		MemFree(glModListEntry.next->DllPath.Buffer);
		MemFree(glModListEntry.next);
		glModListEntry.next = pTmp;
	}
}

/*��ǲ����*/
//ͨ����ǲ��������
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	static PCHAR IrpName[] = {
		"IRP_MJ_CREATE					",
		"IRP_MJ_CREATE_NAMED_PIPE		",
		"IRP_MJ_CLOSE					",
		"IRP_MJ_READ					",
		"IRP_MJ_WRITE					",
		"IRP_MJ_QUERY_INFORMATION		",
		"IRP_MJ_SET_INFORMATION			",
		"IRP_MJ_QUERY_EA				",
		"IRP_MJ_SET_EA					",
		"IRP_MJ_FLUSH_BUFFERS			",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION	",
		"IRP_MJ_DIRECTORY_CONTROL		",
		"IRP_MJ_FILE_SYSTEM_CONTROL		",
		"IRP_MJ_DEVICE_CONTROL			",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL	",
		"IRP_MJ_SHUTDOWN				",
		"IRP_MJ_LOCK_CONTROL			",
		"IRP_MJ_CLEANUP					",
		"IRP_MJ_CREATE_MAILSLOT			",
		"IRP_MJ_QUERY_SECURITY			",
		"IRP_MJ_SET_SECURITY			",
		"IRP_MJ_POWER					",
		"IRP_MJ_SYSTEM_CONTROL			",
		"IRP_MJ_DEVICE_CHANGE			",
		"IRP_MJ_QUERY_QUOTA				",
		"IRP_MJ_SET_QUOTA				",
		"IRP_MJ_PNP						"
	};
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	KdPrint(("%s\n", IrpName[stack->MajorFunction]));

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
//������IRP��ǲ����
NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	NTSTATUS status;
	PCHAR pInBuf;	//��IRP�д����������ں�̬�Ļ�������ַ
	PCHAR pOutBuf;	//��IRP������������û�̬�Ļ�������ַ
	ULONG iBufLen;	//�������ݵ��ֽڳ���
	ULONG oBufLen;	//������ݵ��ֽڳ���
	ULONG retLen = 0;	//�������û�̬�����ݳ���
	PIO_STACK_LOCATION stack;	//��ȡ��ǰIO�豸ջ��Ԫ
	ULONG ctlCode;	//���ƹ�����
	KdBreakPoint();
	//�ֲ�������ֵ
	stack = IoGetCurrentIrpStackLocation(pIrp);
	ctlCode = stack->Parameters.DeviceIoControl.IoControlCode;
	pInBuf = pIrp->AssociatedIrp.SystemBuffer;
	//pOutBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	iBufLen = stack->Parameters.DeviceIoControl.InputBufferLength;
	//oBufLen = stack->Parameters.DeviceIoControl.OutputBufferLength;

	//KdBreakPoint();
	switch (ctlCode)
	{
	case EnumProc:
	{
		KdPrint(("DispatchIOCTL:EnumProc In\n"));
		pOutBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		oBufLen = stack->Parameters.DeviceIoControl.OutputBufferLength;
		status = EnumPs(
			pOutBuf,
			oBufLen,
			&retLen
		);

	}break;
	case EnumThd:
	{
		KdPrint(("DispatchIOCTL:EnumThd In\n"));
		pOutBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		oBufLen = stack->Parameters.DeviceIoControl.OutputBufferLength;
		status = EnumThdByPid(
			*(PHANDLE)pInBuf,
			pOutBuf,
			oBufLen,
			&retLen
		);
	}break;
	case EnumPMod:
	{
		KdPrint(("DispatchIOCTL:EnumPMod In\n"));
		pOutBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		oBufLen = stack->Parameters.DeviceIoControl.OutputBufferLength;
		status = EnumModByPid(
			*(PHANDLE)pInBuf,
			pOutBuf,
			oBufLen,
			&retLen
		);

	}break;
	case SuspendProc:
	{
		KdPrint(("DispatchIOCTL:SuspendProc In\n"));
		status = PsSuspend(
			*(PHANDLE)pInBuf
		);
	}break;
	case ResumeProc:
	{
		KdPrint(("DispatchIOCTL:ResumeProc In\n"));
		status = PsResume(
			*(PHANDLE)pInBuf
		);
	}break;
	case TerminateProc:
	{
		KdPrint(("DispatchIOCTL:TerminateProc In\n"));
		status = PsTerminate(
			*(PHANDLE)pInBuf
		);
	}break;
	default:
	{
		KdPrint(("DispatchIOCTL:Unknow IOCTLCode!\n"));
		status = STATUS_UNSUCCESSFUL;
	}
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = retLen;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}

/*Ӧ�ó���ͨ�ź���*/
//ö�����н���
//���ý���ö�ٺ��������н�����Ϣװ����ʱȫ�ֽ�������
//����ʱȫ�ֽ����������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ�ֽ�������
NTSTATUS EnumPs(
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen)
{
	NTSTATUS status;
	ULONG wLen = 0;
	PMYPROCESS pMyProcTmp;
	if (!oBuf || !retLen)
	{
		KdPrint(("EnumPs:INVALID PARAMETER!\n"));
		return STATUS_INVALID_PARAMETER;
	}

	//���ý���ö�ٺ��������н�����Ϣװ����ʱȫ�ֽ�������
	PsEnum1();
	__try
	{
		KdBreakPoint();
		//����ʱȫ�ֽ����������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
		//д�������������ʱȫ�ֽ�������
		pMyProcTmp = glProcListEntry.next;
		while (pMyProcTmp)
		{
			if (wLen + sizeof(MYPROCESS) > oLen)
			{	//д������ݳ��ȳ����������������ʱ���ضϲ������ʱȫ�ֽ�������
				EmptyGlProc();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{	//��������������㹻����д��һ��MYPROCESS�ṹ������ݣ�д������д����wLen��������д����
				RtlCopyMemory(
					oBuf + wLen,
					pMyProcTmp,
					sizeof(MYPROCESS)
				);
				wLen += sizeof(MYPROCESS);
				pMyProcTmp = pMyProcTmp->next;
				MemFree(glProcListEntry.next);
				glProcListEntry.next = pMyProcTmp;
			}
		}
		*retLen = wLen;
		return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		EmptyGlProc();
		KdPrint(("EnumPs:Enum Exception!0x%x\n", status));
		*retLen = wLen;
		return status;
	}
}

//ö�ٸ���pid���̵������߳�
//�����߳�ö�ٺ����������߳���Ϣװ����ʱȫ�ֽ�������
//����ʱȫ���߳��������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ���߳�����
NTSTATUS EnumThdByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
)
{
	NTSTATUS status;
	PEPROCESS pEproc = NULL;
	PMYTHREAD pMyThdTmp = NULL;
	ULONG wLen = 0;

	if (!oBuf || !retLen)
	{
		return STATUS_INVALID_PARAMETER;
	}
	status = PsLookupProcessByProcessId(pid, &pEproc);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("EnumThdByPid:LookupProcessByProcessId Failed!0x%x\n", status));
		return status;
	}

	//�����߳�ö�ٺ����������߳���Ϣװ����ʱȫ�ֽ�������
	ThEnum2(pEproc);

	__try
	{
		//����ʱȫ���߳��������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
		//д�������������ʱȫ���߳�����
		pMyThdTmp = glThdListEntry.next;
		while (pMyThdTmp)
		{
			if (wLen + sizeof(MYTHREAD) > oLen)
			{	//д������ݳ��ȳ����������������ʱ���ضϲ������ʱȫ���߳�����
				EmptyGlThd();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{	//��������������㹻����д��һ��MYTHREAD�ṹ�����ݣ�д������д����wLen��������д����
				RtlCopyMemory(
					oBuf + wLen,
					pMyThdTmp,
					sizeof(MYTHREAD)
				);
				wLen += sizeof(MYTHREAD);
				pMyThdTmp = pMyThdTmp->next;
				MemFree(glThdListEntry.next);
				glThdListEntry.next = pMyThdTmp;
			}
		}
		*retLen = wLen;
		return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		EmptyGlThd();
		KdPrint(("EnumThdByPid:Enum Exception!0x%x\n", status));
		*retLen = wLen;
		return status;
	}

}

//ö�ٸ���pid���̵�����װ��ģ��
//����ģ��ö�ٺ���������ģ����Ϣװ����ʱȫ��ģ������
//����ʱȫ��ģ���������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
//д�������������ʱȫ��ģ������
NTSTATUS EnumModByPid(
	IN HANDLE pid,
	OUT PCHAR oBuf,
	IN ULONG oLen,
	OUT ULONG *retLen
)
{
	NTSTATUS status;
	PEPROCESS pEproc = NULL;
	PMYMODINFO pMyModTmp = NULL;
	ULONG wLen = 0;

	if (!oBuf || !retLen)
	{
		return STATUS_INVALID_PARAMETER;
	}
	status = PsLookupProcessByProcessId(pid, &pEproc);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("EnumModByPid:LookupProcessByProcessId Failed!0x%x\n", status));
		return status;
	}

	//����ģ��ö�ٺ���������ģ����Ϣװ����ʱȫ��ģ������
	PsEnumModule(pEproc);
	__try
	{
		//����ʱȫ��ģ���������Ϣ���������ΪoLen�Ļ�����oBuf�У�����д�볤����retLen
		//д�������������ʱȫ��ģ������
		pMyModTmp = glModListEntry.next;
		while (pMyModTmp)
		{
			if (wLen + sizeof(MYMODINFO) + DE_MAX_FILE_NAME_LEN * sizeof(WCHAR) > oLen)
			{	//д�����ݳ��ȳ����������������ʱ���ضϲ������ʱȫ��ģ������
				EmptyGlMod();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				//�������ģ�����
				RtlCopyMemory(
					oBuf + wLen,
					pMyModTmp,
					sizeof(MYMODINFO)
				);
				wLen += sizeof(MYMODINFO);
				//�������ģ��·���ַ��������ֽڵ�λ��
				RtlCopyMemory(
					oBuf + wLen,
					pMyModTmp->DllPath.Buffer,
					DE_MAX_PATH_LEN * sizeof(WCHAR)
				);
				wLen += DE_MAX_PATH_LEN * sizeof(WCHAR);

				pMyModTmp = pMyModTmp->next;
				MemFree(glModListEntry.next->DllPath.Buffer);
				MemFree(glModListEntry.next);
				glModListEntry.next = pMyModTmp;
			}
		}
		*retLen = wLen;
		return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		EmptyGlMod();
		KdPrint(("EnumModByPid:Enum Exception!0x%x\n", status));
		*retLen = wLen;
		return status;
	}
}