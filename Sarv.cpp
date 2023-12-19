#include <iostream>
#include <Windows.h>
#include <cctype>

#define WINDOWS_IGNORE_PACKING_MISMATCH
SHORT consoleLength;
SHORT consoleWidth;

 void *consoleDim( CONSOLE_SCREEN_BUFFER_INFO csbi) {
    consoleLength = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
    consoleWidth = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

// Function to print boundaries and the name "Surv" at top.

void printBoundaries(HANDLE hConsole,CONSOLE_SCREEN_BUFFER_INFO csbi) {

    //Calculating center of the console or terminal.

    short centerX = ((csbi.dwSize.X) / 2);
    short centerY = ((csbi.dwSize.Y) / 2);

    for (int i = 1; i < centerX-2; i++) {
        if (i == centerX - 2)
            std::cout << "SARV" << std::endl;
    }
    
    //Now printing the whole box.

    for (int i = 0; i < consoleLength-1; i++)
    {
        for (int j = 0; j < consoleWidth; j++) {
            if (i != 0 && i != consoleLength - 1) {
                if ((i == 1 || i == consoleLength - 2))
                    std::cout << "-";
                else if (j == 1 || j == consoleWidth - 1)
                    std::cout << "|";
                else
                    std::cout << " ";
            }
        }
        std::cout << std::endl;
    }



}


// Function to load a file and return it's handle/pointer.

HANDLE OpenFile(const wchar_t* filePath) {
    HANDLE hFile = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cout << "Error opening file. Error code: " << error << std::endl;
        return NULL;
    }
    return hFile;
}

//Checkinf if the file exists or not. !!

bool FileOrDirectoryExists(const char* filePath) {
    DWORD attributes = GetFileAttributesA(filePath);
    return (attributes != INVALID_FILE_ATTRIBUTES);
}


// Function to read file 


int main(int argc, char* argv[]) {


    //Getting the information of cosole :

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi; // Stores information regarding console window, like size, width, height etc.
    GetConsoleScreenBufferInfo(hConsole, &csbi);



    // Calling printBoundaries function to print the boundaries accross the terminal.

    printBoundaries(hConsole, csbi);

    //Reading the file, checking if it's exists or not, creating one.

    const char* narrowString = argv[1];
    int wideStringLength = MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, NULL, 0);
    wchar_t* filePath = new wchar_t[wideStringLength];
    MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, filePath, wideStringLength);



    if(FileOrDirectoryExists(argv[1])) {
        std::cout<<"File Doesn't Exists.."<<std::endl;
        std::cout<<"Press Enter to create one !!"<<std::endl;

        if(getchar())
            OpenFile(filePath);


    }
}

