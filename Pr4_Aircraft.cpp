#include <graphics.h>
#include <iostream>
#include <time.h>
#include <list>
#include <windows.h>
#include <cmath> 
#include <chrono>
using namespace std;

// Linker directive to set the subsystem to Windows and specify the entry point
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma comment(lib,"winmm.lib")
#pragma warning(disable:4996)

// Size of the screen
#define WIN_WIDTH 700
#define WIN_HEIGHT 900
#define PI 3.14159265

// Struct to represent Enemy Bullets with owner ID and direction
struct EnemyBullet {
	int x;
	int y;
	int ownerId;
	int dx;
	int dy;

	EnemyBullet(int xx, int yy, int owner_, int deltaX = 0, int deltaY = 5)
		: x(xx), y(yy), ownerId(owner_), dx(deltaX), dy(deltaY) {}
};
// Struct to represent Enemy Planes with unique IDs and health
struct EnemyPlane {
	int x;
	int y;
	int id;
	int health;

	EnemyPlane(int xx, int yy, int id_) : x(xx), y(yy), id(id_), health(2) {}
};

// Define a Node class for player's bullets with speed
class Node {
public:
	Node(int xx, int yy, int spd = 15) { // Default speed is 15
		x = xx;
		y = yy;
		speed = spd;
	}
	int x;
	int y;
	int speed;
};

// Struct to represent Boss Plane
struct BossPlane {
	int x;
	int y;
	int health;

	BossPlane(int xx, int yy) : x(xx), y(yy), health(20) {}
};

// Create lists for enemy planes, player's bullets, and enemy bullets
Node myPlane(200, 680, 15); // Initialize with default speed
list<EnemyPlane> enemyPlaneList;
list<Node> bulletList;
list<EnemyBullet> enemyBulletList;

// Flags to indicate game state
bool gameOver = false;
bool gameWin = false;

// Variable to assign structure 
int nextEnemyId = 1;
const int DEFAULT_BULLET_SPEED = 20;
int enemiesDestroyed = 0; // Global counter
bool bossSpawned = false;
BossPlane* boss = nullptr; // Boss object 

// Function to draw the player's plane with additional details
void DrawMyPlane() {
	// Coordinates for the body of the plane (a triangle, upward-facing)
	POINT body[3];
	body[0].x = myPlane.x + 40; // Tip of the triangle (center top)
	body[0].y = myPlane.y;
	body[1].x = myPlane.x; // Left corner of the base
	body[1].y = myPlane.y + 60;
	body[2].x = myPlane.x + 80; // Right corner of the base
	body[2].y = myPlane.y + 60;

	// Coordinates for the wings (extending out from the sides)
	POINT wings[4];
	wings[0].x = myPlane.x - 20; // Left wing tip
	wings[0].y = myPlane.y + 40;
	wings[1].x = myPlane.x + 20; // Left wing base
	wings[1].y = myPlane.y + 40;
	wings[2].x = myPlane.x + 60; // Right wing base
	wings[2].y = myPlane.y + 40;
	wings[3].x = myPlane.x + 100; // Right wing tip
	wings[3].y = myPlane.y + 40;

	// Coordinates for the tail fin (a small triangle at the back)
	POINT tail[3];
	tail[0].x = myPlane.x + 40; // Tip of the tail fin (center back)
	tail[0].y = myPlane.y + 60;
	tail[1].x = myPlane.x + 30; // Left corner of the tail fin
	tail[1].y = myPlane.y + 80;
	tail[2].x = myPlane.x + 50; // Right corner of the tail fin
	tail[2].y = myPlane.y + 80;

	// Coordinates for the cockpit (small rectangle on top)
	int cockpit_x = myPlane.x + 30;
	int cockpit_y = myPlane.y + 20;
	int cockpit_width = 20;
	int cockpit_height = 10;

	// Set colors and draw the plane
	setfillcolor(WHITE); // Body color
	fillpolygon(body, 3);

	setfillcolor(LIGHTGRAY); // Wing color
	fillpolygon(wings, 4);

	setfillcolor(WHITE); // Tail color
	fillpolygon(tail, 3);

	setfillcolor(BLUE); // Cockpit color
	solidrectangle(cockpit_x, cockpit_y, cockpit_x + cockpit_width, cockpit_y + cockpit_height);
}

