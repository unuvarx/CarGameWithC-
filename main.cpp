

// GITHUB: 
// https://github.com/unuvarx/CarGameWithC-

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <iostream>
#include <queue>
#include <ctime>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <fstream>
#include <cstdlib> // Include for rand() and srand()
#include <ctime> // Include for time()
#include <chrono>
#include <thread>



#define wHeight 40 // height of the road
#define wWidth 100 // width of the road
#define lineX 45 // x coordinate of the middle line
#define lineLEN 10 // distance of the middle line from the beginning and the end
#define EXITY 35 // coordinate showing the end of the road
#define leftKeyArrow 260 // ASCII code of the left arrow key
#define RightKeyArrow 261 // ASCII code of the right arrow key
#define leftKeyA 97 // ASCII code of A
#define RightKeyD 100 // ASCII code of D
#define ESC 27 // ASCII code of the ESC key
#define ENTER 10 // ASCII code of the ENTER key
#define KEYPUP 259 // ASCII code of the up arrow key
#define KEYDOWN 258 // ASCII code of the down arrow key
#define KEYERROR -1 // ASCII code returned if an incorrect key is pressed
#define SAVEKEY 115 // ASCII code of S
#define levelBound 300 // To increase level after 300 points
#define MAXSLEVEL 5 // maximum level
#define ISPEED 500000 // initial value for game moveSpeed
#define DRATESPEED 100000 // to decrease moveSpeed after each new level
#define MINX 5 // minimum x coordinate value when creating cars
#define MINY 10 // the maximum y coordinate value when creating the cars, then we multiply it by -1 and take its inverse
#define MINH 5 // minimum height when creating cars
#define MINW 5 // minimum width when creating cars
#define SPEEDOFCAR 3 // speed of the car driven by the player
#define YOFCAR 34 // y coordinate of the car used by the player
#define XOFCAR 45 // x coordinate of the car used by the player
#define IDSTART 10 // initial value for cars ID
#define IDMAX 20 // maximum value for cars ID
#define COLOROFCAR 3 // color value of the car used by the player
#define POINTX 91 // x coordinate where the point is written
#define POINTY 42 // y coordinate where the point is written
#define MENUX 10 // x coordinate for the starting row of the menus
#define MENUY 5 // y coordinate for the starting row of the menus
#define MENUDIF 2 // difference between menu rows
#define MENUDIFX 20 // difference between menu columns
#define MENSLEEPRATE 200000 // sleep time for menu input
#define GAMESLEEPRATE 250000 // sleep time for player arrow keys
#define EnQueueSleep 1 // EnQueue sleep time
#define DeQueueSleepMin 2 // DeQueue minimum sleep time
#define numOfcolors 4 // maximum color value that can be selected for cars
#define maxCarNumber 5 // maximum number of cars in the queue
#define numOfChars 3 // maximum number of patterns that can be selected for cars
#define settingMenuItem 2 // number of options in the setting menu
#define mainMenuItem 6 // number of options in the main menu

using namespace std;

typedef struct Car {
    int ID;
    int x;
    int y;
    int height;
    int width;
    int speed;
    int clr;
    bool isExist;
    char chr;
} Car;

typedef struct Game {
    int leftKey;
    int rightKey;
    queue<Car> cars;
    bool IsGameRunning;
    bool IsSaveCliked;
    int counter;
    pthread_mutex_t mutexFile;
    Car current;
    int level;
    int moveSpeed;
    int points;
} Game;

Game playingGame; // Global variable used for new game
const char *gameTxt = "game.txt";
const char *CarsTxt = "cars.txt";
const char *pointsTxt = "points.txt";

// Array with options for the Setting menu
const char *settingMenu[50] = {"Play with < and > arrow keys", "Play with A and D keys"};
// Array with options for the Main menu
const char *mainMenu[50] = {"New Game", "Load the last game", "Instructions", "Settings", "Points", "Exit"};

