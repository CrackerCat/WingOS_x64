#include <klib/kernel_util.h>
#include <klib/process_buffer.h>
#include <klib/process_message.h>

namespace sys
{
    process_buffer::process_buffer(int pid, process_buffer_type type)
    {
        current_cursor = 0;
        current_pid = pid;
        current_type = type;
    }

    uint64_t process_buffer::get_length()
    {
        process_request pr = {0};
        pr.type = GET_PROCESS_BUFFER;
        pr.gpb.buffer_type = current_type;
        pr.gpb.get_buffer_length = true;
        pr.gpb.pid_target = current_pid;
        uint64_t result = sys::process_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
        return result;
    }

    void process_buffer::set_cursor(int at)
    {
        current_cursor = at;
    }

    uint64_t process_buffer::next(uint8_t *data, int length)
    {

        process_request pr = {0};
        pr.type = GET_PROCESS_BUFFER;
        pr.gpb.buffer_type = current_type;
        pr.gpb.get_buffer_length = false;
        pr.gpb.pid_target = current_pid;
        pr.gpb.where = current_cursor;
        pr.gpb.length_to_read = length;
        pr.gpb.target_buffer = data;
        uint64_t result = sys::process_message("kernel_process_service", (uint64_t)&pr, sizeof(pr)).read();
        current_cursor += result;
        return result;
    }
} // namespace sys