// Function to draw enemy helicopters with more realistic visuals and health bars
void DrawEnemyPlanes() {
	for (auto& p : enemyPlaneList) {
		// Change color based on health and add visual effects
		if (p.health == 2) {
			setfillcolor(LIGHTGRAY); // Healthy state
		}
		else if (p.health == 1) {
			setfillcolor(DARKGRAY); // Damaged state
			// Optional: add smoke or sparks here for more realism
		}

		// Draw the body of the helicopter (rounded rectangle for main body)
		fillroundrect(p.x, p.y, p.x + 50, p.y + 30, 10, 10);

		// Draw the main rotor (a long horizontal rectangle on top of the copter)
		setfillcolor(BLACK);
		fillrectangle(p.x - 15, p.y - 10, p.x + 65, p.y); // Rotor blades extending beyond the body

		// Draw the tail section of the helicopter (small rectangle extending from the back)
		setfillcolor(LIGHTGRAY);
		fillroundrect(p.x + 40, p.y + 10, p.x + 80, p.y + 20, 5, 5); // Tail section

		// Draw the tail rotor (small vertical rectangle at the end of the tail)
		setfillcolor(BLACK);
		fillrectangle(p.x + 80, p.y + 5, p.x + 85, p.y + 15); // Tail rotor

		// Draw the cockpit as a small oval on the front center of the helicopter
		setfillcolor(BLUE);
		solidellipse(p.x + 10, p.y + 5, p.x + 25, p.y + 15);

		// Draw the landing skids (thin lines below the body)
		setlinecolor(DARKGRAY);
		line(p.x + 5, p.y + 30, p.x + 15, p.y + 40); // Left skid
		line(p.x + 35, p.y + 30, p.x + 45, p.y + 40); // Right skid

		// Draw the health bar with smooth transitions
		int barWidth = 50;
		int barHeight = 5;
		int healthWidth = (p.health * barWidth) / 2; // Health scaling

		// Draw the background of the health bar (white)
		setfillcolor(WHITE);
		fillrectangle(p.x, p.y - 10, p.x + barWidth, p.y - 10 + barHeight);

		// Draw the current health (green)
		setfillcolor(GREEN);
		fillrectangle(p.x, p.y - 10, p.x + healthWidth, p.y - 10 + barHeight);

		// Optional: Draw some visual effect (like flickering) if the helicopter is damaged
		if (p.health == 1) {
			setfillcolor(RED); // Flashing effect to indicate damage
			fillroundrect(p.x, p.y, p.x + 50, p.y + 30, 10, 10); // Flash over the body
		}
	}
}


