Assignment 2 - Multithreading
=============

Study the example program in the attached file. This program creates a new thread. Both the main thread and the new thread then increment the variable counter infinitely. After incrementing the value of counter, each thread prints a message showing the value of the variable. One of the threads exits when it finds that the counter value has exceeded 25. Run the program and explain the output. Why do the print statements stop appearing after a certain point in the program ? Explain.

1. Compile and run the program on a Linux system. Find out what results it produces and explain the results.
Compile as follows:
$ gcc assignment_multithreading.c -o assignment_mt -lpthread 

Run using the following command
$ ./assignment_mt

2. Modify the program and write a correct version that fixes the problem that you just discovered. Explain how you fixed the program.

Please submit a zip file labelled as Assignment4_Yourname.zip. It should include a PDF file with explanations to questions above and a C source file with the modified program.