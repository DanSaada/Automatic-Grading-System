# Automatic-Grading-System
This is an automatic grading system for computer science student assignments, which compiles the files the students submitted, runs their programs, and assigns grades to a csv file based on comparing the student's output files to the expected output.

The system is consists of two parts:

## First part - File Comparison
The initial phase of building the system was to create a program, which is implemented within the compare_text.c file, that has the ability to compare files. This program takes two paths as input, which can be either relative or absolute, and proceeds to compare them. If the files are completely identical, meaning that every character holds the same value in the same position, the program will return 1. In cases where the files exhibit similarity, differing only in spaces or newline characters, the program will yield a return value of 3. However, if the files are neither similar nor identical, which means they are different, the program will return 2.

For example, all this files are similar:

![image](https://github.com/DanSaada/Automatic-Grading-System/assets/112869076/4a7c1a27-6ba5-49b8-b1b7-95fca2d6f86e)

## Second part - Automatic Grading System
The second phase was to build the system itself.

Course of the program:

The system receives a configuration file as an argument, containing three lines describing the paths in which the system should look for the files programmed by the students, the input file and the correct output file. It then changes the file descriptors of the program in accordance.
  
 ![image](https://github.com/DanSaada/Automatic-Grading-System/assets/112869076/5595cd34-5a1a-442b-8714-31a547e37bb4)

The system opens the directory specified in the folder path and iterate through the students's directory and for each student:
* Search for a C file - The system is aware that the students have written programs in a specific programming language, so it actively searches for files written in that language.
* Compiles the C file - The system creates a child process to compile the C file separately from the parent process.
* Runs the C file - The system runs the compiled C file with the input file from the configuration file.
* Comparison phase - The system compares between the user's output and the expected output file from the configuration file.
* Calculate and grade - The system calculates the score of the student and write to results.csv file.
* Delete - The system deletes the user's outputput file and compiled file because they have no more use.

The result are shown in the results.csv file created by the program.

## Installing And Executing
    
To clone and run this application, you'll need [Git](https://git-scm.com) installed on your computer.
  
From your command line:

  
```bash
# Clone this repository.
$ git clone https://github.com/DanSaada/Automatic-Grading-System

# Go into the repository.
$ cd Automatic-Grading-System

# Compile the first part.
$ gcc -o comp.out compare_text.c

# Compile the second part.
$ gcc auto_check_grader.c

# Run the program.
 ./a.out autoGraderTest/conf.txt
```

## Author
- [Dan Saada](https://github.com/DanSaada)
