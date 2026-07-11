#include "../include/service.h"
#include "../include/user_model.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TODOS 100

static Todo todo_database[MAX_TODOS];
static int todo_count = 0;

void seed_todos(void) {
    if (todo_count < MAX_TODOS) {
        todo_database[todo_count].id = 1;
        strncpy(todo_database[todo_count].title, "Buy groceries", sizeof(todo_database[todo_count].title) - 1);
        todo_database[todo_count].completed = 0;
        todo_count++;
    }

    if (todo_count < MAX_TODOS) {
        todo_database[todo_count].id = 2;
        strncpy(todo_database[todo_count].title, "Finish C web server project", sizeof(todo_database[todo_count].title) - 1);
        todo_database[todo_count].completed = 1;
        todo_count++;
    }

    printf("Database seeded with %d items.\n", todo_count);
}

const char* get_todos(size_t *out_len)
{
    cJSON *root_array = cJSON_CreateArray();

    for (int i = 0; i < todo_count; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", todo_database[i].id);
        cJSON_AddStringToObject(item, "title", todo_database[i].title);
        cJSON_AddBoolToObject(item, "completed", todo_database[i].completed);
        cJSON_AddItemToArray(root_array, item);
    }

    char *json_string = cJSON_PrintUnformatted(root_array);
    cJSON_Delete(root_array);

    if (out_len) {
        *out_len = strlen(json_string);
    }

    return json_string;
}

const char* render_todo_view(size_t *out_len)
{
    const char *filepath = "public/index.html";
    FILE *file = fopen(filepath, "rb");

    if (!file) {
        char *error_str = strdup("<h1>500 - Internal Server Error: Template missing</h1>");
        if (out_len) *out_len = strlen(error_str);
        return error_str;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *html_buffer = malloc(file_size + 1);
    if (!html_buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(html_buffer, 1, file_size, file);
    html_buffer[bytes_read] = '\0';
    fclose(file);

    if (out_len) {
        *out_len = bytes_read;
    }

    return html_buffer; // Returns HTML markup data
}


const char* post_todos(cJSON *json_root) {
    cJSON *title = cJSON_GetObjectItemCaseSensitive(json_root, "title");

    // Ensure title exists and isn't empty
    if (!cJSON_IsString(title) || (strlen(title->valuestring) == 0)) {
        return "{\"status\":\"error\",\"message\":\"Missing or invalid title\"}";
    }

    // Check array constraints bounds
    if (todo_count >= MAX_TODOS) {
        return "{\"status\":\"error\",\"message\":\"Database is full\"}";
    }

    Todo *new_todo = malloc(sizeof(Todo));
    new_todo->id = todo_count + 1;

    strncpy(new_todo->title, title->valuestring, sizeof(new_todo->title) - 1);
    new_todo->title[sizeof(new_todo->title) - 1] = '\0';
    new_todo->completed = 0;

    todo_database[todo_count] = *new_todo;
    todo_count++;

    printf("Created new todo item #%d: %s\n", new_todo->id, new_todo->title);
    return "{\"status\":\"success\"}";
}

const char* delete_todos(int id)
{
    for (int i = 0; i < todo_count; ++i) {
        if (todo_database[i].id == id) {
            printf("Deleted todo #%d: %s\n", todo_database[i].id, todo_database[i].title);

            for (int j = i; j < todo_count - 1; ++j) {
                todo_database[j] = todo_database[j + 1];
            }

            todo_count--;
            return "{\"status\":\"success\"}";
        }
    }

    // Explicit return fallback if the ID does not exist
    return "{\"status\":\"error\",\"message\":\"Todo not found\"}";
}

