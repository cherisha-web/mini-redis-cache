
# Mini-Redis: In-Memory Cache Server in C++

A key-value cache server built from scratch in C++, implementing the same core ideas behind
real systems like Redis and Memcached — an in-memory store, LRU eviction, TCP networking,
and concurrent client handling.

> Status: in progress. This README reflects what's actually built so far — see checklist below.

## What it does

Stores key-value pairs in memory for fast access, evicts the least recently used entries when
full, and (once networking is added) lets client programs connect over TCP to SET, GET, and
DELETE keys — similar to how a real application would talk to a caching layer sitting in front
of a database.

## Features

- [x] Core in-memory store — SET / GET / DELETE using a hash map
- [x] Disk-based persistence — save on shutdown, reload on startup
- [x] LRU eviction — hash map + doubly linked list, O(1) get/set/evict
- [x] TCP networking — client-server communication over sockets
- [x] Concurrent client handling — multi-threaded server with mutex-protected shared state



## Architecture

```
Client
   |
TCP Socket
   |
Server
   |
+------------------------+
| HashMap<Key, Iterator> |  --> O(1) lookup
| Doubly Linked List     |  --> tracks MRU/LRU order
+------------------------+
   |
Disk file (persistence)
```

- The hash map stores an **iterator** into the linked list, not the value directly — this is
  what makes moving a node to the front (marking it "recently used") an O(1) operation instead
  of a search.
- The linked list's head is the most recently used (MRU) entry; the tail is the least recently
  used (LRU) — the one evicted first when the cache is full.

## Tech stack

- C++ (STL: `unordered_map`, list-based structures, `fstream`)
- POSIX sockets for TCP networking
- `std::thread` + `std::mutex` for concurrency

## Build & run

```bash
g++ -std=c++17 -pthread -o cache_server src/main.cpp
./cache_server
```

*(Update this once you have an actual build setup — CMake, multiple source files, etc.)*

## Example usage

```
> SET name Cherry
OK
> GET name
Cherry
> DELETE name
OK
```

*(Once networking is added, show the client-side usage here too — e.g. `./cache_client SET name Cherry`.)*

## Design decisions & what I learned

- Why a hash map + doubly linked list, and not some other structure, for LRU?
- What race condition did you actually hit while adding multi-threading, and how did you fix it?
- What would break first if this had to handle real production traffic, and why?
   to compile : g++ -std=c++17 -o cache main.cpp
   to run : ./cache
## Resources used

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — sockets
- [LeetCode: LRU Cache](https://leetcode.com/problems/lru-cache/) — core data structure
- [cppreference: std::thread](https://en.cppreference.com/w/cpp/thread/thread) / [std::mutex](https://en.cppreference.com/w/cpp/thread/mutex) — concurrency
