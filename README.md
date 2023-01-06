This program wraps input/output systems calls so that
they may be used efficiently. For instance, I use a buffer 
to store bytes whenever the read system call is invoked so 
that subsequent reads obtain bytes from the buffer instead
of invoking costly system calls. The buffer reads more bytes
than the user asked for for every system call. I only allow the 
system call to go through if there is nothig else to read from the
buffer. 

I do the same for the write system call. The buffer stores bytes 
to be written every time the user invokes a write system call. 
The program only allows writing once the buffer is full or when 
there is no more bytes to add to the buffer. This avoids invoking
coslty system call unneccessarily. 

To compile the program run make. To remove unwanted files run
make clean. 
