#ifndef CONTROLLER_H
#define CONTROLLER_H

typedef struct {
    char *data;
    size_t size;
} RequestContext;

enum MHD_Result handler_request_routing(void *cls, struct MHD_Connection *connection,
                                        const char *url, const char *method,
                                        const char *version, const char *upload_data,
                                        size_t *upload_data_size, void **con_cls);

#endif // CONTROLLER_H
