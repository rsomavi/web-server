# HTTP Server in C

A lightweight, multi-process HTTP server written in C for serving static files and executing server-side scripts. Developed as a university assignment for a Computer Networks course.

## Features

- **HTTP/1.1 Support** - Implements common HTTP methods: GET, HEAD, POST, PUT, DELETE, OPTIONS
- **Static File Serving** - Serves HTML, CSS, images, PDFs, and other static content
- **Script Execution** - Supports Python and PHP scripts with URL argument parsing
- **API Integration** - Consumes external APIs and caches responses locally
- **File Management** - RESTful PUT/DELETE operations in the shared directory
- **Logging** - Dedicated logger process with timestamped access logs
- **Configuration** - Customizable via `server.conf` file
- **Process Pool** - Prefork worker model for handling concurrent connections

## Project Structure

```
.
├── src/              # Server source code
├── srclib/           # Library source files (HTTP parser, networking)
├── include/          # Header files
├── lib/              # Compiled library
├── tests/            # Test files
├── www/              # Static web content (served root)
├── Makefile          # Build configuration
├── server.conf       # Server configuration
├── memoria.md        # Project documentation (Spanish)
└── diseño.md         # Design documentation (Spanish)
```

## Building

Prerequisites: GCC, make

```bash
make
```

This compiles the server and generates the `server` binary.

### Build Options

```bash
make all      # Build server (default)
make clean    # Remove build artifacts
```

### Running Tests

```bash
make tests
./server_test_mundo
./server_concurrency_test
./test_parse_request
./test_get_url_args
```

### Memory Checking

```bash
make check_server   # Run server with Valgrind
```

## Running the Server

```bash
./server
```

The server will start listening on the configured port (default: `8080`).

### Configuration (`server.conf`)

```conf
port=8080
backlog=30
max_clients=30
server_root=www/
scripts_directory=scripts/
shared_directory=shared/
verbose=true
```

## Example Requests

### GET Static File

```bash
curl http://localhost:8080/index.html
```

### HEAD Request (headers only)

```bash
curl -I http://localhost:8080/image.png
```

### Execute Script with GET

```bash
curl "http://localhost:8080/scripts/hola.py?nombre=Mundo"
```

### POST to Script

```bash
curl -X POST -d "dato1=valor1&dato2=valor2" \
     http://localhost:8080/scripts/procesar.py
```

### Upload File (PUT)

```bash
curl -X PUT -d '{"clave": "valor"}' \
     http://localhost:8080/shared/datos.json
```

### Delete File

```bash
curl -X DELETE http://localhost:8080/shared/archivo.txt
```

### Consume API

```bash
curl http://localhost:8080/api/oracion-hoy
```

## Academic Context

This project was developed for the **Computer Networks** course at university. It demonstrates fundamental networking concepts including:

- TCP socket programming
- HTTP protocol implementation
- Process management and forking
- Inter-process communication
- File I/O and MIME type handling

## Documentation

- `memoria.md` - Detailed functionality and requirements
- `diseño.md` - Design decisions and architecture
