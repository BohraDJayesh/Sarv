#include <iostream>
#include <Windows.h>
#include <cctype>
#include <thread>
#include <iomanip>
#include <WinUser.h>

// ANSI escape codes for cursor and scrolling control
const char* hideCursor = "\x1b[?25l";
const char* disableScrolling = "\x1b[?1049h\x1b[H";
const char* restoreScrolling = "\x1b[?1049l";
const char* showCursor = "\x1b[?25h";

#define WINDOWS_IGNORE_PACKING_MISMATCH
LONG consoleLength;
LONG consoleWidth;
HANDLE hConsole;
bool editMode = true;

//Hook for listening to keyLL.

void RelocateCursor(HANDLE hConsole, int X, int Y);
void moveCursorToLastRow();
void moveCursorToFirstRow();


void printAsciiArt() {
    std::cout << std::setw(3) << "  _________                   " << std::endl;
    std::cout << std::setw(3) << " /   _____/____ __________  __" << std::endl;
    std::cout << std::setw(3) << " \_____  \\__  \\_  __ \  \/ /" << std::endl;
    std::cout << std::setw(3) << " /        \/ __ \|  | \/\   / " << std::endl;
    std::cout << std::setw(3) << "/_______  (____  /__|    \_/  " << std::endl;
    std::cout << std::setw(3) << "        \/     \/             " << std::endl;
}

 void consoleDim( CONSOLE_SCREEN_BUFFER_INFO csbi) {
    //consoleLength = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
     consoleLength = csbi.dwSize.X;
     consoleWidth = csbi.dwSize.Y;
    //consoleWidth = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

 void RelocateCursor(HANDLE hConsole, int X, int Y) {
     COORD newPosition = { static_cast<SHORT>(X), static_cast<SHORT>(Y) };
     SetConsoleCursorPosition(hConsole, newPosition);
 }

void printBoundaries(HANDLE hConsole,CONSOLE_SCREEN_BUFFER_INFO csbi) {

    //Calculating center of the console or terminal.

    short centerX = ((csbi.dwSize.X) / 2);
    short centerY = ((csbi.dwSize.Y) / 2);

    for (int i = 1; i <= centerX-2; i++) {
        if (i == centerX - 2)
            std::cout << "SARV" << std::endl;
        else
        {
            std::cout << " ";
        }
    }
    
    //Now printing the whole box.

    for (int i = 0; i < 2*centerY - 1; i++)
    {
        if (i == 0) //to skip the first line for SARV test.
            continue;
        for (int j = 0; j < 2*centerX; j++) {

            if ((i == 1 || i == 2*centerY - 2))
                std::cout << char(219);
            else if (j == 0 || j == 2*centerX - 1)
                std::cout << char(186);
            else
                std::cout << " ";
        }
        std::cout << std::endl;
    }



}

DWORD ClearScreen() {
    HANDLE hStdOut;

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Fetch existing console mode so we correctly add a flag and not turn off others
    DWORD mode = 0;
    if (!GetConsoleMode(hStdOut, &mode))
    {
        return ::GetLastError();
    }

    // Hold original mode to restore on exit to be cooperative with other command-line apps.
    const DWORD originalMode = mode;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Try to set the mode.
    if (!SetConsoleMode(hStdOut, mode))
    {
        return ::GetLastError();
    }

    // Write the sequence for clearing the display.
    DWORD written = 0;
    PCWSTR sequence = L"\x1b[2J";
    if (!WriteConsoleW(hStdOut, sequence, (DWORD)wcslen(sequence), &written, NULL))
    {
        // If we fail, try to restore the mode on the way out.
        SetConsoleMode(hStdOut, originalMode);
        return ::GetLastError();
    }

    // To also clear the scroll back, emit L"\x1b[3J" as well.
    // 2J only clears the visible window and 3J only clears the scroll back.

    // Restore the mode on the way out to be nice to other command-line applications.
    SetConsoleMode(hStdOut, originalMode);

}

std::atomic<bool> exitRequested(false);

BOOL CtrlHandler(DWORD fdwCtrlType) {
    DWORD result = ERROR_SUCCESS;
    std::cout << restoreScrolling;
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        // Moving to end, and thein printing ctrl+c pressed.
        RelocateCursor(hConsole, 2, consoleLength-2);
        std::cout << "^C" << std::endl;
        
        //result = 
            ClearScreen();

        //if (result != 1) {
        //    std::cerr << "There are Resource allocated, Wait for few minutes !!!. "<<std::endl<<"Error Code : "  << result << std::endl;
        //    return static_cast<int>(result);
        //}

        exit(0);
        return TRUE; // Returning TRUE to prevent the default handler
    default:
        return FALSE;
    }
   
}

