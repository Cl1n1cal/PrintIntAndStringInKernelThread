#include <ntddk.h>
#define STRING_SIZE 100

typedef struct _THREAD_DATA {
	char Value1[STRING_SIZE];
	int Value2;
} THREAD_DATA, * PTHREAD_DATA;

//Prototypes
void DriverUnload(PDRIVER_OBJECT DriverObject);

void EventsWaiter();

NTSTATUS DriverCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

void MyThreadFunction(PVOID Context);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject)
{
	DbgPrint("DriverEntry called\n");

	EventsWaiter();

	//To enabling opening of a handle for this driver
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;


	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	//empty for now
}

//This driver is to wait on the named event instructed by the userspace appliacatio
//This driver is to wait on all named events

void EventsWaiter()
{
	DbgPrint("EventsWaiter called\n");

	NTSTATUS status;
	HANDLE threadHandle;
	PTHREAD_DATA pThreadData;

	// Allocate memory for the thread's data
	pThreadData = (PTHREAD_DATA)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(THREAD_DATA), 'tdTh');

	if (pThreadData) {
		// Initialize the data
		RtlCopyMemory(pThreadData->Value1, "Hello from the thread", sizeof("Hello From the Thread"));
		pThreadData->Value2 = 20;

		// Create the system thread, passing the data as the Context parameter
		status = PsCreateSystemThread(&threadHandle, 0, NULL, NULL, NULL, MyThreadFunction, (PVOID)pThreadData);

		if (NT_SUCCESS(status)) {
			// Successfully created the thread
			ZwClose(threadHandle);  // Close the handle, thread still runs
		}
		else {
			// Handle failure case, free the memory
			ExFreePool(pThreadData);
		}
	}
	else {
		DbgPrint("Could not allocate memory\n");
	}

}

void MyThreadFunction(PVOID Context)
{
	PTHREAD_DATA pThreadData = (PTHREAD_DATA)Context;  // Cast the Context back to the correct type

	// Use the passed data
	DbgPrint("Value1 = %s\n", pThreadData->Value1);
	DbgPrint("Value2 = %d\n", pThreadData->Value2);

	// Free the allocated memory once the data is no longer needed
	ExFreePool(pThreadData);

	// Terminate the thread
	PsTerminateSystemThread(STATUS_SUCCESS);
}


NTSTATUS DriverCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
