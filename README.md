# Math Server

A Unix/Linux **client-server application written in C** for performing computational tasks over a network.

The project demonstrates socket programming and concurrent server design. A client sends computational requests to the server, which processes the requested operation and returns the result. The supported computations include **matrix inversion** and **K-means clustering**.

The server implementation explores process-based and POSIX-thread-based execution to handle computational work concurrently. The K-means functionality uses numerical data to perform clustering, making the project a combination of network programming, concurrency, and numerical computation in C.