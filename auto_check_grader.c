// Dan Saada 208968560

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>

#define MAX_PATH_LENGTH 1024
#define MAX_LINE_LENGTH 1024

typedef struct {
    char folderPath[MAX_PATH_LENGTH];
    char inputFilePath[MAX_PATH_LENGTH];
    char outputFilePath[MAX_PATH_LENGTH];
} ConfigData;

/**
 * Prints an error message indicating that a system call has failed, using perror().
 * The message will include the given syscall name, followed by the error message
 * corresponding to errno. Then the program will exit with an error code of -1.
 *
 * @param syscall a string containing the name of the system call that has failed.
 * @param err The standard saved error fd
 */
void errorIn(char *syscall, int err) {
    // save the corrent error fd
    int tempErr = dup(STDERR_FILENO);
    // redirect errors to program console
    dup2(err, STDERR_FILENO);

    char errorMsg[256];
    strcpy(errorMsg, "Error in: ");
    strcat(errorMsg, syscall);
    strcat(errorMsg, "\n");

    if(write(2, errorMsg, strlen(errorMsg)) == -1){}

    // redirect back to errors.txt
    dup2(tempErr, STDERR_FILENO);
    close(tempErr);
}

/**
 * Prints an error message indicating that a system call has failed,
 * The message will include the given syscall name, then the program will exit with an error code of -1.
 *
 * @param syscall a string containing the name of the system call that has failed.
 */
void criticalError(char *syscall) {

    char errorMsg[256];
    strcpy(errorMsg, "Error in: ");
    strcat(errorMsg, syscall);
    strcat(errorMsg, "\n");

    if(write(2, errorMsg, strlen(errorMsg)) == -1){exit(-1);}
    exit(-1);
}

/**
 * Reads a path from a file descriptor and stores it in a given string buffer.
 * @param fileDescriptor The file descriptor to read from.
 * @param path The string buffer to store the path in.
 */
void readPath(int fileDescriptor, char *path) {
    char buffer[MAX_PATH_LENGTH];
    int bytesRead;
    int i = 0;
    while ((bytesRead = read(fileDescriptor, buffer + i, 1)) > 0) {
        if (buffer[i] == '\n') {
            buffer[i] = '\0';
            strcpy(path, buffer);
            return;
        }
        i += bytesRead;
    }
    if (bytesRead == -1) {
        criticalError("read");
    }
}

/**
 * Extracts the configuration data from a file at a given path.
 * @param configFilePath The path to the configuration file.
 * @param configData A pointer to a ConfigData struct to store the extracted data in.
 * @return A configData struct containing the data from the configuration file
 */
ConfigData configDataExtraction(const char *configFilePath) {
    ConfigData configData;
    // Open the configuration file
    int configFile = open(configFilePath, O_RDONLY);
    if (configFile < 0) {
        criticalError("open");
    }

    // Read the paths from the configuration file
    readPath(configFile, configData.folderPath);
    readPath(configFile, configData.inputFilePath);
    readPath(configFile, configData.outputFilePath);

    close(configFile);
    return configData;
}

/**
 * Check if a directory exist in the system.
 *
 * @param configData A ConfigData struct containing directory path
 */
void checkDirExist(ConfigData configData) {
    // Check if the directory path exists and is a directory
    struct stat path_stat;
    stat(configData.folderPath, &path_stat);
    if (!S_ISDIR(path_stat.st_mode)) {
        char errMsg[] = "Not a valid directory\n";
        if(write(STDERR_FILENO, errMsg, strlen(errMsg)) == -1){exit(-1);}
        exit(-1);
    }
}

/**
 * Check if input and output files exist in the system
 *
 * @param configData A ConfigData struct containing input and output file paths
 */
void checkFilesExist(ConfigData configData) {
    // Check if input file exists
    if (access(configData.inputFilePath, F_OK) == -1) {
        char errMsg[] = "Input file not exist\n";
        if(write(STDERR_FILENO, errMsg, strlen(errMsg)) == -1){exit(-1);}
        exit(-1);
    }

    // Check if output file exists
    if (access(configData.outputFilePath, F_OK) == -1) {
        char errMsg[] = "Output file not exist\n";
        if(write(STDERR_FILENO, errMsg, strlen(errMsg)) == -1){exit(-1);}
        exit(-1);
    }
}

