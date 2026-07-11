#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "../include/controller.h"
#include "../include/service.h"

static enum MHD_Result handle_get_requests(struct MHD_Connection *connection, const char *url) {
    // Serve the Vue page
    if (strcmp(url, "/") == 0) {
        size_t view_len = 0;
        const char *html_view = render_todo_view(&view_len);

        struct MHD_Response *response = MHD_create_response_from_buffer(view_len, (void*)html_view, MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header(response, "Content-Type", "text/html");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    // Serve static json
    } else if (strcmp(url, "/api/todos") == 0) {
        size_t json_len = 0;
        const char *json_data = get_todos(&json_len);

        struct MHD_Response *response = MHD_create_response_from_buffer(json_len, (void*)json_data, MHD_RESPMEM_MUST_FREE);
        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    // Fallback for other requests
    } else {
        const char *not_found_body = "<h1>404 Not Found</h1>";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(not_found_body), (void*)not_found_body, MHD_RESPMEM_PERSISTENT);
        return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    }
}

static enum MHD_Result handle_post_requests(struct MHD_Connection *connection, const char *url,
                                            const char *upload_data, size_t *upload_data_size,
                                            void **con_cls)
{
    // Step 1: Initialize context block on the very first header callback
    if (*con_cls == NULL) {
        RequestContext *new_ctx = calloc(1, sizeof(RequestContext));
        if (new_ctx == NULL) return MHD_NO;
        *con_cls = new_ctx;
        return MHD_YES;
    }
    RequestContext *ctx = (RequestContext *)*con_cls;

    // Step 2: While stream fragments are coming in, save them to memory
    if (*upload_data_size > 0) {
        char *new_buffer = realloc(ctx->data, ctx->size + *upload_data_size + 1);
        if (!new_buffer) return MHD_NO;
        ctx->data = new_buffer;
        memcpy(&(ctx->data[ctx->size]), upload_data, *upload_data_size);
        ctx->size += *upload_data_size;
        ctx->data[ctx->size] = '\0';
        *upload_data_size = 0;

        return MHD_YES;
    }

    // Step 3: Payload complete! (*upload_data_size == 0) Process the stored data now.
    if (strcmp(url, "/todos") == 0) {
        // Fallback safety if curl sent zero bytes
        if (ctx->data == NULL) {
            ctx->data = strdup("{}");
        }
        printf("\n--- incoming Payload Debug Log ---\n");
        printf("Raw Buffer: %s\n", ctx->data);
        printf("----------------------------------\n");

        cJSON *root = cJSON_Parse(ctx->data);

        if (root == NULL) {
            const char *err = "{\"error\":\"Malformed or Empty JSON Input\"}";
            struct MHD_Response *r = MHD_create_response_from_buffer(strlen(err), (void *)err, MHD_RESPMEM_MUST_COPY);
            MHD_add_response_header(r, "Content-Type", "application/json");
            enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, r);
            MHD_destroy_response(r); return ret;
        }

        // Send to service.c to modify the array
        const char *response_body = post_todos(root);
        cJSON_Delete(root);

        // Build the network return payload package back to client
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_body), (void *)response_body, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_CREATED, response);
        MHD_destroy_response(response);
        return ret;
    }
    return MHD_NO;
}

static enum MHD_Result handle_delete_requests(struct MHD_Connection *connection, const char *url)
{
    const char *prefix = "/api/todos/";

    if (strncmp(url, prefix, strlen(prefix)) == 0) {
        const char *id_start = url + strlen(prefix);
        const int id = atoi(id_start);

        if (id <= 0) {
            const char *bad_req = "{\"status\":\"error\",\"message\":\"Invalid ID specified\"}";
            struct MHD_Response *response = MHD_create_response_from_buffer(
                strlen(bad_req), (void *)bad_req, MHD_RESPMEM_PERSISTENT
            );
            enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
            MHD_destroy_response(response);
            return ret;
        }

        const char *response_body = delete_todos(id);

        struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(response_body), (void *)response_body, MHD_RESPMEM_PERSISTENT
        );

        MHD_add_response_header(response, "Content-Type", "application/json");
        enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Fallback requests
    const char *not_found = "{\"status\":\"error\",\"message\":\"Not Found\"}";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(not_found), (void *)not_found, MHD_RESPMEM_PERSISTENT
    );
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}


enum MHD_Result handler_request_routing(void *cls, struct MHD_Connection *connection,
                                        const char *url, const char *method,
                                        const char *version, const char *upload_data,
                                        size_t *upload_data_size, void **con_cls)
{
    if (strcmp(method, "GET") == 0) {
        return handle_get_requests(connection, url);
    }
    else if (strcmp(method, "POST") == 0) {
        return handle_post_requests(connection, url, upload_data, upload_data_size, con_cls);
    }
    else if (strcmp(method, "DELETE") == 0) {
        return handle_delete_requests(connection, url);
    }

    const char *bad_method = "{\"error\":\"Method Not Allowed\"}";
    struct MHD_Response *r = MHD_create_response_from_buffer(strlen(bad_method), (void *)bad_method, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(r, "Content-Type", "application/json");
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, r);
    MHD_destroy_response(r);
    return ret;
}