void moveCursorToFirstRow() {
    // Set the cursor position to the first row
    COORD cursorPosition = { 3, 2 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

void moveCursorToLastRow() {
    // Get the console screen buffer size
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    // Set the cursor position to the last row
    COORD cursorPosition = { 3, csbi.dwSize.Y - 1 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}


void printRestrictedArea( HANDLE hConsole, const char* buffer, DWORD bufferSize) {

    // Adjusting the loop indices to skip the first two and last two rows and columns
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int width = 2*csbi.dwSize.X;
    int height = csbi.dwSize.Y - 3;
    int bufferIndex = 2;

    //RelocateCursor(hConsole, 0, 2);
    
    for (int i = 0; i < height-1; i++) {
        bool isbreak = false;
        RelocateCursor(hConsole, 0, i + 2);

        for (int j = 0; j < width-1; j++) {
            if (j == 0) {
                std::cout << char(186);  // Vertical bar character
            }
            else if (j == width - 3) {
                break;  // Vertical bar character
            }
            else if (bufferIndex < static_cast<int>(bufferSize)) {
                // Print the character from the buffer
                if (buffer[bufferIndex] == '\n') {
                    // Handle newline characters by moving to the next line
                    std::cout << '\n';
                    RelocateCursor(hConsole, 1, i + 3);
                    j = 0; bufferIndex++;
                }
                else {
                    std::cout << buffer[bufferIndex++];
                }
            }
            else {
                isbreak = true;
                break;
            }
        }
        if (isbreak)
            break;
    }
}

void keyListens() {
    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) && editMode == true) {
            editMode = false;
            moveCursorToLastRow();
        }
        else if (GetAsyncKeyState('I') && editMode == false) {
            editMode = true;
            moveCursorToFirstRow();
        }
    }
}

int main(int argc, char* argv[]) {

    HCURSOR hCursor = LoadCursor(NULL, IDC_IBEAM);
    SetCursor(hCursor);


    HANDLE hThreadtoKey = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)keyListens, NULL, 0,NULL);


    std::cout << disableScrolling;

    //Checking if the arguments are defined are not.
    if (argc < 2) {
        printAsciiArt();
        std::cerr << "Usage: " << argv[0] << " <file_path> -<optional_Arguments>" << std::endl;
        getchar();
        exit(0);
    }

    // Event to continously monitor for ctrl+c event.

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);


    //Getting the information of cosole :

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi; 
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    consoleDim(csbi);

    printBoundaries(hConsole, csbi);

    // Relocating the cursor to the starting position of the window.
    RelocateCursor(hConsole, 2, 2);


    //Starting the IO operation.
    // Handling the file input and checking if the file exists or not, if it doesn't creating one, using OPEN_ALWAYS.

    std::string filePath = argv[1];
    HANDLE fileHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating or opening the file !! " << GetLastError() << std::endl;
        return 1;
    }
    DWORD fileSize = GetFileSize(fileHandle, NULL);
    char* buffer = new char[fileSize];

    DWORD bytesRead;

    //Reading and checking for bytes.
    if (ReadFile(fileHandle, buffer, fileSize, &bytesRead, NULL) && bytesRead == fileSize) {
        printRestrictedArea(hConsole, buffer, bytesRead);
    }
    else {
        std::cerr << "Error reading the file. Error code: " << GetLastError() << std::endl;
    }
    

    while (!exitRequested) {

        // Sleep to avoid busy-waiting and reduce CPU usage
        Sleep(100);
    };

    //Finaly deleting the allocated buffers.
    delete[] buffer;
    CloseHandle(hThreadtoKey);

   // Currently On - have to go for keythreads as the hooks are not for me, not experienced enough to use hooks that's all I can conclude from that.;
 
    // Re-enable cursor and scrolling
    std::cout << restoreScrolling<< showCursor;
}

