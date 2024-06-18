# Port Scanner

Simple command-line tool implemented in C++ for scanning ports on a given host. 
It utilizes socket programming to check the status (open, closed) of TCP ports.

Features
* Scan Range: Specify a range of ports to scan, or scan all ports from 1 to 65535.
* Multi-threaded: Utilizes multiple threads for faster scanning.
* Output: Prints the results of the scan to the console.