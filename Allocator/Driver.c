#pragma warning (disable : 4996) // "'ExAllocatePoolWithTag': ExAllocatePoolWithTag is deprecated, use ExAllocatePool2."

#include <ntddk.h>

#define DEVICE_NAME L"\\Device\\Allocator"
#define DOS_DEVICE_NAME L"\\DosDevices\\Allocator"

#define POOL_TAG 'liaF'

#define IOCTL_ALLOCATE_PAGED_OBJECT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

UNICODE_STRING g_DeviceName = { 0 };
UNICODE_STRING g_DosDeviceName = { 0 };

PDEVICE_OBJECT g_pDeviceObject = 0;

NTSTATUS DriverDispatchCreateClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	NTSTATUS Status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, 0);
	return Status;
}

NTSTATUS DriverDispatchControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIO = IoGetCurrentIrpStackLocation(pIrp);

	switch (pIO->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_ALLOCATE_PAGED_OBJECT:
	{
		PVOID pPagedObject = 0;
		PVOID pSystemBuffer = pIrp->AssociatedIrp.SystemBuffer;

		if (!pSystemBuffer)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		// Target Allocation Size: 0xA0 (-0x10 bytes to accomodate for the pool header (nt!_POOL_HEADER))
		if (pIO->Parameters.DeviceIoControl.InputBufferLength != 0x90)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		pPagedObject = ExAllocatePoolWithTag(PagedPool, pIO->Parameters.DeviceIoControl.InputBufferLength, POOL_TAG);
		if (!pPagedObject)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		memcpy(pPagedObject, pSystemBuffer, pIO->Parameters.DeviceIoControl.InputBufferLength);
		break;
	}
	}

	pIrp->IoStatus.Status = Status;
	pIrp->IoStatus.Information = 0;

	IoCompleteRequest(pIrp, 0);
	return Status;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	IoDeleteSymbolicLink(&g_DosDeviceName);
	IoDeleteDevice(g_pDeviceObject);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS Status = STATUS_SUCCESS;

	RtlInitUnicodeString(&g_DeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&g_DosDeviceName, DOS_DEVICE_NAME);

	Status = IoCreateDevice(pDriverObject, 0, &g_DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, 0, &g_pDeviceObject);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	Status = IoCreateSymbolicLink(&g_DosDeviceName, &g_DeviceName);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(g_pDeviceObject);
		return Status;
	}

	ASSERT(pDriverObject);
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDispatchCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDispatchCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatchControl;

	pDriverObject->DriverUnload = DriverUnload;

	return Status;
}