void drawCar(Car c, int type, int direction); // prints or remove the given car on the screen
void printWindow(); // Draws the road on the screen
void *newGame(void *); // manages new game
void initGame(); // Assigns initial values to all control parameters for the new game
void initWindow(); // Creates a new window and sets I/O settings
void displayMenu(const char *menu[], int menuSize, int highlight); // Displays the menu
void Menu(); // Main menu
void settingsMenu(); // Settings menu
void instructions(); // Instructions menu
void points(); // Points menu
void *loadGame(void *); // Load game menu
void setTerminalSize(int width, int height); // Sets the terminal size
void updatePoints(int points);
bool checkCollision(Car playerCar, Car otherCar);
void updatePointsFile(int points);
void saveGame(Game savedGame);  // Saves the info in the global variable neccessary to load the last name
void lastGame(Game savedGame); // Assigns the saved values to all control parameters for the new game
int main() {
    // Set terminal size to 100x40
    setTerminalSize(wWidth, wHeight);

    playingGame.leftKey = leftKeyArrow;
    playingGame.rightKey = RightKeyArrow;
    // initGame(); // Bu çağrıyı kaldırıyoruz
    initWindow();

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Menü öğeleri için yeşil
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Seçili öğe için sarı arka plan
    init_pair(3, COLOR_RED, COLOR_BLACK); // Seçili öğe için kırmızı metin

    Menu();

    return 0;
}


void setTerminalSize(int width, int height) {
    struct winsize ws;
    ws.ws_col = width;
    ws.ws_row = height;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws); // Set the terminal size
}



// SENA - SEFA
Car createRandomCar(int id) {
    Car newCar;
    newCar.ID = id;
    
    // Arabanın genişliği ve en sağ/en sol sınırları hesaplanıyor
    int carWidth = MINW + rand() % (7- MINW+1 );
    int leftBound = MINX + carWidth;
    int rightBound = wWidth - MINX - carWidth;
    
    // Ortadaki şeridin x koordinatı
    int middleLineX = lineX;
    
    // Arabanın x koordinatını belirlerken şerit çizgilerine ve orta şeride denk gelmemesi için sınırları kullan
    while (true) {
        newCar.x = leftBound + rand() % (rightBound - leftBound);
        // Arabanın x koordinatı orta şeridin x koordinatından belirli bir uzaklıkta olmalı
        if (abs(newCar.x - middleLineX) > carWidth + 1) { // +1 farklılık sağlamak için
            break;
        }
    }
    
    newCar.y = -MINY; // Start above the screen
    newCar.height = MINH + rand() % (7-MINH+1); // Random height between 5-7
    newCar.width = carWidth; // Belirlenen genişlik
    newCar.speed = newCar.height/2; // Arabanın hızı, uzunluğunun yarısı
    newCar.clr = 1 + rand() % numOfcolors; // Random color
    newCar.isExist = true;
	
    int ch= rand()%(3-1+1)+1; // Random number between 1-3 so we can pick the char of the car
    switch(ch){
	case 1:
            newCar.chr = '*';
            break; // Car character
        case 2:
            newCar.chr = '+';
            break; // Car character
        case 3:
            newCar.chr = '#';
            break; // Car character
    }
    return newCar;
}