/// Function to draw the boss with enhanced visuals and a more dynamic health bar
void DrawBoss() {
	if (boss == nullptr) return;

	// Define custom colors
	COLORREF ORANGE = RGB(255, 165, 0);
	COLORREF DARKBLUE = RGB(0, 0, 139);

	// Base body (main shape)
	setfillcolor(BLUE); // Main color for the boss
	fillroundrect(boss->x, boss->y, boss->x + 100, boss->y + 60, 15, 15); // Larger boss shape

	// Wings (extending outward from the body)
	POINT wings[4];
	wings[0].x = boss->x - 20; wings[0].y = boss->y + 20; // Left wing tip
	wings[1].x = boss->x + 10; wings[1].y = boss->y + 20; // Left wing base
	wings[2].x = boss->x + 90; wings[2].y = boss->y + 20; // Right wing base
	wings[3].x = boss->x + 120; wings[3].y = boss->y + 20; // Right wing tip

	setfillcolor(DARKGRAY); // Wing color
	fillpolygon(wings, 4);

	// Cockpit (oval on top)
	setfillcolor(DARKBLUE);
	solidellipse(boss->x + 40, boss->y + 10, boss->x + 60, boss->y + 30);

	// Engines (small rectangles at the bottom)
	setfillcolor(DARKGRAY);
	solidrectangle(boss->x + 20, boss->y + 60, boss->x + 35, boss->y + 75); // Left engine
	solidrectangle(boss->x + 65, boss->y + 60, boss->x + 80, boss->y + 75); // Right engine

	// Optional: Draw exhaust flames for the engines when the boss is damaged
	if (boss->health <= 10) {
		setfillcolor(ORANGE); // Exhaust color
		solidrectangle(boss->x + 20, boss->y + 75, boss->x + 35, boss->y + 85); // Left flame
		solidrectangle(boss->x + 65, boss->y + 75, boss->x + 80, boss->y + 85); // Right flame
	}

	// Boss health bar with smoother transitions and color changes based on health
	int barWidth = 100;
	int barHeight = 10;
	int healthWidth = (boss->health * barWidth) / 20; // Boss max health = 20

	// Health bar background (gray)
	setfillcolor(LIGHTGRAY);
	fillrectangle(boss->x, boss->y - 20, boss->x + barWidth, boss->y - 20 + barHeight);

	// Health bar color transitions (red to yellow to green)
	if (boss->health > 15) {
		setfillcolor(GREEN); // Healthy
	}
	else if (boss->health > 5) {
		setfillcolor(YELLOW); // Mid-health
	}
	else {
		setfillcolor(RED); // Low health
	}

	// Draw the current health bar with smooth size based on health
	fillrectangle(boss->x, boss->y - 20, boss->x + healthWidth, boss->y - 20 + barHeight);

	// Optional: Add visual feedback like a flashing border when the boss is damaged
	if (boss->health <= 5) {
		setlinecolor(RED);
		rectangle(boss->x - 2, boss->y - 22, boss->x + 102, boss->y - 8); // Flashing red border
	}
}


// Function to draw player's bullets as filled circles
void DrawPlayerBullets() {
	setfillcolor(YELLOW);
	for (auto& p : bulletList) {
		fillcircle(p.x, p.y, 5);
	}
}

// Function to draw enemy bullets as small filled circles
void DrawEnemyBullets() {
	for (auto& p : enemyBulletList) {
		if (p.ownerId == -1) { // Boss's bullets
			setfillcolor(WHITE); // Different color for boss bullets
			fillcircle(p.x, p.y, 6); // Larger size
		}
		else { // Regular enemy bullets
			setfillcolor(GREEN);
			fillcircle(p.x, p.y, 3);
		}
	}
}

// Function to handle keyboard input using GetAsyncKeyState for continuous movement
void HandleMovement() {
	// Define constants for movement and boundaries
	const int planeWidth = 80;
	const int planeHeight = 60;
	const int speed = 8;
	const int fireCooldown = 300; // Cooldown between bullets in milliseconds

	// Using high-resolution clock for precise timing
	static std::chrono::steady_clock::time_point lastFireTime = std::chrono::steady_clock::now();

	// Up arrow key
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		myPlane.y -= speed;
		if (myPlane.y < 0) myPlane.y = 0;
	}

	// Down arrow key
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		myPlane.y += speed;
		if (myPlane.y > WIN_HEIGHT - planeHeight) myPlane.y = WIN_HEIGHT - planeHeight;
	}

	// Left arrow key
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		myPlane.x -= speed;
		if (myPlane.x < 0) myPlane.x = 0;
	}

	// Right arrow key
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		myPlane.x += speed;
		if (myPlane.x > WIN_WIDTH - planeWidth) myPlane.x = WIN_WIDTH - planeWidth;
	}

	// Spacebar to fire bullets
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		// Get current time using high-resolution clock
		auto currentTime = std::chrono::steady_clock::now();
		auto timeSinceLastFire = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFireTime).count();

		if (timeSinceLastFire >= fireCooldown) { // Fire every fireCooldown ms
			// Create bullet at the center of the plane
			bulletList.emplace_back(myPlane.x + planeWidth / 2, myPlane.y, DEFAULT_BULLET_SPEED);

			// Update last fire time
			lastFireTime = currentTime;
		}
	}
}