/**
 * Searches for a C file in the specified directory.
 *
 * @param dirPath the directory path to search for the C file
 * @return a pointer to a string containing the path of the C file, or NULL if not found
 */
char *findCFile(char *dirPath, int err) {
    DIR *dir;
    struct dirent *dirEntry;
    char filePath[MAX_PATH_LENGTH];

    if ((dir = opendir(dirPath)) == NULL) {
        errorIn("opendir", err);
    }

    // iterate through the directory and search for the c file.
    while ((dirEntry = readdir(dir)) != NULL) {
        if (dirEntry->d_type == DT_REG && strlen(dirEntry->d_name) >= 2 &&
            strcmp(dirEntry->d_name + strlen(dirEntry->d_name) - 2, ".c") == 0) {
            snprintf(filePath, MAX_PATH_LENGTH, "%s/%s", dirPath, dirEntry->d_name);
            closedir(dir);
            return strdup(filePath);
        }
    }
    // didn't find a c file.
    closedir(dir);
    return NULL;
}

/**
 * Compiles a C file at a given path into an executable file.
 * @param cFilePath The path to the C file to compile.
 * @param exeFilePath The path to store the executable file at.
 * @param err The standard saved error fd
 * @return 0 if the compilation worked and 1 otherwise
 */
int compileCFile(const char *cFilePath, char *exeFilePath, int err) {
    pid_t pid;
    int status;
    char compiledFilePath[MAX_PATH_LENGTH];

    // Find the position of the last dot in the file path
    char *dotPos = strrchr(cFilePath, '.');
    // Calculate the length of the file name without the extension
    int fileNameLen = dotPos - cFilePath;
    // Copy the file name without the extension to the compiled file path
    strncpy(compiledFilePath, cFilePath,fileNameLen);
    // Add null character at the end of the copied string
    compiledFilePath[fileNameLen] = '\0';
    // Append the ".out" extension to the compiled file path
    strcat(compiledFilePath, ".out");
    // Copy the compiled file path to the exe file path
    strcpy(exeFilePath, compiledFilePath);

    const char *const args[] = {"gcc", "-o", compiledFilePath, cFilePath, NULL};

    pid = fork();
    if (pid < 0) {
        errorIn("fork", err);
    } else if (pid == 0) {  // Child process
        if (execvp("gcc", (char *const *) args) == -1) {
            errorIn("execvp", err);
        }
        exit(0);
    } else {  // Parent process
        wait(&status);
        if (status != 0) {
            return 1;
        }
        return 0;
    }
}

/**
 * Delete the executable file that was compiled as part of the program's logic.
 * @param exeFilePath The path to store the executable file at.
 * @param err The standard saved error fd
 */
void deleteExeFile(char *exeFilePath, int err) {
    // Extract the directory name and execution file name with the .out suffix
    char exeFileName[MAX_PATH_LENGTH];
    char *lastSlash = strrchr(exeFilePath, '/');
    if (lastSlash == NULL) {
        strcpy(exeFileName, exeFilePath);
    } else {
        strcpy(exeFileName, lastSlash + 1);
    }

    // Save the current working directory
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, MAX_PATH_LENGTH) == NULL) {
        errorIn("getcwd", err);
        return;
    }

    // Change to the directory containing the exe file
    char dirName[MAX_PATH_LENGTH];
    strncpy(dirName, exeFilePath, lastSlash - exeFilePath + 1);
    dirName[lastSlash - exeFilePath + 1] = '\0';
    if (chdir(dirName) == -1) {
        errorIn("chdir", err);
        return;
    }

    // Change the permissions of the file to allow writing and executing
    chmod(exeFileName, S_IRWXU | S_IRWXG | S_IRWXO);

    // Remove the file
    if (access(exeFileName, F_OK) == 0) {
        if (remove(exeFileName) == -1) {
            errorIn("remove", err);
        }
    }

    // Change back to the original working directory
    if (chdir(cwd) == -1) {
        errorIn("chdir", err);
        exit(-1);
    }
}

/**
 * Redirect the standard file descriptors to a given file descriptors.
 * @param inputFd The input file descriptor.
 * @param outputFd The output file descriptor.
 * @param err The standard saved error fd
 */
