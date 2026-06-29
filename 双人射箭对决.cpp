/*这个小程序的灵感来自我2017年读初一时做的小游戏 
当时由贪吃蛇源码修改而来，称为弓箭手大作战
现在的C++代码使用AI辅助编写，修复了许多问题 
祝你天天开心(′·ω·`) 
——东风九号  2026.6.20 
*/

#include <conio.h>
#include <iostream>
#include <windows.h>
#include <vector>
#include <algorithm>
//#include <cstring>
using namespace std;

HANDLE print = GetStdHandle(STD_OUTPUT_HANDLE);
int SHOOT_COOLDOWN = 300;

// ========== 障碍物地图 ==========
const int MAP_W = 24;
const int MAP_H = 21;
bool isPause = false;
bool back = false;
int sleeptime = 10;
int gameMap[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};
bool IsBlock(int x, int y)
{
    if(x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return true;
    return gameMap[y][x] == 1;
}

// 玩家结构体
struct Player {
    int x, y;
    int lastX, lastY;
    int hp;
    int hpmax;
    int color;
    DWORD lastShootTime;
} p1 = {4, 15, 4, 15, 100, 100, 0xa, 0}, p2 = {15, 4, 15, 4, 100, 100, 0xc, 0};

// 箭矢结构体
struct Arrow {
    int x, y;
    int lastX, lastY;
    int dir;
    int owner;
    int color;
    bool alive;
};
vector<Arrow> arrows;

void gotoxy(int x, int y, unsigned color, const char* ch);
void drawGame();
void handleInput();
void updateArrows();
void checkHit();
void showMenu();
void showHelp();
bool isGameOver();
void Settings();
void aboutgame();
const char* GetArrowSymbol(int dir);

/* 调整控制台全局字体大小
// height：字体像素高度（常用12/16/20/24，数字越大字越大）
void SetConsoleFont(int fontHeight)
{
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi;
    ZeroMemory(&cfi, sizeof(cfi));
    cfi.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hOutput, FALSE, &cfi);
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = fontHeight;
    wcscpy(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hOutput, FALSE, &cfi);
}
*/

void gotoxy(int x, int y, unsigned color, const char* ch) {
    COORD pos;
    pos.X = x << 1;
    pos.Y = y;
    SetConsoleTextAttribute(print, color);
    SetConsoleCursorPosition(print, pos);
    printf("%s", ch);
}

void DrawPauseTip()
{
    // 居中绘制暂停文字
    gotoxy(35, 10, 0xe, "===== 游戏暂停 =====");
    gotoxy(35, 12, 0xe, "按 ESC 继续游戏");
    gotoxy(35, 13, 0xe, "按 M 返回主菜单");
}

void ClearScreen()
{
    COORD coordScreen = { 0, 0 };
    DWORD dwCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 获取缓冲区总字符数
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // 用空格填满整个缓冲区
    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &dwCharsWritten);
    FillConsoleOutputAttribute(hConsole, 0, dwConSize, coordScreen, &dwCharsWritten);

    // 光标放回左上角
    SetConsoleCursorPosition(hConsole, coordScreen);
}

const char* GetArrowSymbol(int dir)
{
    switch (dir)
    {
        case 1: return "↑";
        case 2: return "↓";
        case 3: return "←";
        case 4: return "→";
        default: return "  ";
    }
}