// Function to handle collision between enemy bullets and player's plane
void CheckEnemyBulletCollision() {
	// Define the player's plane bounding box for collision
	int playerLeft = myPlane.x;
	int playerRight = myPlane.x + 80;
	int playerTop = myPlane.y;
	int playerBottom = myPlane.y + 60;

	for (auto p = enemyBulletList.begin(); p != enemyBulletList.end();) {
		// Simple bounding box collision detection
		if (p->x >= playerLeft && p->x <= playerRight &&
			p->y >= playerTop && p->y <= playerBottom) {
			// Collision detected
			gameOver = true;
			return;
		}
		else {
			++p;
		}
	}
}

// Function to handle collision between player's bullets and enemy planes
void CheckBulletCollision(bool& gameWinFlag) {
	// First, handle collisions with regular enemies
	for (auto pDj = enemyPlaneList.begin(); pDj != enemyPlaneList.end();) {
		bool planeDestroyed = false;
		int destroyedPlaneId = pDj->id;

		for (auto pZd = bulletList.begin(); pZd != bulletList.end();) {
			// Simple bounding box collision detection
			if (pZd->x >= (pDj->x - 10) && pZd->x <= (pDj->x + 50 + 10) &&
				pZd->y >= (pDj->y - 10) && pZd->y <= (pDj->y + 30 + 10)) {
				// Collision detected
				pZd = bulletList.erase(pZd);

				pDj->health -= 1;

				if (pDj->health <= 0) {
					pDj = enemyPlaneList.erase(pDj);
					planeDestroyed = true;
					enemiesDestroyed++;
				}
				else {
					++pDj;
				}
				break;
			}
			else {
				++pZd;
			}
		}

		if (planeDestroyed) {
			// Remove all bullets fired by the destroyed enemy plane
			for (auto it = enemyBulletList.begin(); it != enemyBulletList.end();) {
				if (it->ownerId == destroyedPlaneId) {
					it = enemyBulletList.erase(it);
				}
				else {
					++it;
				}
			}
		}
		else {
			++pDj;
		}
	}

	// Now, handle collisions with the boss
	if (bossSpawned && boss != nullptr) {
		for (auto pZd = bulletList.begin(); pZd != bulletList.end();) {
			// Simple bounding box collision detection
			if (pZd->x >= (boss->x - 10) && pZd->x <= (boss->x + 100 + 10) &&
				pZd->y >= (boss->y - 10) && pZd->y <= (boss->y + 60 + 10)) {
				// Collision detected
				pZd = bulletList.erase(pZd); // Remove bullet

				boss->health -= 1; // Decrease boss health

				if (boss->health <= 0) {
					// Boss defeated
					delete boss;
					boss = nullptr;
					bossSpawned = false;
					enemiesDestroyed = 0;
					gameWinFlag = true;
				}
				else {
				}
				break;
			}
			else {
				++pZd;
			}
		}
	}
}

// Function to remove off-screen bullets and enemy planes
void Cleanup() {
	// Remove enemy planes that move out of the screen
	for (auto p = enemyPlaneList.begin(); p != enemyPlaneList.end();) {
		if (p->y > WIN_HEIGHT) {
			p = enemyPlaneList.erase(p);
		}
		else {
			++p;
		}
	}

	// Remove player's bullets that move out of the screen
	for (auto p = bulletList.begin(); p != bulletList.end();) {
		if (p->y < 0) {
			p = bulletList.erase(p);
		}
		else {
			++p;
		}
	}

	// Remove enemy bullets that move out of the screen (with some buffer for spread bullets)
	for (auto p = enemyBulletList.begin(); p != enemyBulletList.end();) {
		if (p->y > WIN_HEIGHT || p->x < -20 || p->x > WIN_WIDTH + 20) {
			p = enemyBulletList.erase(p);
		}
		else {
			++p;
		}
	}
}