void initGame() {
    srand(time(0)); // Seed random number generator
    playingGame.cars = queue<Car>();
    playingGame.counter = IDSTART;
    playingGame.mutexFile = PTHREAD_MUTEX_INITIALIZER; // Assign the initial value for the mutex
    playingGame.level = 1;
    playingGame.moveSpeed = ISPEED;
    playingGame.points = 0;
    playingGame.IsSaveCliked = false;
    playingGame.IsGameRunning = true;
    playingGame.current.ID = IDSTART - 1;
    playingGame.current.height = MINH;
    playingGame.current.width = MINW;
    playingGame.current.speed = SPEEDOFCAR;
    playingGame.current.x = XOFCAR;
    playingGame.current.y = YOFCAR;
    playingGame.current.clr = COLOROFCAR;
    playingGame.current.chr = '*';
}
// SEFA
void moveAndDrawCars() {
    std::chrono::steady_clock::time_point lastEnqueueTime = std::chrono::steady_clock::now(); // Araç eklenme zamanını takip edin
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now(); // Oyun başlangıç zamanı
    int initialCarsDisplayed = 0; // Ekrana çıkartılan ilk araçların sayısı
    queue<Car> ontheRoad;
    while (playingGame.IsGameRunning && playingGame.IsSaveCliked != true) {
        pthread_mutex_lock(&playingGame.mutexFile); // Mutex'i kilitler
        queue<Car> temp;
        while (!playingGame.cars.empty()) {
            Car c = playingGame.cars.front();
            playingGame.cars.pop();
            drawCar(c, 1, 0); // Aracı ekrandan kaldır
            c.y += c.speed; // Y pozisyonunu güncelle
            if (c.y < EXITY) {
		ontheRoad.push(c);
                temp.push(c);
                drawCar(c, 2, 0); // Yeni pozisyonla aracı çiz
            } else {
		ontheRoad.pop();
                playingGame.points += (c.height * c.width); // Araba ekrandan çıktığında puanı topla
		    
		if (playingGame.level < MAXSLEVEL){ // If the car hasn't reached max level {
			if (playingGame.points > (levelBound)*playingGame.level) { // After every 300 points earned
				playingGame.level += 1; // level is increased by 1 
				playingGame.moveSpeed -= DRATESPEED;  // and moveSpeed of the game is reduced by 100000
			}
		}
		    
                updatePoints(playingGame.points); // Puanları güncelle ve ekrana yaz
            }
        }

        // Şu anki zaman ve son araç eklenme zamanını kontrol edin
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastEnqueueTime).count();
        auto initialDuration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        // İlk 5 aracı 4 saniye arayla ekrana çıkar
        if (initialCarsDisplayed < maxCarNumber) {
            if (initialDuration >= 4 * initialCarsDisplayed && duration >= 4) {
                temp.push(createRandomCar(playingGame.counter++));
                lastEnqueueTime = now; // Son eklenme zamanını güncelle
                initialCarsDisplayed++;
            }
        } else {
            // 4 saniye geçtiyse yeni bir araç oluşturun
            if (duration >= 4 && temp.size() < maxCarNumber) {
                temp.push(createRandomCar(playingGame.counter++));
                lastEnqueueTime = now; // Son eklenme zamanını güncelle
            }
        }
	if (playingGame.counter > IDMAX) {   // If the cars ID exceeds 20, it's set back to 10
                playingGame.counter = IDSTART;
	}
        
	playingGame.cars = temp;
        
	if (playingGame.IsSaveCliked == true) {
                playingGame.cars = ontheRoad;  // Takes the list of cars on the traffic when the game was saved
        }

        pthread_mutex_unlock(&playingGame.mutexFile); // Mutex'i açar
        usleep(playingGame.moveSpeed); // Bir süre uyur
    }
}


// SEFA - SENA
void* newGame(void *) {
    initGame(); // Oyun başladığında initGame fonksiyonu çağrılacak

    pthread_t carThread;
    pthread_create(&carThread, NULL, reinterpret_cast<void* (*)(void*)>(moveAndDrawCars), NULL);

    printWindow();
    drawCar(playingGame.current, 2, 1); // Draw the car the player is driving on the screen
    int key;
    while (playingGame.IsGameRunning) { // Continue until the game is over
        key = getch(); // Get input for the player to press the arrow keys
		queue<Car> temp = playingGame.cars;
        if (key != KEYERROR) {
            if (key == playingGame.leftKey) { // If the left key is pressed
                drawCar(playingGame.current, 1, 1); // Removes player's car from screen
                playingGame.current.x -= playingGame.current.speed; // Update position
                drawCar(playingGame.current, 2, 1); // Draw player's car with new position
            } else if (key == playingGame.rightKey) { // If the right key is pressed
                drawCar(playingGame.current, 1, 1); // Removes player's car from screen
                playingGame.current.x += playingGame.current.speed; // Update position
                drawCar(playingGame.current, 2, 1); // Draw player's car with new position
            } else if (key == ESC) {
                playingGame.IsGameRunning = false; // Exit the game if ESC is pressed
                updatePointsFile(playingGame.points); // Puanı dosyaya yaz
                Menu(); // Return to the main menu
            }  else if (key == SAVEKEY){  // If the S key is pressed
                playingGame.IsSaveCliked = true;  
                playingGame.IsGameRunning = false; 
                updatePointsFile(playingGame.points);
		
				pthread_mutex_lock(&playingGame.mutexFile);
                saveGame(playingGame);   // Saves the info neccessary to load the game back
                pthread_mutex_unlock(&playingGame.mutexFile);
                Menu(); // Return to the main menu
            }

            // Check collision with other cars
            pthread_mutex_lock(&playingGame.mutexFile);
            
            pthread_mutex_unlock(&playingGame.mutexFile);
            
        }
		while (!temp.empty()) {
                if (checkCollision(playingGame.current, temp.front())) {
                    playingGame.IsGameRunning = false; // Oyunu sonlandır
                    updatePointsFile(playingGame.points); // Puanı dosyaya yaz
                    Menu(); // Return to the main menu
                    break; // Döngüden çık
                }
                temp.pop(); // Kuyruktaki bir sonraki arabaya geç
        }
        usleep(GAMESLEEPRATE); // Sleep for a short period
    }
    return NULL;
}


