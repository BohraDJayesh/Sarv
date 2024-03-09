#include <iostream>
#include <Windows.h>
#include <cctype>
#include <thread>
#include <iomanip>
#include <WinUser.h>
#include <vector>

// ANSI escape codes for cursor and scrolling control
const char* hideCursor = "\x1b[?25l";
const char* disableScrolling = "\x1b[?1049h\x1b[H";
const char* restoreScrolling = "\x1b[?1049l";
const char* showCursor = "\x1b[?25h";
DWORD fileSize = 0;
std::atomic<bool> exitRequested(false);

#define WINDOWS_IGNORE_PACKING_MISMATCH
LONG consoleLength;
LONG consoleWidth;
HANDLE hConsole;
bool editMode = true;

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
     consoleLength = csbi.dwSize.X;
     consoleWidth = csbi.dwSize.Y;
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
    DWORD mode = 0;
    if (!GetConsoleMode(hStdOut, &mode))
    {
        return ::GetLastError();
    }
    const DWORD originalMode = mode;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hStdOut, mode))
    {
        return ::GetLastError();
    }
    DWORD written = 0;
    PCWSTR sequence = L"\x1b[2J";
    if (!WriteConsoleW(hStdOut, sequence, (DWORD)wcslen(sequence), &written, NULL))
    {
        SetConsoleMode(hStdOut, originalMode);
        return ::GetLastError();
    }
    SetConsoleMode(hStdOut, originalMode);

}

BOOL CtrlHandler(DWORD fdwCtrlType) {
    DWORD result = ERROR_SUCCESS;
    std::cout << restoreScrolling;
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        // Moving to end, and thein printing ctrl+c pressed.
        RelocateCursor(hConsole, 2, consoleLength-2);
        std::cout << "^C" << std::endl;
            ClearScreen();
        exit(0);
        return TRUE; // Returning TRUE to prevent the default handler
    default:
        return FALSE;
    }
   
}

void moveCursorToFirstRow() {   
    COORD cursorPosition = { 3, 2 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

void moveCursorToLastRow() {
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    COORD cursorPosition = { 3, csbi.dwSize.Y - 1 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

void printRestrictedArea(HANDLE hConsole, std::vector<char>& buffer, DWORD bufferSize) {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int width = 2 * csbi.dwSize.X;
    int height = csbi.dwSize.Y - 3;
    int bufferIndex = 2;

    for (int i = 0; i < height - 1; i++) {
        bool isbreak = false;
        RelocateCursor(hConsole, 0, i + 2);

        for (int j = 0; j < width - 1; j++) {
            if (j == 0) {
                std::cout << char(186);  
            }
            else if (j == width - 3) {
                break; 
            }
            else if (bufferIndex < static_cast<int>(bufferSize)) {
                if (static_cast<char>(buffer[bufferIndex]) == '\n') {
                    std::cout << '\n';
                    
                    //Based on the \n and remaining length on the screen, inserting spaces to map correctly curson to the buffer.
                    
                    GetConsoleScreenBufferInfo(hConsole, &csbi);
                    COORD cursorPos = csbi.dwCursorPosition;
                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPos);
                    int spaceNeeded = (width - 3) - cursorPos.X;
                    buffer.insert(buffer.begin() + cursorPos.X, spaceNeeded, ' ');

                    
                    RelocateCursor(hConsole, 1, i + 3);
                    j = 0; 
                    bufferIndex += spaceNeeded;
                }
                else {
                    std::cout << static_cast<char>(buffer[bufferIndex++]);
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

//Incomplete..
void doScroll(char key) {
    if (key == 'u') {
        //Will start later, currently working on the edit part.
    }
}

void editScreen(std::vector<char>& buffer) {
    INPUT_RECORD inputRecord;
    DWORD numEvents;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD cursorPosition = csbi.dwCursorPosition;
    while (true) {
        INPUT_RECORD inputRecord;
        DWORD numEvents;

        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inputRecord, 1, &numEvents);
        if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
            char inputChar = inputRecord.Event.KeyEvent.uChar.AsciiChar;
            if (isalnum(inputChar)) {
                ClearScreen();
                printBoundaries(hConsole, csbi);
                buffer.insert(buffer.begin() + cursorPosition.X, inputChar);
                printRestrictedArea(hConsole, buffer, buffer.size());
            }
        }

    }
}

void keyListens(std::vector<char>& buffer) {
    static HANDLE hThreadtoEdit;
    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) && editMode == true) {
            editMode = false;
            CloseHandle(hThreadtoEdit);
            moveCursorToLastRow();
        }
        else if (GetAsyncKeyState('I') && editMode == false) {
            editMode = true;
            hThreadtoEdit = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)editScreen, &buffer, 0, NULL);
            moveCursorToFirstRow();
        }
        else if (GetAsyncKeyState(VK_DOWN) && editMode == true) {
            doScroll('u');
        }
    }
}





int main(int argc, char* argv[]) {
    std::cout<<disableScrolling;
    HCURSOR hCursor = LoadCursor(NULL, IDC_IBEAM);
    SetCursor(hCursor);

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




    //std::cout << disableScrolling;
    if (argc < 2) {
        printAsciiArt();
        std::cerr << "Usage: " << argv[0] << " <file_path> -<optional_Arguments>" << std::endl;
        getchar();
        exit(0);
    }

    std::string filePath = argv[1];
    HANDLE fileHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating or opening the file !! " << GetLastError() << std::endl;
        return 1;
    }
    fileSize = GetFileSize(fileHandle, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Error getting the size of the file. Error code: " << GetLastError() << std::endl;
        CloseHandle(fileHandle);
        return 1;
    }
    char* bufferOriginal = new char[fileSize];
    
    //std::vector<char> buffer(fileSize);
    
    DWORD bytesRead;
    HANDLE hThreadtoKey;
    
    //Reading and checking for bytes.
    if (ReadFile(fileHandle, bufferOriginal, fileSize, &bytesRead, NULL) || bytesRead != fileSize) {

        std::vector<char> buffer(bufferOriginal, bufferOriginal + fileSize);

        printRestrictedArea(hConsole, buffer, fileSize);
        hThreadtoKey = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)keyListens, &buffer, 0, NULL);
        
    }
    else {

        std::cerr << "Error reading the file. Error code: " << GetLastError() << std::endl;
        CloseHandle(fileHandle);
        return 1;
       
    }


    while (!exitRequested) {

        // Sleep to avoid busy-waiting and reduce CPU usage
        Sleep(100);
    };

    delete[] bufferOriginal;
    CloseHandle(hThreadtoKey);

 
    std::cout << restoreScrolling<< showCursor;
}

