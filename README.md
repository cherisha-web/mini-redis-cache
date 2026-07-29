
# Mini-Redis: In-Memory Cache Server in C++

A key-value cache server built from scratch in C++, implementing the same core ideas behind
real systems like Redis and Memcached — an in-memory store, LRU eviction, TCP networking,
and concurrent client handling.


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
  g++ -std=c++17 -o cache server.cpp
 ./cache
```

## Example usage

```
> SET name Cherry
OK
Cherry
> DELETE name
OK
```

*(e.g. `./cache_client SET name Cherry`.)*

## Design decisions & what I learned

- Why a unordered_map + doubly linked list, and not some other structure, for LRU?
DLL (doubly linked list) maintains the order of usage allowing to insert a node in the front and remove from the back in O(1) time, unordered map helps look up values and delete nodes in O(1) average time instead of using DLL alone which will take O(n).

- What race condition did you actually hit while adding multi-threading, and how did you fix it?
Since multiple client threads share the same unordered_map and doubly linked list, two threads could execute put(), get(), or DELETE() simultaneously. For example, two put() operations could both modify head->frontptr at the same time, corrupting the linked list or leaving the cache in an inconsistent state. I fixed this by protecting the critical section with a global std::mutex and std::lock_guard, so only one thread modifies the shared cache at a time.

- What would break first if this had to handle real production traffic, and why?
There are a couple of limitations in the current implementation. First, if a key or value contains spaces, stringstream splits the input on whitespace, so those keys or values are parsed incorrectly. Second, every successful GET, SET, and DELETE rewrites the entire cache to disk to preserve the LRU order. This works for a small project, but it would become inefficient for a large cache or a high request rate because of the frequent disk writes.

   
## Resources used

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — sockets
- [LeetCode: LRU Cache](https://leetcode.com/problems/lru-cache/) — core data structure
- [cppreference: std::thread](https://en.cppreference.com/w/cpp/thread/thread) / [std::mutex](https://en.cppreference.com/w/cpp/thread/mutex) — concurrency
