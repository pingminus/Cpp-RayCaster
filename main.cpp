#include <iostream>
#include <Windows.h>
#include <cmath>  
#include <chrono>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float PlayerX = 3.0f;
float PlayerY = 8.0f;
float PlayerAngle = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;
float Depth = 16.0f;
float FOV = 3.14159f / 4.0f;



int main() {
   

    // Windows Screen Buffer
    wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);

    DWORD dwBytesWritten = 0;
    COORD bufferSize = { (SHORT)nScreenWidth, (SHORT)nScreenHeight };
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Set console window size
    SMALL_RECT windowSize = { 0, 0, (SHORT)(nScreenWidth), (SHORT)(nScreenHeight) };
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    // Reapply the buffer size in case the window size change adjusted it
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
        {'#', '.', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '#', '#', '#', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '.', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '.', '#'},
        {'#', '.', '.', '#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
        {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
    };

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Main game loop for ray casting
    while (true) {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float ElapsedTime = elapsedTime.count();

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
            PlayerAngle -= (1.0f) * ElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
            PlayerAngle += (1.0f) * ElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            PlayerX += cosf(PlayerAngle) * 5.0f * ElapsedTime;
            PlayerY += sinf(PlayerAngle) * 5.0f * ElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
            PlayerX -= cosf(PlayerAngle) * 5.0f * ElapsedTime;
            PlayerY -= sinf(PlayerAngle) * 5.0f * ElapsedTime;
        }

        for (int x = 0; x < nScreenWidth; x++) {
            float RayAngle = (PlayerAngle - FOV / 2.0f) + ((float)x / (float)nScreenWidth) * FOV;
            float DistanceToWall = 0.0f;
            bool HitWall = false;

            float RayX = cosf(RayAngle);
            float RayY = sinf(RayAngle);

            while (!HitWall && DistanceToWall < Depth) {
                DistanceToWall += 0.1f;
                int nTestX = (int)(PlayerX + RayX * DistanceToWall);
                int nTestY = (int)(PlayerY + RayY * DistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    HitWall = true;
                    DistanceToWall = Depth;
                }
                else {
                    if (mapMatrix[nTestY][nTestX] == '#') {
                        HitWall = true;
                    }
                }
            }

            int Ceiling = (int)(nScreenHeight / 2.0 - nScreenHeight / DistanceToWall);
            int Floor = nScreenHeight - Ceiling;
            short Shade = ' ';

            if (DistanceToWall <= Depth / 4.0f)             Shade = 0x2588;
            else if (DistanceToWall < Depth / 3.0f)         Shade = 0x2593;
            else if (DistanceToWall < Depth / 2.0f)         Shade = 0x2592;
            else if (DistanceToWall < Depth)                Shade = 0x2591;
            else                                            Shade = ' ';

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < Ceiling) {
                    screen[y * nScreenWidth + x] = ' ';
                }
                else if (y >= Ceiling && y <= Floor) {
                    screen[y * nScreenWidth + x] = Shade;
                }
                else {
                    float b = 1.0f - ((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f);
                    screen[y * nScreenWidth + x] = (b < 0.25) ? '#' : (b < 0.5) ? 'X' : (b < 0.75) ? '=' : '.';
                }
            }
        }

        for (int nx = 0; nx < nMapWidth; nx++) {
            for (int ny = 0; ny < nMapHeight; ny++) {
                screen[(ny + 1) * nScreenWidth + nx] = mapMatrix[ny][nx];
            }
        }

        screen[((int)PlayerY + 1) * nScreenWidth + (int)PlayerX] = 'P';
        screen[nScreenHeight * nScreenWidth - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0, 0 }, &dwBytesWritten);
    }

    return 0;
}
