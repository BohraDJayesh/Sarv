#include <iostream>
#include <Windows.h>
#include <cctype>
#include <thread>
#include <iomanip>

// ANSI escape codes for cursor and scrolling control
const char* hideCursor = "\x1b[?25l";
const char* disableScrolling = "\x1b[?1049h\x1b[H";
const char* restoreScrolling = "\x1b[?1049l";
const char* showCursor = "\x1b[?25h";

#define WINDOWS_IGNORE_PACKING_MISMATCH
LONG consoleLength;
LONG consoleWidth;
HANDLE hConsole;


//function to print ascii art of Sarv.
void printAsciiArt() {
    std::cout << std::setw(3) << "  _________                   " << std::endl;
    std::cout << std::setw(3) << " /   _____/____ __________  __" << std::endl;
    std::cout << std::setw(3) << " \_____  \\__  \\_  __ \  \/ /" << std::endl;
    std::cout << std::setw(3) << " / \ / __ \|  | \ / \ / " << std::endl;
    std::cout << std::setw(3) << "/_______  (____  /__|    \_/  " << std::endl;
    std::cout << std::setw(3) << "        \/     \/             " << std::endl;
}

 void consoleDim( CONSOLE_SCREEN_BUFFER_INFO csbi) {
    //consoleLength = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
     consoleLength = csbi.dwSize.X;
     consoleWidth = csbi.dwSize.Y;
    //consoleWidth = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}


 //function to relocate the pointer to the starting of the terminal.
 void RelocateCursor(HANDLE hConsole, int X, int Y) {
     COORD newPosition = { static_cast<SHORT>(X), static_cast<SHORT>(Y) };
     SetConsoleCursorPosition(hConsole, newPosition);
 }

// Function to print boundaries and the name "Surv" at top.

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

//function to clear the screen.

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

// Function to handle Ctrl+C event
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



//function to move mouse at a particular position. Quest - why not make a lambda function for that ? No we can't cause we have to listen for 
//different mouse keys as well !!!.
void moveMouse(HANDLE hConsole, CONSOLE_SCREEN_BUFFER_INFO csbi) {
    while (true) {
        int X = csbi.dwCursorPosition.X;
        int Y = csbi.dwCursorPosition.Y;
        // Check the state of the arrow keys
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            RelocateCursor(hConsole, X-1, Y);
        }

        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            RelocateCursor(hConsole, X+1, Y);
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            RelocateCursor(hConsole, X, Y-1);
        }

        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            RelocateCursor(hConsole, X, Y+1);
        }
        
        // Optional: Add a small delay to reduce CPU usage
        Sleep(100);
    }
}

int main(int argc, char* argv[]) {

    // Disable cursor and scrolling
    std::cout << disableScrolling;

    //Checking if the arguments are defined are not.
    if (argc < 2) {
        printAsciiArt();
        std::cerr << "Usage: " << argv[0] << " <file_path> -<optional_Arguments>" << std::endl;
        getchar();
        //ClearScreen();
        exit(0);
    }

    // Event to continously monitor for ctrl+c event.

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);


    //Getting the information of cosole :

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi; // Stores information regarding console window, like size, width, height etc.
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    consoleDim(csbi);

    //Function or threading for continously listening for mouse movements and do what's required.

    std::thread MouseListen(moveMouse, hConsole, csbi);

    // Calling printBoundaries function to print the boundaries accross the terminal.

    printBoundaries(hConsole, csbi);

    //Relocating the cursor to the starting position of the window.
    RelocateCursor(hConsole, 2, 2);


    std::wstring filePath;


    
    while (!exitRequested) {
        // Sleep to avoid busy-waiting and reduce CPU usage
        Sleep(100);
    };
    

    MouseListen.join();

    // Re-enable cursor and scrolling
    std::cout << restoreScrolling<< showCursor;
}