void drawGame() {
    // 无system("cls")，靠重绘全覆盖消除残影
    // 1. 重绘完整墙体边框，跳过玩家坐标不擦方块
    for(int y=0;y<MAP_H;y++)
    {
        for(int x=0;x<MAP_W;x++)
        {
            // 跳过玩家当前格子，不覆盖空白
            if((x == p1.x && y == p1.y) || (x == p2.x && y == p2.y))
                continue;

            if(gameMap[y][x]==1)
                gotoxy(x,y,0x8,"■");
            else
                gotoxy(x,y,0,"  ");
        }
    }

    // 2. 仅移动时擦除玩家上一帧旧位置，静止不刷新
    if (!(p1.x == p1.lastX && p1.y == p1.lastY))
        gotoxy(p1.lastX, p1.lastY, 0, "  ");
    if (!(p2.x == p2.lastX && p2.lastY))
        gotoxy(p2.lastX, p2.lastY, 0, "  ");
    gotoxy(p1.x, p1.y, p1.color, "■");
    gotoxy(p2.x, p2.y, p2.color, "■");

    // 3. 擦除旧箭矢，绘制新箭矢
    for (auto& arr : arrows) {
        if (arr.alive && arr.lastX != p1.x && arr.lastX != p2.x)
            gotoxy(arr.lastX, arr.lastY, 0, "  ");
    }
    for (auto& arr : arrows) {
        if (arr.alive) {
            const char* sym = GetArrowSymbol(arr.dir);
            gotoxy(arr.x, arr.y, arr.color, sym);
        }
    }

    // 4. 每次重绘血条，覆盖旧文字
    gotoxy(27, 8, 0, "        ");
    gotoxy(27, 8, 0xa, "player 1:");
    gotoxy(27, 9, 0, "        ");
    gotoxy(27, 9, 0xa, "HP:");
    printf("%d    ", p1.hp);

    gotoxy(27, 13, 0, "        ");
    gotoxy(27, 13, 0xc, "player 2:");
    gotoxy(27, 14, 0, "        ");
    gotoxy(27, 14, 0xc, "HP:");
    printf("%d    ", p2.hp);
}

