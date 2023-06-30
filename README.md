# DNSRelay

A DNS Relay Server Implementation with C

### Funtions
- User Host File supported
- Mutithreading with thread pool
- LRU Cache combined with Trie Tree + SQLite
- Mutiple Debug level log supported

### Project Structure


```
dns_relay/
|-- include/         # headers
|-- src/             # sources code
|   |-- common/      # cross-platform codes
|   |-- linux/       # codes for Linux
|   |-- windows/     # codes for Windows
|-- Makefile         # Makefile
|-- README.md        # README
```

### Usage
Build:
```
make
```
