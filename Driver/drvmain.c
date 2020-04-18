#include"drvdef.h"

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING SymboliclinkName = RTL_CONSTANT_STRING(L"\\??\\ProcToolDev");
	IoDeleteDevice(DriverObject->DeviceObject);
	IoDeleteSymbolicLink(&SymboliclinkName);
	
	EmptyGlProc();
	EmptyGlThd();
	EmptyGlMod();

	KdPrint(("Goodbye Driver!\n"));
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	KdPrint(("Hello Driver!\n"));
	pDriverObject->DriverUnload = Unload;
	NTSTATUS status;
	int i;
	PDEVICE_OBJECT pDevObj = NULL;
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\ProcToolDev");
	UNICODE_STRING SymboliclinkName = RTL_CONSTANT_STRING(L"\\??\\ProcToolDev");

	status = IoCreateDevice(
		pDriverObject,
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pDevObj
	);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("DriverEntry:Device Create Failed!0x%x\n", status));
		return status;
	}
	status = IoCreateSymbolicLink(
		&SymboliclinkName,
		&DeviceName
	);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("DriverEntry:DeviceSymboliclink Create Failed!0x%x\n", status));
		IoDeleteDevice(pDevObj);
		return status;
	}

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		pDriverObject->MajorFunction[i] = DispatchRoutine;
	}
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIOCTL;

	pDevObj->Flags |= 0;
	pDevObj->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}