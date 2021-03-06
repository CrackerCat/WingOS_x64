#include <arch.h>
#include <gdt.h>
#include <interrupt.h>
#include <liballoc.h>
#include <utility.h>
#include <virtual.h>
//#include <arch/msr_syscall.h> sorry this file will exist one day
#include <com.h>
#include <device/acpi.h>
#include <device/apic.h>
#include <device/apic_timer.h>
#include <device/ata_driver.h>
#include <device/hpet.h>
#include <device/local_data.h>
#include <device/madt.h>
#include <device/mboot_module.h>
#include <device/pci.h>
#include <device/pit.h>
#include <device/ps_keyboard.h>
#include <device/ps_mouse.h>
#include <device/rtc.h>
#include <filesystem/echfs.h>
#include <filesystem/file_system.h>
#include <filesystem/partition/base_partition.h>
#include <kernel.h>
#include <kernel_service/kernel_service.h>
#include <logging.h>
#include <pic.h>
#include <process.h>
#include <programm_launcher.h>
#include <smp.h>
#include <sse.h>

static char stack[STACK_SIZE] = {0};
static uintptr_t bootdat = 0;
__attribute__((section(".stivalehdr"), used))
stivale_header header = {.stack = (uintptr_t)stack + (sizeof(char) * STACK_SIZE),
                         .flags = 1,
                         .framebuffer_width = 1440,
                         .framebuffer_height = 900,
                         .framebuffer_bpp = 32,
                         .entry_point = 0};

stivale_struct boot_loader_data_copy;

void start_process();

/* ____    __    ____  __  .__   __.   _______      ______        _______.
//  \   \  /  \  /   / |  | |  \ |  |  /  _____|    /  __  \      /       |
//   \   \/    \/   /  |  | |   \|  | |  |  __     |  |  |  |    |   (----`
//    \            /   |  | |  . `  | |  | |_ |    |  |  |  |     \   \
//     \    /\    /    |  | |  |\   | |  |__| |    |  `--'  | .----)   |
//      \__/  \__/     |__| |__| \__|  \______|     \______/  |_______/
//
*/

unsigned int get_cpu_count()
{
    return smp::the()->processor_count;
}
unsigned int get_current_cpu_id()
{
    return apic::the()->get_current_processor_id();
}
bool echo_out(const char *data, unsigned int size)
{
    return com_write_strn(data, size);
}
ASM_FUNCTION void kernel_start(stivale_struct *bootloader_data)
{
    asm volatile("and rsp, -16");
    asm volatile("cli");
    // fs is used for getting cpu n°
    asm volatile("mov ax, 0");
    asm volatile("mov fs, ax");
    init_sse();

    com_initialize(COM_PORT::COM1);
    log("COM1", LOG_INFO) << "com port 1 loaded";

    memcpy(&boot_loader_data_copy, bootloader_data, sizeof(stivale_struct));

    setup_gdt();

    init_idt();
    PIT::the()->init_PIT();
    tss_init((uintptr_t)stack + sizeof(char) * STACK_SIZE);

    init_physical_memory(bootloader_data);
    init_vmm(bootloader_data);

    ps_mouse::the()->init();
    ps_keyboard::the()->init();
    pic_init();
    RTC::the()->init();
    acpi::the()->init((reinterpret_cast<stivale_struct *>(bootdat))->rsdp);

    acpi::the()->getFACP();

    madt::the()->init();

    apic::the()->init();

    smp::the()->init();

    apic_timer::the()->init();
    for (int i = 0; i < 16; i++)
    {

        apic::the()->set_redirect_irq(0, i, 1);
    }

    set_current_data(get_current_cpu());

    mboot_module::the()->init(bootloader_data);
    bootdat = ((uint64_t)&boot_loader_data_copy);

    lock_process();
    printf("\n\n");
    printf(":::       ::: ::::::::::: ::::    :::  ::::::::         ::::::::   ::::::::\n");
    printf(":+:       :+:     :+:     :+:+:   :+: :+:    :+:       :+:    :+: :+:    :+:\n");
    printf("+:+       +:+     +:+     :+:+:+  +:+ +:+              +:+    +:+ +:+        \n");
    printf("+#+  +:+  +#+     +#+     +#+ +:+ +#+ :#:              +#+    +:+ +#++:++#++ \n");
    printf("+#+ +#+#+ +#+     +#+     +#+  +#+#+# +#+   +#+#       +#+    +#+        +#+ \n");
    printf(" #+#+# #+#+#      #+#     #+#   #+#+# #+#    #+#       #+#    #+# #+#    #+# \n");
    printf("  ###   ###   ########### ###    ####  ########         ########   ########  \n");

    printf("\n");
    init_multi_process(start_process);
    asm volatile("sti");
    asm("hlt");
}
void start_process()
{
    load_kernel_service();
    pci_system::the()->init();
    //ata_driver::the()->init();
    //  set_io_device((io_device *)ata_driver::the(), 0);

    main_fs_system::the()->init_file_system();

    //   launch_programm("init_fs/memory_service.exe", main_fs_system::the()->main_fs());

    _start((stivale_struct *)bootdat);
}
