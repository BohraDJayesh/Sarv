#include <iostream>
#include <Windows.h>
#include <cctype>
#include <thread>

#define WINDOWS_IGNORE_PACKING_MISMATCH
LONG consoleLength;
LONG consoleWidth;

 void consoleDim( CONSOLE_SCREEN_BUFFER_INFO csbi) {
    //consoleLength = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
     consoleLength = csbi.dwSize.X;
     consoleWidth = csbi.dwSize.Y;
    //consoleWidth = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
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
                std::cout << "-";
            else if (j == 0 || j == 2*centerX - 1)
                std::cout << "|";
            else
                std::cout << " ";
        }
        std::cout << std::endl;
    }



}

//function to clear the screen.
#include <iostream>
#include <Windows.h>

void ClearScreen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStdOut == INVALID_HANDLE_VALUE) {
        std::cerr << "Error getting console handle: " << GetLastError() << std::endl;
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hStdOut, &dwMode)) {
        std::cerr << "Error getting console mode: " << GetLastError() << std::endl;
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(hStdOut, dwMode)) {
        std::cerr << "Error setting console mode: " << GetLastError() << std::endl;
        return;
    }

    // ANSI escape sequence for clearing the screen
    const char* clearScreenSequence = "\x1b[2J";

    DWORD dwWritten = 0;
    if (!WriteConsoleA(hStdOut, clearScreenSequence, static_cast<DWORD>(strlen(clearScreenSequence)), &dwWritten, nullptr)) {
        std::cerr << "Error writing to console: " << GetLastError() << std::endl;
    }

    // Restore the original console mode
    SetConsoleMode(hStdOut, dwMode);
}



std::atomic<bool> exitRequested(false);

// Function to handle Ctrl+C event
BOOL CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        // Perform your task here
        ClearScreen();
        return TRUE; // Returning TRUE to prevent the default handler
    default:
        return FALSE;
    }
}



//function to relocate the pointer to the starting of the terminal.
void RelocateCursor(HANDLE hConsole, int X, int Y) {
    COORD newPosition = { static_cast<SHORT>(X), static_cast<SHORT>(Y) };
    SetConsoleCursorPosition(hConsole, newPosition);
}

void ctrlMonitor() {

}




int main(int argc, char* argv[]) {

    // Thread to continously monitor for ctrl+c event.

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
    std::thread monitorEvent(ctrlMonitor);

    //Getting the information of cosole :

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi; // Stores information regarding console window, like size, width, height etc.
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    consoleDim(csbi);


    // Calling printBoundaries function to print the boundaries accross the terminal.

    printBoundaries(hConsole, csbi);

    //Relocating the cursor to the starting position of the window.
    RelocateCursor(hConsole,2,2);

    //More Code goes here, make your changes here rest all should be remained unchanged.




    char temp = getchar();

    monitorEvent.join();
    ClearScreen();
    

}