void redirectFDs(int inputFd, int outputFd, int err) {
    // Redirect input and output to the appropriate files
    if (dup2(inputFd, STDIN_FILENO) == -1) {
        errorIn("dup2", err);
    }

    if (dup2(outputFd, STDOUT_FILENO) == -1) {
        errorIn("dup2", err);
    }
}

/**
 * Runs a program at a given path with a specified input file and output file.
 * @param programPath The path to the program executable.
 * @param inputFilePath The path to the input file.
 * @param outputFilePath The path to the output file.
 * @param err The standard saved error fd
 * @return The time in milliseconds that the program took to run, or -1 if the program did not terminate normally.
 */
int runProgram(const char *programPath, const char *inputFilePath, const char *outputFilePath, int err) {
    pid_t pid;
    int inputFd, outputFd, status;
    struct timeval start, end;

    // Open the input and output files
    inputFd = open(inputFilePath, O_RDONLY);
    if (inputFd < 0) {
        errorIn("open", err);
    }

    outputFd = open(outputFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (outputFd < 0) {
        errorIn("open", err);
    }

    gettimeofday(&start, NULL);
    pid = fork();
    if (pid < 0) {
        errorIn("fork", err);
    } else if (pid == 0) {  // Child process

        // Redirect input and output to the appropriate files
        redirectFDs(inputFd, outputFd, err);

        const char *const args[] = {programPath, NULL};
        if (execvp(programPath, (char *const *) args) == -1){
            errorIn("execvp", err);
        }
        exit(0);

    } else {  // Parent process
        wait(&status);
        gettimeofday(&end, NULL);

        if (WIFEXITED(status)) {
            // Close the input and output files
            if (close(inputFd) == -1) {
                errorIn("close", err);
            }
            if (close(outputFd) == -1) {
                errorIn("close", err);
            }
            int runningTime = (end.tv_sec - start.tv_sec);
            return runningTime;
        } else {
            // Close the input and output files
            if (close(inputFd) == -1) {
                errorIn("close", err);
            }
            if (close(outputFd) == -1) {
                errorIn("close", err);
            }
        }
    }
}


/**
 * Compares the contents of two files and returns the result of the comparison.
 * @param file1Path The path to the first file to compare.
 * @param file2Path The path to the second file to compare.
 * @param err The standard saved error fd
 * @return An integer representing the result of the comparison: 0 if the files are identical, 1 if they are different,
 * 2 if there was an error opening one of the files, 3 if the files are similar but not identical.
 */
int compareFiles(const char *file1Path, const char *file2Path, int err) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        errorIn("fork", err);
    } else if (pid == 0) {  // Child process
        const char *const args[] = {"comp.out", file1Path, file2Path, NULL};
        if (execvp("./comp.out", (char *const *) args) == -1) {
            errorIn("execvp", err);
        }
        exit(0);
    } else {  // Parent process
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exitCode = WEXITSTATUS(status);
            return exitCode;
        } else {
            return 0;
        }
    }
}

/**
 * Writes the result of the student's score to a results file.
 *
 * @param resultsFile The file descriptor of the results file
 * @param username The username of the the student who is being graded
 * @param score The score achieved by the student
 * @param reason A string describing the reason why the student got this score
 */
void writeResult(int resultsFile, const char *username, int score, char *reason) {
    // Write the result to the results file
    char resultLine[MAX_LINE_LENGTH];
    int resultLineLength = snprintf(resultLine, MAX_LINE_LENGTH, "%s,%d,%s\n", username, score, reason);
    if (write(resultsFile, resultLine, resultLineLength) == -1) {}
}

/**
 * Writes the result of a student's program to the results file.
 * @param resultsFile The file descriptor of the results file to write to.
 * @param username The username of the student.
 * @param compareResult The result of comparing the student's output to the expected output.
 * @param runningTime The running time of the student's program.
 * @param compilationResult The result of compiling the student's C file.
 */
void calcScore(int resultsFile, const char *username, int compareResult, int runningTime, int compilationResult) {
    char reason[MAX_LINE_LENGTH];
    int score;

    // Check if there was a compilation error
    if (compilationResult == 1) {
        score = 10;
        strcpy(reason, "COMPILATION_ERROR");
    }
    // Check if the compiled C file ran for more than 5 seconds
    else if (runningTime > 5) {
        score = 20;
        strcpy(reason, "TIMEOUT");
    }
    // Check if the output is correct
    else if (compareResult == 1) {
        score = 100;
        strcpy(reason, "EXCELLENT");
    }
    // Check if the output is not correct but is similar
    else if (compareResult == 3) {
        score = 75;
        strcpy(reason, "SIMILAR");
    }
    // Check if the output is not correct
    else if (compareResult == 2) {
        score = 50;
        strcpy(reason, "WRONG");
    }
    writeResult(resultsFile, username, score, reason);
}