void handleInput() {
    if (!_kbhit()) return;
    int ch = _getch();
    DWORD currentTime = GetTickCount();
	if (ch == 27){
    isPause = true;
    return;
}
    //玩家1 WASD
    if (ch != 0 && ch != 224) {
        if (ch == 'w' || ch == 'W') {
            int nx=p1.x, ny=p1.y-1;
            if(!IsBlock(nx,ny) && !(nx==p2.x&&ny==p2.y)){
                p1.lastX = p1.x;
                p1.lastY = p1.y;
                p1.y=ny;
            }
        } else if (ch == 's' || ch == 'S') {
            int nx=p1.x, ny=p1.y+1;
            if(!IsBlock(nx,ny) && !(nx==p2.x&&ny==p2.y)){
                p1.lastX = p1.x;
                p1.lastY = p1.y;
                p1.y=ny;
            }
        } else if (ch == 'a' || ch == 'A') {
            int nx=p1.x-1, ny=p1.y;
            if(!IsBlock(nx,ny) && !(nx==p2.x&&ny==p2.y)){
                p1.lastX = p1.x;
                p1.lastY = p1.y;
                p1.x=nx;
            }
        } else if (ch == 'd' || ch == 'D') {
            int nx=p1.x+1, ny=p1.y;
            if(!IsBlock(nx,ny) && !(nx==p2.x&&ny==p2.y)){
                p1.lastX = p1.x;
                p1.lastY = p1.y;
                p1.x=nx;
            }
        }
        //I上箭
        else if (ch == 'i' || ch == 'I') {
            if (currentTime - p1.lastShootTime >= SHOOT_COOLDOWN) {
                Arrow newArr;
                newArr.x = p1.x;
                newArr.y = p1.y;
                newArr.lastX = p1.x;
                newArr.lastY = p1.y;
                newArr.dir = 1;
                newArr.owner = 1;
                newArr.color = 0xa;
                newArr.alive = true;
                arrows.push_back(newArr);
                p1.lastShootTime = currentTime;
            }
        }
        //K下箭
        else if (ch == 'k' || ch == 'K') {
            if (currentTime - p1.lastShootTime >= SHOOT_COOLDOWN) {
                Arrow newArr;
                newArr.x = p1.x;
                newArr.y = p1.y;
                newArr.lastX = p1.x;
                newArr.lastY = p1.y;
                newArr.dir = 2;
                newArr.owner = 1;
                newArr.color = 0xa;
                newArr.alive = true;
                arrows.push_back(newArr);
                p1.lastShootTime = currentTime;
            }
        }
        //J左箭
        else if (ch == 'j' || ch == 'J') {
            if (currentTime - p1.lastShootTime >= SHOOT_COOLDOWN) {
                Arrow newArr;
                newArr.x = p1.x;
                newArr.y = p1.y;
                newArr.lastX = p1.x;
                newArr.lastY = p1.y;
                newArr.dir = 3;
                newArr.owner = 1;
                newArr.color = 0xa;
                newArr.alive = true;
                arrows.push_back(newArr);
                p1.lastShootTime = currentTime;
            }
        }
        //L右箭
        else if (ch == 'l' || ch == 'L') {
            if (currentTime - p1.lastShootTime >= SHOOT_COOLDOWN) {
                Arrow newArr;
                newArr.x = p1.x;
                newArr.y = p1.y;
                newArr.lastX = p1.x;
                newArr.lastY = p1.y;
                newArr.dir = 4;
                newArr.owner = 1;
                newArr.color = 0xa;
                newArr.alive = true;
                arrows.push_back(newArr);
                p1.lastShootTime = currentTime;
            }
        }
    }

    //玩家2方向键
    if (ch == 0 || ch == 224) {
        ch = _getch();
        if (ch == 72) {
            int nx=p2.x, ny=p2.y-1;
            if(!IsBlock(nx,ny) && !(nx==p1.x&&ny==p1.y)){
                p2.lastX = p2.x;
                p2.lastY = p2.y;
                p2.y=ny;
            }
        } else if (ch == 80) {
            int nx=p2.x, ny=p2.y+1;
            if(!IsBlock(nx,ny) && !(nx==p1.x&&ny==p1.y)){
                p2.lastX = p2.x;
                p2.lastY = p2.y;
                p2.y=ny;
            }
        } else if (ch == 75) {
            int nx=p2.x-1, ny=p2.y;
            if(!IsBlock(nx,ny) && !(nx==p1.x&&ny==p1.y)){
                p2.lastX = p2.x;
                p2.lastY = p2.y;
                p2.x=nx;
            }
        } else if (ch == 77) {
            int nx=p2.x+1, ny=p2.y;
            if(!IsBlock(nx,ny) && !(nx==p1.x&&ny==p1.y)){
                p2.lastX = p2.x;
                p2.lastY = p2.y;
                p2.x=nx;
            }
        }
    }
    //玩家2 8426射箭
    else if (ch == '8') {
        if (currentTime - p2.lastShootTime >= SHOOT_COOLDOWN) {
            Arrow newArr;
            newArr.x = p2.x;
            newArr.y = p2.y;
            newArr.lastX = p2.x;
            newArr.lastY = p2.y;
            newArr.dir = 1;
            newArr.owner = 2;
            newArr.color = 0xc;
            newArr.alive = true;
            arrows.push_back(newArr);
            p2.lastShootTime = currentTime;
        }
    } else if (ch == '2') {
        if (currentTime - p2.lastShootTime >= SHOOT_COOLDOWN) {
            Arrow newArr;
            newArr.x = p2.x;
            newArr.y = p2.y;
            newArr.lastX = p2.x;
            newArr.lastY = p2.y;
            newArr.dir = 2;
            newArr.owner = 2;
            newArr.color = 0xc;
            newArr.alive = true;
            arrows.push_back(newArr);
            p2.lastShootTime = currentTime;
        }
    } else if (ch == '4') {
        if (currentTime - p2.lastShootTime >= SHOOT_COOLDOWN) {
            Arrow newArr;
            newArr.x = p2.x;
            newArr.y = p2.y;
            newArr.lastX = p2.x;
            newArr.lastY = p2.y;
            newArr.dir = 3;
            newArr.owner = 2;
            newArr.color = 0xc;
            newArr.alive = true;
            arrows.push_back(newArr);
            p2.lastShootTime = currentTime;
        }
    } else if (ch == '6') {
        if (currentTime - p2.lastShootTime >= SHOOT_COOLDOWN) {
            Arrow newArr;
            newArr.x = p2.x;
            newArr.y = p2.y;
            newArr.lastX = p2.x;
            newArr.lastY = p2.y;
            newArr.dir = 4;
            newArr.owner = 2;
            newArr.color = 0xc;
            newArr.alive = true;
            arrows.push_back(newArr);
            p2.lastShootTime = currentTime;
        }
    }
}

