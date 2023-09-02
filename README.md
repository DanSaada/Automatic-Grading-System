# Automatic-Grading-System
This is an automatic grading system for computer science student assignments, which compiles the files the students submitted, runs their programs, and assigns grades to a csv file based on comparing the student's output files to the expected output.

The system is consists of two parts:

## First part - File Comparison
The initial phase of building the system was to create a program, which is implemented within the compare_text.c file, that has the ability to compare files. This program takes two paths as input, which can be either relative or absolute, and proceeds to compare them. If the files are completely identical, meaning that every character holds the same value in the same position, the program will return 1. In cases where the files exhibit similarity, differing only in spaces or newline characters, the program will yield a return value of 3. However, if the files are neither similar nor identical, which means they are different, the program will return 2.
For example, all this files are similar:
![image](https://github.com/DanSaada/Automatic-Grading-System/assets/112869076/eec0a3b4-a7db-4bda-a1d3-9241a2dd1eb7)



