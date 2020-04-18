#include"drvdef.h"

/*驱动初始化*/
//初始化临时全局单链表
VOID init()
{
	glProcListEntry.next = NULL;
	glThdListEntry.next = NULL;
	glThdListEntry.next = NULL;
}

/*全局单链表操作*/
//申请(分页)内存
PVOID MemAlloc(IN size_t size)
{
	return ExAllocatePool(PagedPool, size);
}
//释放内存
VOID MemFree(IN PVOID p)
{
	ExFreePool(p);
}
//全局单链表清空,保留头节点
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

/*派遣函数*/
//通用派遣函数处理
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
//功能码IRP派遣函数
NTSTATUS DispatchIOCTL(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	NTSTATUS status;
	PCHAR pInBuf;	//从IRP中传入数据至内核态的缓冲区地址
	PCHAR pOutBuf;	//从IRP中输出数据至用户态的缓冲区地址
	ULONG iBufLen;	//传入数据的字节长度
	ULONG oBufLen;	//输出数据的字节长度
	ULONG retLen = 0;	//返回至用户态的数据长度
	PIO_STACK_LOCATION stack;	//获取当前IO设备栈单元
	ULONG ctlCode;	//控制功能码
	KdBreakPoint();
	//局部变量赋值
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

/*应用程序通信函数*/
//枚举所有进程
//调用进程枚举函数，所有进程信息装入临时全局进程链表
//将临时全局进程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局进程链表
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

	//调用进程枚举函数，所有进程信息装入临时全局进程链表
	PsEnum1();
	__try
	{
		KdBreakPoint();
		//将临时全局进程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
		//写入完毕则销毁临时全局进程链表
		pMyProcTmp = glProcListEntry.next;
		while (pMyProcTmp)
		{
			if (wLen + sizeof(MYPROCESS) > oLen)
			{	//写入后数据长度超出输出缓冲区长度时，截断并清空临时全局进程链表
				EmptyGlProc();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{	//输出缓冲区长度足够，则写入一个MYPROCESS结构体的数据，写入后更新写长度wLen并销毁已写入结点
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

//枚举给定pid进程的所有线程
//调用线程枚举函数，所有线程信息装入临时全局进程链表
//将临时全局线程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局线程链表
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

	//调用线程枚举函数，所有线程信息装入临时全局进程链表
	ThEnum2(pEproc);

	__try
	{
		//将临时全局线程链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
		//写入完毕则销毁临时全局线程链表
		pMyThdTmp = glThdListEntry.next;
		while (pMyThdTmp)
		{
			if (wLen + sizeof(MYTHREAD) > oLen)
			{	//写入后数据长度超出输出缓冲区长度时，截断并清空临时全局线程链表
				EmptyGlThd();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{	//输出缓冲区长度足够，则写入一个MYTHREAD结构体数据，写入后更新写长度wLen并销毁已写入结点
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

//枚举给定pid进程的所有装载模块
//调用模块枚举函数，所有模块信息装入临时全局模块链表
//将临时全局模块链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
//写入完毕则销毁临时全局模块链表
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

	//调用模块枚举函数，所有模块信息装入临时全局模块链表
	PsEnumModule(pEproc);
	__try
	{
		//将临时全局模块链表的信息输出至长度为oLen的缓冲区oBuf中，返回写入长度至retLen
		//写入完毕则销毁临时全局模块链表
		pMyModTmp = glModListEntry.next;
		while (pMyModTmp)
		{
			if (wLen + sizeof(MYMODINFO) + DE_MAX_FILE_NAME_LEN * sizeof(WCHAR) > oLen)
			{	//写入数据长度超出输出缓冲区长度时，截断并清空临时全局模块链表
				EmptyGlMod();
				*retLen = wLen;
				return STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				//拷贝输出模块参数
				RtlCopyMemory(
					oBuf + wLen,
					pMyModTmp,
					sizeof(MYMODINFO)
				);
				wLen += sizeof(MYMODINFO);
				//拷贝输出模块路径字符串（宽字节单位）
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