void updateArrows() {
    for (auto& arr : arrows) {
        if (!arr.alive) continue;
        arr.lastX = arr.x;
        arr.lastY = arr.y;

        int nx = arr.x, ny = arr.y;
        switch (arr.dir) {
            case 1: ny--; break;
            case 2: ny++; break;
            case 3: nx--; break;
            case 4: nx++; break;
        }

        // ========== 预判下一帧是否命中玩家 ==========
        bool hitPlayer = false;
        if(arr.owner == 1){
            // 玩家1的箭预判下一格是不是玩家2
            if(nx == p2.x && ny == p2.y){
                p2.hp -= 40;
                hitPlayer = true;
            }
        }else{
            // 玩家2的箭预判下一格是不是玩家1
            if(nx == p1.x && ny == p1.y){
                p1.hp -= 40;
                hitPlayer = true;
            }
        }
        if(hitPlayer){
            arr.alive = false;
            continue;
        }

        // 墙体阻挡，玩家不阻挡箭矢
        if(IsBlock(nx, ny))
        {
            arr.alive = false;
            continue;
        }

        arr.x = nx;
        arr.y = ny;
        if (arr.x < 1 || arr.x > 23 || arr.y < 1 || arr.y > 19) {
            arr.alive = false;
        }
    }
    arrows.erase(remove_if(arrows.begin(), arrows.end(), [](const Arrow& a) {
        return !a.alive;
    }), arrows.end());
}

void checkHit() {
    // 命中逻辑已移到updateArrows预判，本函数可保留空或删除
}

bool isGameOver() {
    if (p1.hp <= 0) {
        ClearScreen();
        gotoxy(15, 12, 0xc, "玩家2胜利！");
        Sleep(3000);
        return true;
    }
    if (p2.hp <= 0) {
        ClearScreen();
        gotoxy(15, 12, 0xa, "玩家1胜利！");
        Sleep(3000);
        return true;
    }
	if (back == true) {
        ClearScreen();
        gotoxy(15, 12, 0xf, "游戏结束");
        Sleep(1500);
        return true;
    }
    return false;
}

void showMenu() {
	int j;
    ClearScreen();
    gotoxy(15,10,0xf,"┏━━━━━━━━━━━━┓");
	gotoxy(15,11,0xf,"┃双人射箭对决┃");
	gotoxy(15,12,0xf,"┗━━━━━━━━━━━━┛"); 
	gotoxy(15,16,0xe,"东风九号 制作"); 
	gotoxy(15,18,0xf,"按任意键开始游戏");
    _getch();
    ClearScreen();
    for(j=1;j>=1;j++){
	gotoxy(0,0,0xd,"1.双人模式\n2.人机模式（暂未开放）\n3.操作说明\n4.设置\n5.关于\n6.退出游戏"); 
	char ch1=_getch();
	if(ch1=='1'){break;}
	//if(ch1=='2'){ClearScreen();gotoxy(0,0,0xf,"暂未开放");Sleep(2500);continue;}
	if(ch1=='3'){ClearScreen();showHelp();continue;}
	if(ch1=='4'){ClearScreen();Settings();}
	if(ch1=='5'){ClearScreen();aboutgame();}
	if(ch1=='6'||ch1==27){exit(0);}
	else{continue;}
}
}

void Settings(){
	gotoxy(2, 0, 0xf, "全局帧刷新间隔(Q)");
	gotoxy(2, 1, 0xf, "1.10（默认） 2.25  3.50  4.100");
	gotoxy(2, 2, 0xf, "玩家生命值(W)");
	gotoxy(2, 3, 0xf, "1.50  2.100（默认） 3.150  4.200");
	gotoxy(2, 4, 0xf, "箭矢发射间隔(E)");
	gotoxy(2, 5, 0xf, "1.100  2.200  3.300（默认） 4.400");
	gotoxy(2, 6, 0xf, "按Esc键返回");
	while(true){
	gotoxy(0, 8, 0xe, "当前帧刷新间隔：");printf("%i    ",sleeptime);
	gotoxy(0, 9, 0xe, "当前玩家最大生命值：");printf("%i    ",p1.hpmax);
	gotoxy(0, 10, 0xe, "当前射箭间隔：");printf("%i    ",SHOOT_COOLDOWN);
	char ch2;
	char ch1=_getch();
	if(ch1=='q'||ch1=='Q'){
	gotoxy(0, 0, 0xc, "■");
	gotoxy(0, 2, 0xf, "  ");
	gotoxy(0, 4, 0xf, "  ");
		ch2=_getch();
		if(ch2=='1'){sleeptime=10;}
		if(ch2=='2'){sleeptime=25;}
		if(ch2=='3'){sleeptime=50;}
		if(ch2=='4'){sleeptime=100;}
		gotoxy(0, 0, 0xf, "  ");
		continue;
	}
	if(ch1=='w'||ch1=='W'){
	gotoxy(0, 0, 0xf, "  ");
	gotoxy(0, 2, 0xc, "■");
	gotoxy(0, 4, 0xf, "  ");
		ch2=_getch();
		if(ch2=='1'){p1.hpmax=50;p2.hpmax=50;}
		if(ch2=='2'){p1.hpmax=100;p2.hpmax=100;}
		if(ch2=='3'){p1.hpmax=150;p2.hpmax=150;}
		if(ch2=='4'){p1.hpmax=200;p2.hpmax=200;}
		gotoxy(0, 2, 0xf, "  ");
		p1.hp = p1.hpmax;
    	p2.hp = p2.hpmax;
		continue;
	}
	if(ch1=='e'||ch1=='E'){
	gotoxy(0, 0, 0xf, "  ");
	gotoxy(0, 2, 0xf, "  ");
	gotoxy(0, 4, 0xc, "■");
		ch2=_getch();
		if(ch2=='1'){SHOOT_COOLDOWN=100;}
		if(ch2=='2'){SHOOT_COOLDOWN=200;}
		if(ch2=='3'){SHOOT_COOLDOWN=300;}
		if(ch2=='4'){SHOOT_COOLDOWN=400;}
		gotoxy(0, 4, 0xf, "  ");
		continue;
	}
	else if(ch1==27){ClearScreen();return;}
	continue;
	}
}