/**
* Runs the program logic, which iterates through the user's directory, compiles and runs the C file,
* compares the user's output to the expected output, calculates the score, and writes the result to the
* results.csv file.
* @param configData A ConfigData struct that contains the necessary configuration data.
* @param resultFile The file descriptor of the results.csv file to write the results to.
* @param err The standard saved error fd
*/
void programLogic(ConfigData configData, int resultFile, int err) {
    // Opens the directory specified in the folderPath
    DIR *dir;
    struct dirent *dirEntry;
    if ((dir = opendir(configData.folderPath)) == NULL) {
        errorIn("opendir", err);
    }

    // iterate through the user's directory
    while ((dirEntry = readdir(dir)) != NULL) {
        // the readdir function returns also the directories "." and ".." which we want to avoid.
        // we also want to avoid from file that are not directories.
        if (dirEntry->d_type == DT_DIR && strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0) {
            // concat the path of the user's directory with one of the directories inside it
            char subfolderPath[MAX_PATH_LENGTH];
            strcpy(subfolderPath, configData.folderPath);
            strcat(subfolderPath, "/");
            strcat(subfolderPath, dirEntry->d_name);

            // search C file.
            char *cFilePath = findCFile(subfolderPath, err);

            // no C file
            if (cFilePath == NULL) {
                writeResult(resultFile, dirEntry->d_name, 0, "NO_C_FILE");
            } else {
                // compile C file.
                char exeFilePath[MAX_PATH_LENGTH];

                int compilationResult = compileCFile(cFilePath, exeFilePath, err);

                // run C file.
                char programOutput[MAX_LINE_LENGTH] = "user-output.txt";
                char programPath[MAX_PATH_LENGTH];
                strcpy(programPath, exeFilePath);
                int runningTime = runProgram(programPath, configData.inputFilePath, programOutput, err);

                // compare between the user's output and the expected output.
                int compareResult = compareFiles(configData.outputFilePath, programOutput, err);

                // Calculate the score and write to results.csv file
                calcScore(resultFile, dirEntry->d_name, compareResult, runningTime, compilationResult);

                // delete the user's outputput file
                if (remove(programOutput) == -1) {
                    errorIn("remove", err);
                }
                // delete compiled file
                deleteExeFile(exeFilePath, err);
            }
        }
    }
    closedir(dir);
}

/**
* Close all the file descriptors that were opened in the course of the program.
* @param resultFile The file descriptor of the results.csv file to write the results to.
* @param errorsFile The file descriptor of the errors.txt file to write the errors to.
* @param err The standard saved error fd
*/
void closeFDs(int resultsFile, int errorsFile, int err) {
    if(close(resultsFile) == -1) {errorIn("close", err);}
    if(close(errorsFile) == -1) {errorIn("close", err);}
    if(close(err) == -1) {errorIn("close", err);}
}

/**
* Making all the preparations for to run the program logic.
* @param configFilePath The path to the configuration file.
*/
void autoCheckGrader(const char *configFilePath) {
    // Extract configuration data from file
    ConfigData configData = configDataExtraction(configFilePath);

    // validate configuration file data
    checkDirExist(configData);
    checkFilesExist(configData);

    // Create results.csv file
    int resultsFile = open("results.csv", O_WRONLY | O_CREAT, 0644);
    if (resultsFile < 0) {
        criticalError("open");
    }

    // saving the standard error fd
    int err = dup(STDERR_FILENO);

    // open errors output file
    int errorsFile = open("errors.txt", O_CREAT | O_RDWR | O_APPEND, 0777);
    if (errorsFile < 0) {
        criticalError("open");
    }
    dup2(errorsFile, STDERR_FILENO);

    programLogic(configData, resultsFile, err);

    closeFDs(resultsFile, errorsFile, err);
}

int main(int argc, char *argv[]) {
    // Validate arguments
    if (argc != 2) {
        exit(-1);
    }

    //run the program
    autoCheckGrader(argv[1]);

    return 0;
}
