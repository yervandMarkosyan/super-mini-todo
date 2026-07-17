# C Backend + Vue 3 Composition API Todo App

A high-performance, lightweight Todo application combining a modern, slick frontend built with **Vue 3 (Composition API)** and **Tailwind CSS**, powered by a blisteringly fast embedded **C backend** using **GNU microhttpd** and **cJSON**. 

This project demonstrates how to migrate high-level backend logic (originally written in Go/Gin) down to systemic C code while handling real-world web concerns like HTTP state preservation, memory cycle cleanup, streaming request bodies, and RESTful CRUD endpoints.

---

## 🚀 Features

- **Blazing Fast Embedded C Stack:** Powered by GNU microhttpd for high-concurrency multi-threaded connection handling.
- **Robust JSON Parsing:** Strict token-validated JSON ingestion powered by `cJSON`.
- **Slick Vue 3 Frontend:** Clean, modular UI leveraging the Vue 3 Composition API via zero-overhead global CDN builds.
- **Full RESTful Lifecycle Sync:**
  - `GET /api/todos`: Pulls persistent structures dynamically.
  - `POST /api/todos`: Registers non-overflowing sequentially tracked task records.
  - `PUT /api/todos/:id`: Updates both completion states and inline text changes.
  - `DELETE /api/todos/:id`: Drops backend allocations safely while instantly updating frontend states.
- **Inline Text Editing:** Double-click any task title to open a custom-focused, reactive input field. Press `Enter` or click away (`blur`) to commit directly to the C service.

---

## 🛠️ Tech Stack & Requirements

### Frontend
- **Framework:** Vue 3 (Composition API Global Distribution)
- **Styling:** Tailwind CSS 4.0 Engine
- **Icons & Assets:** Native Vector SVG

### Backend
- **Language:** Standard C (GCC Compiler)
- **HTTP Engine:** GNU microhttpd (`libmicrohttpd`)
- **JSON Framework:** `cJSON`

---

## 📦 System Installation

Before compiling the application, ensure your operating system has the necessary system header bindings.

### Ubuntu / Debian Linux
```bash
sudo apt update
sudo apt install libmicrohttpd-dev gcc make curl
```

### macOS (via Homebrew)
```bash
brew install libmicrohttpd cjson
```

---

## 🏗️ Project Architecture Layout

```text
todo-app/
├── Makefile             # System Build Automation Script
├── include/             # C Global Header Declarations
│   ├── cJSON.h
│   └── router.h
├── public/              # Static Frontend Assets Asset Directory
│   └── index.html       # Single Page Vue 3 Application Framework
└── src/                 # System C Source Executables
    ├── cJSON.c
    ├── main.c
    └── router.c
```

---

## ⚙️ Compilation & Running

### 1. Compile the Backend
Compile the source using `gcc` passing your custom `include` header path lookup flag:

```bash
gcc src/main.c src/router.c src/cJSON.c -Iinclude -o todo_server -lmicrohttpd
```

### 2. Start the Server
Run the generated executable directly in your command line terminal:

```bash
./todo_server
```
The server will initialize an internal polling loop running natively on port **`8080`**.

### 3. Open the Application
Simply mount your browser context to point directly to your service layer, or configure your internal proxy files to deliver your static `public/index.html` frontend layout alongside the backend loop.

---

## 🛡️ Critical C Memory & Architectural Details

Unlike memory-managed runtimes like Go, migrating web route patterns to C introduces system architecture constraints handled natively within this repository:

1. **State Preservation Across Cycles (`con_cls`):** GNU microhttpd executes connection loops in a multi-pass structure. Raw socket data fragments stream chunk-by-chunk. This system implements a custom `RequestContext` struct allocated dynamically on pass one, preventing mid-flight JSON evaluation memory corruption.
2. **Buffer Overflow Guards:** Path metrics parse straight via standard boundary `strncmp` rules. String data injections utilize size-restricted `strncpy` procedures enforcing an explicit null-termination byte (`\0`) mapping, rendering your backend baseline fully secure against heap-smashing vulnerabilities.
3. **Safe Dynamic ID Resolution:** Avoids client-side timestamp keys (like JavaScript's `Date.now()`) which instantly overflow standard 32-bit signed C `int` variables. Instead, IDs are calculated sequentially by the backend array tracker (`todo_count`).
