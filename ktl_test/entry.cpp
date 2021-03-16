#include "ktl_test.h"
#include "test.h"

#include <ktl_core.h>
#include <kernel>
#include <memory>
#include <mutex>
#include <string>
#include <wdf>

DECLARE_CONST_UNICODE_STRING(ntDeviceName, L"\\Device\\" KTL_TEST_DEVICE_USERMODE_NAME);

extern "C"
{
    DRIVER_INITIALIZE DriverEntry;
    EVT_WDF_DEVICE_FILE_CREATE KtlTestCreate;
    EVT_WDF_FILE_CLOSE KtlTestClose;
    EVT_WDF_IO_QUEUE_IO_READ KtlTestFileIoRead;
    EVT_WDF_IO_QUEUE_IO_WRITE KtlTestFileIoWrite;
    EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL KtlTestFileIoDeviceControl;
    EVT_WDF_IO_IN_CALLER_CONTEXT KtlTestDeviceIoInCallerContext;
    EVT_WDF_DEVICE_SHUTDOWN_NOTIFICATION KtlTestDriverShutdown;
    EVT_WDF_DRIVER_UNLOAD KtlTestDriverUnload;
    EVT_WDF_DEVICE_CONTEXT_CLEANUP KtlTestDriverContextCleanup;
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, KtlTestCreate )
#pragma alloc_text( PAGE, KtlTestClose )
#pragma alloc_text( PAGE, KtlTestDriverShutdown )
#pragma alloc_text( PAGE, KtlTestDriverUnload )
#pragma alloc_text( PAGE, KtlTestDriverContextCleanup )
#endif

static ktl::unique_ptr<KtlGlobalState> State{ new (ktl::pool_type::NonPaged) KtlGlobalState };

NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING      RegistryPath
)
{
    if (!ktl::initialize_runtime())
        return STATUS_FAILED_DRIVER_ENTRY;

    ktl::scope_exit cleanupRuntime([]() -> void
        {
            ktl::unload_runtime();
        });

    ktl::wdf_driver_config        config;
    ktl::wdf_object_attributes    attributes;
    WDFDRIVER                     hDriver;

    config->DriverInitFlags |= WdfDriverInitNonPnpDriver;
    config->EvtDriverUnload = KtlTestDriverUnload;

    attributes->EvtCleanupCallback = KtlTestDriverContextCleanup;

    NTSTATUS status = WdfDriverCreate(DriverObject,
        RegistryPath,
        &attributes,
        &config,
        &hDriver);
    if (!NT_SUCCESS(status))
    {
        LOG_ERROR("WdfDriverCreate failed: %#x\n", status);
        return status;
    }

    // Normally in addDevice
    ktl::wdf_device_init init{ hDriver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R };

    if (!init)
    {
        LOG_ERROR("Unable to allocate WDF device init\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = WdfDeviceInitAssignName(&init, &ntDeviceName);

    if (!NT_SUCCESS(status)) {
        LOG_ERROR("Unable to assign device name: %wZ\n", &ntDeviceName);
        return status;
    }

    init.set_exclusive_access(false);
    init.set_io_type(WdfDeviceIoBuffered);
    init.set_shutdown_handler(KtlTestDriverShutdown);
    init.set_file_object_config(KtlTestCreate, KtlTestClose);
    init.set_device_io_in_caller_context_handler(KtlTestDeviceIoInCallerContext);

    WDFDEVICE controlDevice;
    status = WdfDeviceCreate(init.get(), &attributes, &controlDevice);
    if (!NT_SUCCESS(status))
    {
        LOG_ERROR("WdfDeviceCreate() failed: %#x\n", status);
        return status;
    }

    ktl::wdf_io_queue_config ioQueueConfig;
    ioQueueConfig->EvtIoRead = KtlTestFileIoRead;
    ioQueueConfig->EvtIoWrite = KtlTestFileIoWrite;
    ioQueueConfig->EvtIoDeviceControl = KtlTestFileIoDeviceControl;

    WDFQUEUE queue;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    status = WdfIoQueueCreate(controlDevice, &ioQueueConfig,
        &attributes,
        &queue);
    if (!NT_SUCCESS(status))
    {
        LOG_ERROR("WdfIoQueueCreate() failed: %#x\n", status);
        return status;
    }

    WdfControlFinishInitializing(controlDevice);

    status = WdfDeviceCreateSymbolicLink(controlDevice, &KTL_TEST_DEVICE_NAME);
    if (!NT_SUCCESS(status))
    {
        LOG_ERROR("WdfDeviceCreateSymbolicLink() failed: %#x ('%wZ')\n", status, &KTL_TEST_DEVICE_NAME);
        return status;
    }

    cleanupRuntime.release();

    return STATUS_SUCCESS;
}

VOID
KtlTestCreate(
    IN WDFDEVICE            Device,
    IN WDFREQUEST Request,
    IN WDFFILEOBJECT        FileObject
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(FileObject);

    PAGED_CODE();

    NTSTATUS status;
    ktl::wdf_auto_request request{ Request, status };

    return;
}

VOID
KtlTestClose(
    IN WDFFILEOBJECT FileObject
)
{
    UNREFERENCED_PARAMETER(FileObject);

    PAGED_CODE();

    return;
}

VOID
KtlTestFileIoRead(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            Length
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    NTSTATUS status;

    ktl::wdf_auto_request request{ Request, status };
    request.set_information(0);

    return;
}

VOID KtlTestFileIoWrite(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            Length
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Length);

    NTSTATUS status;

    ktl::wdf_auto_request request{ Request, status };
    request.set_information(0);

    return;
}

VOID
KtlTestFileIoDeviceControl(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t            OutputBufferLength,
    IN size_t            InputBufferLength,
    IN ULONG            IoControlCode
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    NTSTATUS status;

    ktl::wdf_auto_request request{ Request, status };

    switch (IoControlCode)
    {
    case IOCTL_KTLTEST_METHOD_ALGORITHM_TEST:
        break;
    case IOCTL_KTLTEST_METHOD_LIST_TEST:
        break;
    case IOCTL_KTLTEST_METHOD_MEMORY_TEST:
        break;
    case IOCTL_KTLTEST_METHOD_SET_TEST:
        if (!test_set())
            status = STATUS_FAIL_CHECK;
        break;
    case IOCTL_KTLTEST_METHOD_VECTOR_TEST:
        if (!test_vector())
            status = STATUS_FAIL_CHECK;
        break;
    case IOCTL_KTLTEST_METHOD_STRING_TEST:
        if (!test_unicode_string())
            status = STATUS_FAIL_CHECK;
        break;
    case IOCTL_KTLTEST_METHOD_STRING_VIEW_TEST:
        if (!test_unicode_string_view())
            status = STATUS_FAIL_CHECK;
        break;
    default:
        break;
    }

    return;
}

VOID
KtlTestDeviceIoInCallerContext(
    IN WDFDEVICE  Device,
    IN WDFREQUEST Request
)
{
    NTSTATUS status;
    ktl::wdf_auto_request request{ Request, status };

    // All IOCTL we support are METHOD_NEITHER, so we can forward them as-is,
    // and process them in the caller's context.
    // Otherwise, just forward the request.
    auto params = request.params();
    if (params.Type == WdfRequestTypeDeviceControl)
    {
        request.forward(Device);
        return;
    }

    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, KTL_TEST_IOCTL_CONTEXT);

    auto context = request.create_context<KTL_TEST_IOCTL_CONTEXT>(&attributes);
    if (!context)
        return;

    context->UsermodeIn = request.user_in();
    context->UsermodeOut = request.user_out();

    if (!context->UsermodeIn || !context->UsermodeOut)
        return;

    request.forward(Device);
    return;
}

VOID
KtlTestDriverUnload(
    _In_ WDFDRIVER DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();

    ktl::unload_runtime();
}

VOID KtlTestDriverContextCleanup(
    IN WDFOBJECT Driver
)
{
    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();
}

VOID
KtlTestDriverShutdown(
    WDFDEVICE Device
)
{
    UNREFERENCED_PARAMETER(Device);
    PAGED_CODE();
    return;
}