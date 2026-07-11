#include "cJSON.h"

#ifndef SERVICE_H
#define SERVICE_H

void seed_todos();
const char* get_todos(size_t *);
const char* render_todo_view(size_t *);
const char* post_todos(cJSON *json_root);
const char* delete_todos(int id);

#endif // SERVICE_H