// Function to draw debug information
void DrawDebugInfo(int x = 10, int y = 10) {
	// Set font style and color for debug text
	settextcolor(YELLOW);
	settextstyle(16, 0, _T("Consolas"));

	// Create a semi-transparent gradient background with rounded corners
	for (int i = 0; i < 100; i++) {
		setfillcolor(RGB(0, 0, 0, 128 - i)); // Decrease opacity for gradient
		solidrectangle(x - 5, y - 5 + i, x + 250, y - 5 + i + 1);
	}

	// Draw the rounded rectangle with a border
	setfillcolor(RGB(0, 0, 0)); // Main rectangle color
	fillroundrect(x - 5, y - 5, x + 250, y + 100, 10, 10); // Rounded rectangle

	setlinecolor(WHITE); // Outline color
	setlinestyle(PS_SOLID, 1); // Solid line
	rectangle(x - 5, y - 5, x + 250, y + 100); // Outline rectangle

	// Display enemies destroyed
	wchar_t debugText[256];
	swprintf(debugText, 256, L"Enemies Destroyed: %d", enemiesDestroyed);
	outtextxy(x, y, debugText);

	// Display number of enemies on screen
	swprintf(debugText, 256, L"Enemies on Screen: %d", enemyPlaneList.size());
	outtextxy(x, y + 20, debugText);

	// Display player bullets on screen
	swprintf(debugText, 256, L"Player Bullets: %d", bulletList.size());
	outtextxy(x, y + 40, debugText);

	// Display enemy bullets on screen
	swprintf(debugText, 256, L"Enemy Bullets: %d", enemyBulletList.size());
	outtextxy(x, y + 60, debugText);

	// Display boss health if the boss is spawned
	if (bossSpawned && boss != nullptr) {
		swprintf(debugText, 256, L"Boss Health: %d", boss->health);
		outtextxy(x, y + 80, debugText);
	}

	// Optional: Calculate and display FPS (Frames Per Second)
	static auto lastTime = std::chrono::steady_clock::now();
	auto currentTime = std::chrono::steady_clock::now();
	double elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
	lastTime = currentTime;

	int fps = (elapsedTime > 0) ? static_cast<int>(1000.0 / elapsedTime) : 0;
	swprintf(debugText, 256, L"FPS: %d", fps);
	outtextxy(x, y + 100, debugText);
}

// Function to draw the menu with 

void DrawMenu() {
	// Set semi-transparent background
	setfillcolor(RGB(0, 0, 0)); // Black background
	setbkcolor(TRANSPARENT);
	solidrectangle(WIN_WIDTH / 2 - 150, WIN_HEIGHT / 2 - 100, WIN_WIDTH / 2 + 150, WIN_HEIGHT / 2 + 100);

	// Set text style and color for menu options
	settextcolor(WHITE);
	settextstyle(28, 0, _T("Arial"));

	// Center text dynamically based on the window size
	outtextxy(WIN_WIDTH / 2 - textwidth(_T("Game Paused")) / 2, WIN_HEIGHT / 2 - 80, _T("Game Paused"));
	outtextxy(WIN_WIDTH / 2 - textwidth(_T("Press 'M' to Resume")) / 2, WIN_HEIGHT / 2 - 40, _T("Press 'M' to Resume"));
	outtextxy(WIN_WIDTH / 2 - textwidth(_T("Press 'Esc' to Exit")) / 2, WIN_HEIGHT / 2, _T("Press 'Esc' to Exit"));

	// Optional: Highlight currently selected option
	static bool isResumeSelected = true; // Toggle between options

	// Highlight the selected option
	if (isResumeSelected) {
		settextcolor(YELLOW);
		outtextxy(WIN_WIDTH / 2 - textwidth(_T("Press 'M' to Resume")) / 2, WIN_HEIGHT / 2 - 40, _T("Press 'M' to Resume"));
	}
	else {
		settextcolor(YELLOW);
		outtextxy(WIN_WIDTH / 2 - textwidth(_T("Press 'Esc' to Exit")) / 2, WIN_HEIGHT / 2, _T("Press 'Esc' to Exit"));
	}

	// Handle option selection (optional: arrow keys or mouse input to toggle)
	if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {
		isResumeSelected = !isResumeSelected; // Toggle between options
	}
}

