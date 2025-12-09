# HTTP Daemon (httpd)

A lightweight, concurrent HTTP server implementation written in C. This project supports basic HTTP/1.1 methods, Virtual Hosts (VHosts), and daemonization.

## Features

- **HTTP/1.1 Support**: Handles `GET` and `HEAD` requests.
- **Virtual Hosts (VHosts)**: Supports multiple domains/servers on different ports or IPs from a single instance.
- **Concurrency**: Uses `fork()` to handle multiple client connections simultaneously.
- **Daemon Mode**: Can run in the background as a daemon process.
- **Configuration**: Flexible INI-style configuration file.
- **Logging**: Request and error logging capabilities.

## Building

To build the project, simply run:

```bash
make
```

This will generate the `httpd` executable.

To clean up build artifacts:

```bash
make clean
```

## Usage

The server is controlled using the `-a` flag followed by a command (`start`, `stop`, `restart`) and a configuration file.

### Starting the Server

```bash
./httpd -a start <path/to/config.conf>
```

### Stopping the Server

```bash
./httpd -a stop <path/to/config.conf>
```

### Restarting the Server

```bash
./httpd -a restart <path/to/config.conf>
```

## Configuration

The configuration file uses a simple structure. You can define global settings and multiple VHost blocks.

**Example `config.conf`:**

```ini
[global]
log_file = server.log
log = true
pid_file = /tmp/httpd.pid

[[vhosts]]
server_name = server1
port = 8080
ip = 127.0.0.1
root_dir = www/site1/
default_file = index.html

[[vhosts]]
server_name = server2
port = 8081
ip = 127.0.0.1
root_dir = www/site2/
default_file = index.html
```

## Testing

### Automated Tests

A Python test suite is included to verify VHost isolation and concurrency.

```bash
python3 tests/test_vhosts.py
```

### Interactive "Multiverse" Demo

Experience the VHost capabilities visually with the included demo.

1.  **Start the demo server:**
    ```bash
    ./httpd -a start demo/demo.conf
    ```
2.  **Open your browser:**
    - [http://localhost:8080](http://localhost:8080) (Cyberpunk Theme)
    - [http://localhost:8081](http://localhost:8081) (Nature Theme)
3.  **Stop the demo:**
    ```bash
    ./httpd -a stop demo/demo.conf
    ```