void initWindow() {
    initscr();            // initialize the ncurses window
    start_color();        // enable color manipulation
    keypad(stdscr, true); // enable the keypad for the screen
    nodelay(stdscr, false);// set the getch() function to blocking mode
    curs_set(0);          // hide the cursor
    cbreak();             // disable line buffering
    noecho();             // don't echo characters entered by the user
    clear();              // clear the screen
    sleep(1);
	 timeout(0); // Non-blocking getch()
}
// SENA
void printWindow() {
    for (int i = 1; i < wHeight - 1; ++i) {
        mvprintw(i, 2, "*"); // left side of the road
        mvprintw(i, 0, "*");
        mvprintw(i, wWidth - 1, "*"); // right side of the road
        mvprintw(i, wWidth - 3, "*");
    }
    for (int i = lineLEN; i < wHeight - lineLEN; ++i) { // line in the middle of the road
        mvprintw(i, lineX, "#");
    }
    int j = 5;
    for (int i = 1; i < 4; i++) {
	attron(COLOR_PAIR(1));    // Color pairing for green
        mvprintw(j, wWidth + 7, "*");
        mvprintw(j+1, wWidth + 6, "* *");      // The leaves of the trees
	mvprintw(j+2, wWidth + 5, "* * *");
        attroff(COLOR_PAIR(1));    // Color green turned off
	
	attron(COLOR_PAIR(3));     // Color pairing for red
        mvprintw(j+3, wWidth + 7, "#");    // The tree trunk
	mvprintw(j+4, wWidth + 7, "#");
        attroff(COLOR_PAIR(3));    // Color red turned off
        j +=10;
    }
}

// SEFA
void drawCar(Car c, int type, int direction) {
	
    if (playingGame.IsSaveCliked != true && playingGame.IsGameRunning == true) {
        char drawnChar;
        if (type == 1)
            drawnChar = ' ';
        else
            drawnChar = c.chr;

        // Eski konumu temizle
        mvhline(c.y, c.x, ' ', c.width);
        mvhline(c.y + c.height - 1, c.x, ' ', c.width);
        mvvline(c.y, c.x, ' ', c.height);
        mvvline(c.y, c.x + c.width - 1, ' ', c.height);
       int textLength = 2; // Varsayılan metin uzunluğu
		if ((c.height * c.width) >= numOfChars) {
			textLength = 3; // Metin uzunluğunu 3 olarak ayarla
		}
		for (int i = 0; i < textLength; ++i) {
			mvprintw(c.y + 1, c.x + 1 + i, " "); // Metni temizle
		} // Metni temizle

        // Yeni konumu çiz
        init_pair(c.ID, c.clr, 0); // Renk çiftini burada çağırıyoruz
        attron(COLOR_PAIR(c.ID)); // Renk çiftini aktifleştiriyoruz
        mvhline(c.y, c.x, drawnChar, c.width);
        mvhline(c.y + c.height - 1, c.x, drawnChar, c.width);
        mvvline(c.y, c.x, drawnChar, c.height);
        mvvline(c.y, c.x + c.width - 1, drawnChar, c.height);

        // Metni yaz, ancak arabaların boyutlarına göre kontrol et
        if (type != 1 && (c.height * c.width) >= numOfChars) {
            mvprintw(c.y + 1, c.x + 1, "%d", c.height * c.width);
        }

        attroff(COLOR_PAIR(c.ID)); // Renk çiftini kapatıyoruz
    }
}





