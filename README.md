# MS12-020
 Remote Desktop Protocol Vulnerability

I will likely not continue working on this exploit further as this is very much infeasible. I always wondered why no one wrote up an exploit for this bug even a decade later. To put it bluntly, I understand now.

What makes this bug in particular so difficult is the constraints you're forced to work with. Not only are you exploiting an extremely tight window to achieve a UAF, you are also limited in what memory you can use. In this case I attempted to write a privilege escalation exploit under the assumption that an LPE is easier (for context, take a look at SMBGhost). However, because the exploit process is not directly interacting with the driver (rather sending a packet to localhost and having the Service Host deal with it), this is even harder than I could have imagined. Any user-mode memory that is allocated from the exploit process will not be seen from the driver's perspective as it is working within the Service Host process' address space.

There is one possible solution remaining that I can think of, but it is extremely difficult if not downright impossible:
Find a memory location anywhere in kernel-space that contains data that will suffice for this exploit. This could work, but one would need to find a function that would be ideal to call **AND** have a multitude of pointers somehow leading to the function you want to call.

An actor *could* try to allocate some memory in the paged pool after ensuring it's deterministic and copy some user-data to it, but then one has to be concerned about allocation ordering and some quirks with the heap allocator itself. But with this approach, one has to find a well-controlled paged pool object, ensure it's 0xA0 bytes in size, and somehow fit all of the kernel shellcode into the heap (and find its address or otherwise hard-code it in some way). The exploit's reliability is already *very* low (around the ~10% success rate mark), and this approach *will* tank it further, on top of all the other problems described.

With that said, I think it's fair to throw in the towel for this exploit in particular and focus on something else.