void showHelp() {
    gotoxy(0, 0, 0xf, "玩家1：WASD移动，IJKL射箭");
    gotoxy(0, 1, 0xf, "玩家2：↑←↓→移动，8426射箭");
    gotoxy(0, 2, 0xf, "玩家默认100点HP，射箭固定减少40点HP");
    gotoxy(0, 3, 0xf, "方块会阻挡箭矢");
    gotoxy(0, 4, 0xf, "使用Esc暂停游戏，暂停时按M返回主菜单");
    gotoxy(0, 6, 0xf, "按任意键返回菜单...");
    _getch();
    ClearScreen();
}

void aboutgame() {
	printf("本程序使用C++编写\n");
	Sleep(2000);
	printf("这个小游戏的灵感来自我2017年读初一时做的小游戏\n");
	Sleep(2000);
	printf("当时由贪吃蛇源码修改而来，称为弓箭手大作战\n");
	Sleep(2000);
	printf("现在的代码使用AI辅助编程，修复了许多问题\n");
	Sleep(2000);
	printf("祝你天天开心(^_^)\n");
	Sleep(2000);
	printf("——作者  东风九号  2026.6.20\n");
	Sleep(2000);
	printf("\n按任意键返回菜单...");
    _getch();
    ClearScreen();
}

int main() {
    CONSOLE_CURSOR_INFO cursor;
    cursor.dwSize = 1;
    cursor.bVisible = FALSE;
    SetConsoleCursorInfo(print, &cursor);
	showMenu();
    while (true) {
    drawGame();

    // 暂停判断
    if (isPause)
    {
        DrawPauseTip(); // 显示暂停文字
        char ch = _getch();
        if (ch == 27) // ESC恢复
        {
        	isPause = false;
        	ClearScreen();
        }
        if (ch == 'm' || ch == 'M') // 返回主菜单 
        {
            ClearScreen();
            back = true;
        if (isGameOver()) {
        p1.hp = p1.hpmax;
    	p2.hp = p2.hpmax;
        p1 = {4, 15, 4, 15, p1.hp, p1.hpmax, 0xa, 0};
        p2 = {15, 4, 15, 4, p2.hp, p2.hpmax, 0xc, 0};
        arrows.clear();
        isPause = false; // 重置暂停状态
        back = false;
        showMenu();
    }
    Sleep(sleeptime);
    continue;
        }
        Sleep(sleeptime);
        continue; // 跳过更新、输入逻辑
    }

    // 未暂停才执行游戏逻辑
    handleInput();
    updateArrows();
    checkHit();

    if (isGameOver()) {
    	p1.hp = p1.hpmax;
    	p2.hp = p2.hpmax;
        p1 = {4, 15, 4, 15, p1.hp, p1.hpmax, 0xa, 0};
        p2 = {15, 4, 15, 4, p2.hp, p2.hpmax, 0xc, 0};
        arrows.clear();
        isPause = false; // 重置暂停状态
        back = false;
        showMenu();
    }
    Sleep(sleeptime);
}
    return 0;
}
