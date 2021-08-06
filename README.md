# Greenhouse

Server component of STM32 greenhouse project.

## Features

- HTTP Webserver

## WebServer

| IP | Port | Protocol |
|----|------|----------|
| 192.168.1.100 | 443 | HTTPS |

### Endpoints

| URL | Method | Status | Description |
|-----|--------|--------|-------------|
| `/` | `GET`  | `200`  | Homepage    |
| `/` | `POST` | `204`  | Destination for sensor data in CSV format |