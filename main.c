#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/controller.h"
#include "include/service.h"

#define PORT 8888


void request_completed_callback(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    if (*con_cls != NULL) {
        // Cast and safely free the context buffer
        RequestContext *ctx = (RequestContext *)*con_cls;
        if (ctx->data) free(ctx->data);
        free(ctx);
        *con_cls = NULL; // Prevent dangling pointer
    }
}

int main()
{
    seed_todos();
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                              PORT, NULL, NULL,
                              &handler_request_routing, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, &request_completed_callback, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start daemon.\n");
        return 1;
    }
    printf("Server running on port %d. Press Enter to stop.\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}

