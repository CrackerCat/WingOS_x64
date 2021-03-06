#pragma once
#include <arch.h>
#include <gdt.h>
#include <stdint.h>

#define IDT_ENTRY_COUNT 256
#define INTGATE 0x8e
#define TRAPGATE 0xeF

typedef void (*irq_handler_func)(unsigned int irq);
class idt_entry
{
public:
    uint16_t offset_low16;
    uint16_t cs;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_mid16;
    uint32_t offset_high32;
    uint32_t zero;
    idt_entry(){};
    idt_entry(void *idt_handler, uint8_t idt_ist, uint8_t idt_type)
    {
        uintptr_t p = (uintptr_t)idt_handler;
        offset_low16 = (uint16_t)p;
        cs = gdt_selector::KERNEL_CODE;
        ist = idt_ist;
        attributes = idt_type;
        offset_mid16 = (uint16_t)(p >> 16);
        offset_high32 = (uint32_t)(p >> 32);
        zero = 0;
    };
} __attribute__((packed));
struct idtr
{
    uint16_t size;    // size of the IDT table
    uintptr_t offset; // address of the IDT table
} __attribute__((packed));

void init_idt();

void dump_backtrace(const char *msg, uint64_t *array);
void dumpregister(InterruptStackFrame *stck);
void add_irq_handler(irq_handler_func func, uint8_t irq_target);