// SEFA
void displayMenu(const char *menu[], int menuSize, int highlight) {
    clear();
    for (int i = 0; i < menuSize; ++i) {
        if (i == highlight) {
            attron(COLOR_PAIR(3)); // Kırmızı renk için renk çifti 3
            attron(A_BOLD); // Metni kalın yap
            mvprintw(MENUY + i * MENUDIF, MENUX - 2, "->"); // Ok işareti ekle
        } else {
            attron(COLOR_PAIR(1)); // Seçilmeyen öğeler için yeşil metin
            mvprintw(MENUY + i * MENUDIF, MENUX - 2, "  "); // Ok işaretini temizle
        }
        mvprintw(MENUY + i * MENUDIF, MENUX, menu[i]);
        if (i == highlight) {
            attroff(A_BOLD); // Kalın metni kapat
            attroff(COLOR_PAIR(3)); // Kırmızı rengi kapat
        } else {
            attroff(COLOR_PAIR(1)); // Yeşil metni kapat
        }
    }
    refresh();
}

// SEFA
void Menu() {
    int highlight = 0;
    int choice;
    bool inMenu = true;

    while (inMenu) {
        displayMenu(mainMenu, mainMenuItem, highlight);
        choice = getch();
        switch (choice) {
            case KEYDOWN:
                highlight = (highlight + 1) % mainMenuItem;
                break;
            case KEYPUP:
                highlight = (highlight - 1 + mainMenuItem) % mainMenuItem;
                break;
            case ENTER:
                switch (highlight) {
                    case 0: // New Game
                        inMenu = false; // Menü döngüsünden çık
                        clear(); // Ekranı temizle
                        refresh();
                        pthread_t th1;
                        pthread_create(&th1, NULL, newGame, NULL);
                        pthread_join(th1, NULL);
                        break;
                    case 1: // Load Game
                        inMenu = false;
                        clear();
                        refresh();
                        pthread_t th2;
                        pthread_create(&th2, NULL, loadGame, NULL);
                        pthread_join(th2, NULL);
                        break;
                    case 2: // Instructions
                        instructions();
                        break;
                    case 3: // Settings
                        settingsMenu();
                        break;
                    case 4: // Points
                        points();
                        break;
                    case 5: // Exit
			 playingGame.IsGameRunning = false;
			 endwin();
			 clear(); // Ekranı temizle
			 return;
                }
                break;
            case ESC:
                playingGame.IsGameRunning = false;
                endwin();
                return;
            default:
                break;
        }
    }
}





// SENA
void saveGame(Game savedGame) {
    fstream carsFile(CarsTxt, ios::out | ios::binary); // Open file for writing in binary form
    if (carsFile.is_open()) {  // Check for opening errors
	    while (!savedGame.cars.empty()) {
		    Car c = savedGame.cars.front(); // Takes the cars in trafic one by one
		    savedGame.cars.pop();
		    carsFile.write(reinterpret_cast<char*>(&c), sizeof(Car)); // And writes the info on Cars.txt file
	    }
	    carsFile.close();
    }
    else {
        cout << "Cars.txt file cannot be opened!!" << endl;  
    }
    
    fstream gameFile(gameTxt, ios::out | ios::binary);   // Open file for writing in binary form
    if (gameFile.is_open()) {
        gameFile.write(reinterpret_cast<char*>(&savedGame), sizeof(Game)); // Save the Game struct info on the file in binary
        gameFile.close();
    }
    else {
        cout << "Game.txt file cannot be opened!!" << endl;
    }
}




// SENA
void lastGame(Game savedGame) {
    srand(time(0)); // Seed random number generator
    playingGame.cars = queue<Car>();
    playingGame.counter = savedGame.counter;
    playingGame.mutexFile = savedGame.mutexFile;// Assign the initial value for the mutex
    playingGame.level = savedGame.level;
    playingGame.moveSpeed = savedGame.moveSpeed;
    playingGame.points = savedGame.points;
    playingGame.IsSaveCliked = false;
    playingGame.IsGameRunning = true;
    playingGame.current.ID = savedGame.current.ID;
    playingGame.current.height = savedGame.current.height;
    playingGame.current.width = savedGame.current.width;
    playingGame.current.speed = savedGame.current.speed;
    playingGame.current.x = savedGame.current.x;
    playingGame.current.y = savedGame.current.y;
    playingGame.current.clr = savedGame.current.clr;
    playingGame.current.chr = savedGame.current.chr;


}