// Function declarations
void BossSpreadShoot();
void BossCircularShoot(int numBullets, double speed);

// Function to handle shooting by enemies and boss with enhanced weapons
void EnemyShoot(DWORD currentTime, DWORD& lastEnemyShootTime) {
	// Enemy planes shoot every 2000ms
	if (currentTime - lastEnemyShootTime >= 2000) {
		for (auto& enemy : enemyPlaneList) {
			// Each enemy fires a bullet from the bottom center of the plane
			enemyBulletList.emplace_back(enemy.x + 25, enemy.y + 30, enemy.id);
		}
		lastEnemyShootTime = currentTime;
	}

	// Boss shoots every 800ms with enhanced weapons
	static DWORD lastBossShootTime = 0;
	if (bossSpawned && boss != nullptr && currentTime - lastBossShootTime >= 800) {
		// Spread shooting pattern
		BossSpreadShoot();

		// Circular shooting pattern
		BossCircularShoot(8, 5); // 8 bullets, speed of 5

		lastBossShootTime = currentTime;
	}
}

// Function for boss spread shooting pattern
void BossSpreadShoot() {
	if (boss == nullptr) return; // Ensure the boss exists
	enemyBulletList.emplace_back(boss->x + 30, boss->y + 60, -1, -2, 8); // Left bullet
	enemyBulletList.emplace_back(boss->x + 50, boss->y + 60, -1, 0, 8); // Center bullet
	enemyBulletList.emplace_back(boss->x + 70, boss->y + 60, -1, 2, 8); // Right bullet
}

// Function for boss circular shooting pattern
void BossCircularShoot(int numBullets, double speed) {
	if (boss == nullptr) return; // Ensure the boss exists
	double angleIncrement = 360.0 / numBullets;
	for (int i = 0; i < numBullets; ++i) {
		double angleDeg = i * angleIncrement;
		double angleRad = angleDeg * (PI / 180.0);
		int dx = static_cast<int>(speed * cos(angleRad));
		int dy = static_cast<int>(speed * sin(angleRad));

		// Initialize bullets slightly away from the boss
		int startX = boss->x + 50 + dx * 2;
		int startY = boss->y + 60 + dy * 2;

		enemyBulletList.emplace_back(startX, startY, -1, dx, dy);
	}
}

// Function to display a message in the center of the screen
void DisplayCenteredMessage(const wchar_t* message) {
	settextcolor(RED);
	settextstyle(48, 0, _T("Arial"));

	// Calculate text width and height for centering
	int textWidth = textwidth(message);
	int textHeight = textheight(message);

	outtextxy((WIN_WIDTH - textWidth) / 2, (WIN_HEIGHT - textHeight) / 2, message);
}

// Function to display a message and wait for a mouse click to exit
void DisplayMessageAndWait(const wchar_t* message) {
	cleardevice(); // Clear the screen before displaying the message
	DisplayCenteredMessage(message);
	FlushBatchDraw();

	// Wait for a mouse click to exit
	ExMessage msg;
	while (true) {
		if (peekmessage(&msg, EM_MOUSE)) {
			if (msg.message == WM_LBUTTONDOWN) { // Left mouse button click
				break;
			}
		}
		Sleep(10);
	}
}
// Game state variables


// Declare your structures and classes here...

// Background variables
IMAGE im_bk; // Background image
int dx = 0; // Offset for scrolling
const int speed = 3; // Speed of scrolling

