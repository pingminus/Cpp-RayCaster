#include <iostream>
#include <Windows.h>
#include <cmath>  
#include <chrono>

using namespace std;

// Screen dimensions
int nScreenWidth = 120;   // Console screen width
int nScreenHeight = 40;   // Console screen height

// Player position and angle
float PlayerX = 3.0f;     // Player's X position
float PlayerY = 8.0f;     // Player's Y position
float PlayerAngle = 0.0f; // Player's angle in radians

// Map dimensions
int nMapHeight = 16;  // Height of the map
int nMapWidth = 16;   // Width of the map

// Raycasting settings
float Depth = 16.0f;    // Maximum distance for raycasting
float FOV = 3.14159f / 4.0f;  // Field of view (in radians)

// Main function
int main() {
    // Allocate space for the screen buffer
    wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);

    DWORD dwBytesWritten = 0;
    COORD bufferSize = { (SHORT)nScreenWidth, (SHORT)nScreenHeight };
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Set console window size
    SMALL_RECT windowSize = { 0, 0, (SHORT)(nScreenWidth), (SHORT)(nScreenHeight) };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    // Reapply the buffer size in case the window size changes
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Define a 2D matrix for the map layout
    char mapMatrix[16][16] = {
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
        {'#', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '#', '#', '#', '#', '#'},
        {'#', '.', '#', '.', '.', '.', '.', '.', '#', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '.', '.', '.', '.', '.', '#', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '#', '#', '.', '.', '.', '#', '#', '#', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '#', '#', '#', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
    };

    // Set up timing for frame rate control
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Main game loop for raycasting
    while (true) {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float ElapsedTime = elapsedTime.count();

        // Player movement controls (W, A, S, D)
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            PlayerAngle -= (1.0f) * ElapsedTime; // Turn left
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
            PlayerAngle += (1.0f) * ElapsedTime; // Turn right
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            PlayerX += cosf(PlayerAngle) * 5.0f * ElapsedTime; // Move forward
            PlayerY += sinf(PlayerAngle) * 5.0f * ElapsedTime; // Move forward
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            PlayerX -= cosf(PlayerAngle) * 5.0f * ElapsedTime; // Move backward
            PlayerY -= sinf(PlayerAngle) * 5.0f * ElapsedTime; // Move backward
        }

        // Raycasting for each column of the screen
        for (int x = 0; x < nScreenWidth; x++) {
            // Calculate the angle for this column's ray
            float RayAngle = (PlayerAngle - FOV / 2.0f) + ((float)x / (float)nScreenWidth) * FOV;
            float DistanceToWall = 0.0f;
            bool HitWall = false;

            // Direction of the ray
            float RayX = cosf(RayAngle);
            float RayY = sinf(RayAngle);

            // Cast the ray and check for walls
            while (!HitWall && DistanceToWall < Depth) {
                DistanceToWall += 0.1f;  // Increment the distance of the ray
                int nTestX = (int)(PlayerX + RayX * DistanceToWall); // Calculate test X position
                int nTestY = (int)(PlayerY + RayY * DistanceToWall); // Calculate test Y position

                // Check if the ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    HitWall = true;  // Ray hit the border of the map
                    DistanceToWall = Depth; // Set max distance
                }
                else {
                    // Check if the ray hits a wall ('#' in the map)
                    if (mapMatrix[nTestY][nTestX] == '#') {
                        HitWall = true;
                    }
                }
            }

            // Calculate ceiling and floor positions for wall height
            int Ceiling = (int)(nScreenHeight / 2.0 - nScreenHeight / DistanceToWall);
            int Floor = nScreenHeight - Ceiling;

            // Shade selection based on distance to wall
            short Shade = ' ';
            if (DistanceToWall <= Depth / 4.0f)             Shade = 0x2588; // Full block
            else if (DistanceToWall < Depth / 3.0f)         Shade = 0x2593; // Three-quarters block
            else if (DistanceToWall < Depth / 2.0f)         Shade = 0x2592; // Half block
            else if (DistanceToWall < Depth)                Shade = 0x2591; // Quarter block
            else                                            Shade = ' ';    // Empty space

            // Render the column from ceiling to floor
            for (int y = 0; y < nScreenHeight; y++) {
                if (y < Ceiling) {
                    screen[y * nScreenWidth + x] = ' ';  // Above the ceiling, empty space
                }
                else if (y >= Ceiling && y <= Floor) {
                    screen[y * nScreenWidth + x] = Shade; // Wall between ceiling and floor
                }
                else {
                    // Render floor with a gradient effect
                    float b = 1.0f - ((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f);
                    screen[y * nScreenWidth + x] = (b < 0.25) ? '#' : (b < 0.5) ? 'X' : (b < 0.75) ? '=' : '.';
                }
            }
        }

        // Draw the map (for reference)
        for (int nx = 0; nx < nMapWidth; nx++) {
            for (int ny = 0; ny < nMapHeight; ny++) {
                screen[(ny + 1) * nScreenWidth + nx] = mapMatrix[ny][nx];
            }
        }

        // Draw the player at the current position
        screen[((int)PlayerY + 1) * nScreenWidth + (int)PlayerX] = 'P';

        // End of frame
        screen[nScreenHeight * nScreenWidth - 1] = '\0';  // Null-terminate the string
        WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0, 0 }, &dwBytesWritten);
    }

    return 0;
}