// SENA
void* loadGame(void*) {

    fstream gameFile(gameTxt, ios:: in | ios::binary);
    gameFile.seekg(0, ios::end);
    int size = gameFile.tellg();
    if (size != 0 ) {
        if (gameFile.is_open()) {
            gameFile.read(reinterpret_cast<char*>(&playingGame), sizeof(Game));
            gameFile.close();
        }
        lastGame(playingGame);

        fstream carsFile(CarsTxt, ios::in | ios::binary);
        if (carsFile.is_open()) {

            while (carsFile.peek() != EOF) {
                Car c;
                carsFile.read(reinterpret_cast<char*>(&c), sizeof(Car));
                playingGame.cars.push(c);
            }
            carsFile.close();
        }
    }
    else {
        initGame();
    }

    pthread_t newGameThread;
    pthread_create(&newGameThread, NULL, reinterpret_cast<void* (*)(void*)>(moveAndDrawCars), NULL);
    
    pthread_mutex_lock(&playingGame.mutexFile);
    printWindow();
    drawCar(playingGame.current, 2, 1); // Draw the car the player is driving on the screen
    pthread_mutex_unlock(&playingGame.mutexFile);
	
    int key;
	
    while (playingGame.IsGameRunning) {// Continue until the game is over
        key = getch(); // Get input for the player to press the arrow keys
		queue<Car> temp = playingGame.cars;
        if (key != KEYERROR) {
            if (key == playingGame.leftKey) { // If the left key is pressed
                drawCar(playingGame.current, 1, 1); // Removes player's car from screen
                playingGame.current.x -= playingGame.current.speed; // Update position
                drawCar(playingGame.current, 2, 1); // Draw player's car with new position
            }
            else if (key == playingGame.rightKey) { // If the right key is pressed
                drawCar(playingGame.current, 1, 1); // Removes player's car from screen
                playingGame.current.x += playingGame.current.speed; // Update position
                drawCar(playingGame.current, 2, 1); // Draw player's car with new position
            }
            else if (key == ESC) {
                playingGame.IsGameRunning = false; // Exit the game if ESC is pressed
                updatePointsFile(playingGame.points); // Puaný dosyaya yaz
                Menu(); // Return to the main menu
            }
            else if (key == SAVEKEY) {  //Saves the game information if the S key is pressed
                playingGame.IsSaveCliked = true;
                playingGame.IsGameRunning = false;
                updatePointsFile(playingGame.points);
                pthread_mutex_lock(&playingGame.mutexFile);
                saveGame(playingGame);
                pthread_mutex_unlock(&playingGame.mutexFile);
                Menu();
            }

            // Check collision with other cars
            pthread_mutex_lock(&playingGame.mutexFile);
            
            pthread_mutex_unlock(&playingGame.mutexFile);
            
        }
		while (!temp.empty()) {
                if (checkCollision(playingGame.current, temp.front())) {
                    playingGame.IsGameRunning = false; // Oyunu sonlandýr
                    updatePointsFile(playingGame.points); // Puaný dosyaya yaz
                    Menu(); // Return to the main menu
                    break; // Döngüden çýk
                }
                temp.pop(); // Kuyruktaki bir sonraki arabaya geç
        }
        usleep(GAMESLEEPRATE); // Sleep for a short period
    }
    return NULL;
    // Load game logic here
}



// SEFA
void displaySettingsMenu(int highlight) {
    clear();
    for (int i = 0; i < settingMenuItem; ++i) {
        if (i == highlight) {
            attron(COLOR_PAIR(3)); // Kırmızı renk için renk çifti 3
            attron(A_BOLD); // Metni kalın yap
            mvprintw(MENUY + i * MENUDIF, MENUX - 2, "->"); // Ok işareti ekle
        } else {
            attron(COLOR_PAIR(1)); // Seçilmeyen öğeler için yeşil metin
            mvprintw(MENUY + i * MENUDIF, MENUX - 2, "  "); // Ok işaretini temizle
        }
        mvprintw(MENUY + i * MENUDIF, MENUX, settingMenu[i]);
        if (i == highlight) {
            attroff(A_BOLD); // Kalın metni kapat
            attroff(COLOR_PAIR(3)); // Kırmızı rengi kapat
        } else {
            attroff(COLOR_PAIR(1)); // Yeşil metni kapat
        }
    }
    refresh();
}