int main() {
	// Initialize random seed
	srand((unsigned int)time(NULL));

	// Initialize graphics window without showing the console
	initgraph(WIN_WIDTH, WIN_HEIGHT, SHOWCONSOLE);
	ShowCursor(false); // Hide the cursor

	// Load the background image
	loadimage(&im_bk, _T("Z:/Desktop/assets/skybg.png"));

	// Initialize timing variables
	DWORD lastEnemySpawnTime = GetTickCount();
	DWORD lastEnemyShootTime = GetTickCount();

	// Flags for debug and menu
	bool showDebug = false;
	bool showMenuFlag = false;

	// Main game loop
	while (!gameOver && !gameWin) {
		DWORD currentTime = GetTickCount();

		// Toggle Debug Info with F1
		if (GetAsyncKeyState(VK_F1) & 0x8000) {
			showDebug = !showDebug;
			Sleep(200); // Debounce delay
		}

		// Toggle Menu with M
		if (GetAsyncKeyState('M') & 0x8000) {
			showMenuFlag = !showMenuFlag;
			Sleep(200); // Debounce delay
		}

		if (showMenuFlag) {
			// Draw Menu
			BeginBatchDraw();
			DrawMenu();
			EndBatchDraw();

			// Handle Menu Inputs
			if (GetAsyncKeyState('M') & 0x8000) { // Resume game with 'M'
				showMenuFlag = false;
				Sleep(200); // Debounce delay
			}
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) { // Exit game with 'Esc'
				gameOver = true;
			}
			Sleep(10); // Small delay to prevent high CPU usage
			continue; // Skip the rest of the loop to pause game updates
		}

		// Scrolling background logic
		putimage(0, 0, dx, WIN_HEIGHT, &im_bk, WIN_WIDTH - dx, 0); // Move the small part
		putimage(dx, 0, WIN_WIDTH - dx, WIN_HEIGHT, &im_bk, 0, 0); // Move the big part
		dx += speed;
		if (dx >= WIN_WIDTH) {
			dx = speed; // Reset dx to loop the background
		}

		// Spawn a new enemy plane every 1000ms if boss is not spawned
		if (!bossSpawned && currentTime - lastEnemySpawnTime >= 1000) {
			int enemyX = rand() % (WIN_WIDTH - 50);
			enemyPlaneList.emplace_back(enemyX, 0, nextEnemyId++);
			lastEnemySpawnTime = currentTime;
		}

		// Spawn the boss when 10 enemies are destroyed
		if (enemiesDestroyed >= 10 && !bossSpawned) {
			boss = new BossPlane(WIN_WIDTH / 2 - 50, WIN_HEIGHT / 2 - 30); // Centered (adjusted for size)
			bossSpawned = true;
		}

		// Enemy planes shoot bullets periodically
		EnemyShoot(currentTime, lastEnemyShootTime);

		// Handle player movement and firing
		HandleMovement();

		// Move player's bullets upwards
		for (auto& bullet : bulletList) {
			bullet.y -= bullet.speed;
		}

		// Move enemy planes downwards
		for (auto& enemy : enemyPlaneList) {
			enemy.y += 2;
		}

		// Move enemy bullets
		for (auto& enemyBullet : enemyBulletList) {
			enemyBullet.x += enemyBullet.dx;
			enemyBullet.y += enemyBullet.dy;
		}

		// Check collisions
		CheckBulletCollision(gameWin);
		CheckEnemyBulletCollision();
		if (gameOver) {
			break;
		}

		// Clean up off-screen objects
		Cleanup();

		// Drawing phase
		BeginBatchDraw();
		cleardevice(); // Clear the screen before drawing

		// Draw all game objects
		DrawMyPlane();
		DrawEnemyPlanes();
		DrawPlayerBullets();
		DrawEnemyBullets();

		// Draw the boss if it exists
		if (bossSpawned && boss != nullptr) {
			DrawBoss();
		}

		// Draw debug info if enabled
		if (showDebug) {
			DrawDebugInfo();
		}

		EndBatchDraw(); // Finalize drawing

		// Control game speed (approximately 60 FPS)
		Sleep(16);
	}

	// Display end game message
	BeginBatchDraw();
	cleardevice();
	if (gameWin) {
		DisplayMessageAndWait(L"GOOD GAME WELLPLAYED");
	}
	else {
		DisplayMessageAndWait(L"GAME OVER");
	}
	EndBatchDraw();

	// Clean up boss if it exists
	if (boss != nullptr) {
		delete boss;
		boss = nullptr;
	}

	closegraph();
	return 0;
}