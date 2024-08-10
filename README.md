Data Structures Used:

struct Process: This structure stores comprehensive information like pid, name, size, etc., related to a process. It includes a map (addrToVals) that associates virtual addresses with values for each process.

mainMemFreePages: A set that keeps track of available page numbers in the main memory that have not been allocated to any process.

virtMemFreePages: Another set that maintains available page numbers in virtual memory that have not been allocated to any process.

procsInMainMem: A vector that holds references to processes currently residing in the main memory.

procsInVirtMem: A vector that contains references to processes residing in the virtual memory.

LRUMainMem: A vector that serves as a queue implementing the Least Recently Used (LRU) algorithm. It stores references to processes that have been executed in the main memory.

Assumptions:

Swapping Process Out: When attempting to swap a process out of main memory, if there is insufficient space in virtual memory, the request will not be processed. It's assumed that virtual memory space is finite.

Swapping Process In: When trying to swap a process into main memory, if there isn't enough space even after swapping out all processes in the LRU queue, then the processes that were loaded earliest or the process with the lowest pid will be swapped out. This assumption ensures efficient memory management.

No Space for LRU Process: An error will be raised if the least recently used process cannot be swapped out to virtual memory due to insufficient space. This error condition ensures that memory allocation issues are properly handled and reported.