// SEFA
void settingsMenu() {
    int highlight = 0;
    int choice;
    bool inSettingsMenu = true;

    while (inSettingsMenu) {
        displaySettingsMenu(highlight);
        choice = getch();
        switch (choice) {
            case KEYDOWN:
                highlight = (highlight + 1) % settingMenuItem;
                break;
            case KEYPUP:
                highlight = (highlight - 1 + settingMenuItem) % settingMenuItem;
                break;
            case ENTER:
                switch (highlight) {
                    case 0: // Play with < and > arrow keys
                        playingGame.leftKey = leftKeyArrow;
                        playingGame.rightKey = RightKeyArrow;
                        break;
                    case 1: // Play with A and D keys
                        playingGame.leftKey = leftKeyA;
                        playingGame.rightKey = RightKeyD;
                        break;
                }
                inSettingsMenu = false;
                break;
            case ESC:
                inSettingsMenu = false;
                break;
            default:
                break;
        }
    }
}

  
// SEFA
void displayInstructions() {
    clear();
    attron(COLOR_PAIR(1)); // Yeşil renk için renk çifti 1
    mvprintw(10, 10, "< or A: moves the car to the left");
    mvprintw(12, 10, "> or D: moves the car to the right");
    mvprintw(14, 10, "ESC: exits the game without saving");
    mvprintw(16, 10, "S: saves and exits the game");
    attroff(COLOR_PAIR(1)); // Yeşil metni kapat
    refresh();
}



  
// SEFA
void instructions() {
    displayInstructions();

    int key;
    while (1) {
        key = getch();
        if (key == ESC) {
            return; // ESC tuşuna basarak Instructions menüsünden çık
        }
    }
}
// SEFA
void points() {
    clear(); // Ekranı temizle
    refresh(); // Ekranı güncelle

    ifstream file(pointsTxt); // Dosyayı aç
    if (file.is_open()) {
        string line;
        int gameNumber = 1;
        int xCoordinate = 10; // Başlangıç x koordinatı
        int yCoordinate = 10; // Başlangıç y koordinatı
        // Dosyadan her satırı oku ve ekrana yaz
        while (getline(file, line)) {
            attron(COLOR_PAIR(1)); // Yeşil renk için renk çifti 1
            mvprintw(yCoordinate, xCoordinate, "Game %d: %s", gameNumber, line.c_str());
            attroff(COLOR_PAIR(1)); // Yeşil metni kapat
            gameNumber++;
            yCoordinate++; // Sonraki satıra geç
            if (yCoordinate >= wHeight - 1) { // Eğer ekranın altına ulaşıldıysa
                yCoordinate = 10; // Y koordinatını başa al
                xCoordinate += 20; // X koordinatını sağa kaydır
                if (xCoordinate >= wWidth - 20) { // Eğer ekranın sağında yer kalmadıysa
                    break; // Döngüden çık
                }
            }
        }
        file.close(); // Dosyayı kapat
    } else {
        // Dosya yoksa "No points..." yazdır
        attron(COLOR_PAIR(1)); // Yeşil renk için renk çifti 1
        mvprintw(10, 10, "No points...");
        attroff(COLOR_PAIR(1)); // Yeşil metni kapat
    }
    refresh();

    int key; // Tuş girişini tutmak için değişken
    while (1) { // Sonsuz döngü
        key = getch(); // Bir tuşa basılmasını bekler
        if (key == ESC || key == ENTER) { // Eğer ESC ya da ENTER tuşuna basılırsa
            Menu(); // Ana menüye geri dön
        }
    }
}





// SEFA
void updatePoints(int points) {
    // Temizle
    mvhline(POINTY, POINTX, ' ', 20);
    // Yaz
    mvprintw(POINTY, POINTX, "Point: %d", points);
    refresh();
}


// SEFA
bool checkCollision(Car playerCar, Car otherCar) {
    // Araba koordinatlarını kontrol ederek çarpışmayı tespit et
    if ((playerCar.y + playerCar.height > otherCar.y) && 
        (playerCar.y < otherCar.y + otherCar.height) &&
        (playerCar.x + playerCar.width > otherCar.x) && 
        (playerCar.x < otherCar.x + otherCar.width)) {
        return true; // Çarpışma var
    }
    return false; // Çarpışma yok
}


// SEFA
void updatePointsFile(int points) {
    ofstream file(pointsTxt, ios::app); // Dosyayı aç ve dosyanın sonuna ekle modunda aç
    if (file.is_open()) {
        file << points << endl; // Puanı dosyaya yaz
        file.close(); // Dosyayı kapat
    } else {
        // Dosya açılamazsa bir hata mesajı yazdırabiliriz
        cout << "Unable to open points.txt file for writing!" << endl;
    }
}