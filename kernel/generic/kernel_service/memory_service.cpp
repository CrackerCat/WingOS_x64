
#include <kernel_service/memory_service.h>
#include <liballoc.h>
#include <logging.h>
#include <process.h>

void memory_service()
{
    log("memory_service", LOG_INFO) << "loaded memory service";
    set_on_request_service(true);

    while (true)
    {
        process_message *msg = read_message();

        if (msg != 0)
        {
            memory_service_protocol *prot = (memory_service_protocol *)msg->content_address;

            set_on_request_service(false);

            if (prot->request_type == REQUEST_PMM_MALLOC)
            {
                msg->response = (uint64_t)(pmm_alloc_zero(prot->length));
                msg->has_been_readed = true;
            }
            else if (prot->request_type == REQUEST_PMM_FREE)
            {
                msg->response = (uint64_t)1;
                pmm_free((void *)((prot->address)), prot->length);
                msg->has_been_readed = true;
            }
            else
            {
                msg->has_been_readed = true;
                msg->response = 0;
                log("memory_service", LOG_ERROR) << "not valid request memory" << (uint64_t)prot->request_type;
            }

            set_on_request_service(true);
        }
        else if (msg == 0)
        {
        }
    }
}
