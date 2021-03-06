// Snake.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#define new(type) (type*)malloc(sizeof(type))

#define WIDTH 100
#define HEIGHT 200
#define FPS 120.0
#define ROW 30
#define COLUMN 40

#define CHAR_LENGTH 2

#define INITSNAKELENGTH 4

#define STR_WALL "█"
#define STR_SNAKE "█"
#define STR_BEAN "★"
#define STR_EMPTY " "

void delLine(short y)
{
	HANDLE hOutput;
	//窗口缓存信息
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	DWORD len, nw;
	//用MSDN上的TCHAR类型跪了，换成char就好
	char fillchar = ' ';
	//定位光标
	COORD startPosition = { 0, y };
	//获取输出句柄
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	//获取窗口缓冲中的信息
	GetConsoleScreenBufferInfo(hOutput, &sbi);
	//窗口缓冲的位置，这里取得X值
	len = sbi.dwSize.X;
	//从特定的位置用特定的字符去填充窗口的缓冲特定次数
	//成功返回非0值，一般都成功，就不判断了
	FillConsoleOutputCharacter(hOutput, fillchar, len, startPosition, &nw);
}

typedef struct _Pos {
	short x;
	short y;
}Pos, *PPos;

typedef struct _MapCoord {
	Pos location;
	int block;
	//char data[CHAR_LENGTH + 1];
}MapCoord;

typedef struct _Snake_body {
	Pos pos;
	struct _Snake_body *next;
	struct _Snake_body *prev;
}SnakeBody, *PSnakeBody;

typedef struct _Snake {
	PSnakeBody head;
	PSnakeBody tail;
	int length;
}Snake, *PSnake;

char screenBuf[WIDTH * HEIGHT];
PSnake snake = 0;
MapCoord **map = 0;
PPos bean = 0;
int score;
char str_score[4];
void initGame();
void exitGame();
int runSnakeGame();
void generateBean(PPos bean);

void writeMapDataToCoord(short x, short y, const char* data);
void writeScreenBuf(short x, short y, const char* data);
unsigned int coordToScreenBufIndex(short x, short y);
void writeScreenBufA(short x, short y, const char* data, int size);
void int2Str(int value, char* str);

int main()
{
	runSnakeGame();
	while (1);
}

void initGame() {
	//memset(screenBuf, ' ', WIDTH * HEIGHT);
	map = (MapCoord**)malloc(COLUMN * sizeof(MapCoord*));
	for (short i = 0; i < COLUMN; i++) {
		map[i] = (MapCoord*)malloc(ROW * sizeof(MapCoord));
	}
	for (short i = 0; i < COLUMN; i++) {
		for (short j = 0; j < ROW; j++) {
			map[i][j].location = { i, j };
			if (i == 0 || j == 0 || i == COLUMN - 1 || j == ROW - 1) {
				map[i][j].block = 1;
				writeMapDataToCoord(i, j, STR_WALL);
			}
			else {
				map[i][j].block = 0;
				writeMapDataToCoord(i, j, STR_EMPTY);
			}

		}
	}

	snake = new(Snake);
	snake->head = 0;
	snake->tail = 0;
	snake->length = 0;

	int length = 0;
	PSnakeBody curBody = new(SnakeBody);
	curBody->next = 0;
	curBody->prev = 0;
	curBody->pos = { COLUMN / 2,ROW / 2 };
	snake->head = curBody;
	snake->tail = curBody;
	map[curBody->pos.x][curBody->pos.y].block = 1;
	writeMapDataToCoord(curBody->pos.x, curBody->pos.y, STR_SNAKE);
	while (length < INITSNAKELENGTH - 1) {
		PSnakeBody body = new(SnakeBody);
		body->next = 0;
		body->prev = curBody;
		body->pos = { curBody->pos.x - 1, curBody->pos.y };
		curBody->next = body;
		snake->tail = body;
		curBody = body;
		map[curBody->pos.x][curBody->pos.y].block = 1;
		writeMapDataToCoord(curBody->pos.x, curBody->pos.y, STR_SNAKE);
		length += 1;
	}

	srand(time(NULL));
	bean = new(Pos);
	generateBean(bean);

	writeScreenBufA(85, ROW / 2, "当前得分:", 10);
	memset(str_score, 0, sizeof(str_score));
	int2Str(score, str_score);
	writeScreenBufA(85, ROW / 2 + 1, str_score, sizeof(str_score));
}

int runSnakeGame() {
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD ec;
	//const SMALL_RECT rect = { 0,0,100,300 };
	//if (!SetConsoleWindowInfo(hOutput, TRUE, &rect)) {
	//	printf("SetConsoleWindowInfo Error:%d", GetLastError());
	//	return 0;
	//}

	//COORD size = { 80,200 };
	//if (!SetConsoleScreenBufferSize(hOutput, size)) {
	//	printf("SetConsoleWindowInfo Error:%d", GetLastError());
	//	return 0;
	//}
	//CONSOLE_SCREEN_BUFFER_INFO csbi;
	//GetConsoleScreenBufferInfo(hOutput, &csbi);

	CONSOLE_CURSOR_INFO cci;
	cci.bVisible = 0;
	cci.dwSize = 1;
	if (!SetConsoleCursorInfo(hOutput, &cci)) {
		printf("SetConsoleWindowInfo Error:%d", GetLastError());
		return 0;
	}
	initGame();
	DWORD nw;
	int count = 0;
	COORD coord = { 0,0 };
	clock_t now = clock();
	clock_t speed = 50; 
	clock_t speedDt = 0;
	int perFrame = (int)(1 / FPS * 1000);
	clock_t dt;
	DWORD wn;
	short direction = 2;
	short snakeDir = direction;
	int pause = 0;
	while (1)
	{
		dt = clock() - now;
		if (dt >= perFrame) {
			if (_kbhit())
			{
				int key = _getch();
				if (key == 'P' || key == 'p') {
					pause = !pause;
				}
				else if (snakeDir == direction && !pause)
				{
					if (key == 'A' || key == 'a')
					{
						if (direction != 2)
							direction = 4;
					}
					else if (key == 'S' || key == 's')
					{
						if (direction != 1)
							direction = 3;
					}
					else if (key == 'D' || key == 'd') {
						if (direction != 4)
							direction = 2;
					}
					else if (key == 'W' || key == 'w') {
						if (direction != 3)
							direction = 1;
					}
				}
			}
			/*if (GetAsyncKeyState('P')) {
			pause = !pause;
			}*/
			if (!pause) {
				/*if (snakeDir == direction) {
				if (GetAsyncKeyState(VK_LEFT)) {
				if (direction != 2)
				direction = 4;
				}
				else if (GetAsyncKeyState(VK_DOWN)) {
				if (direction != 1)
				direction = 3;
				}
				else if (GetAsyncKeyState(VK_RIGHT)) {
				if (direction != 4)
				direction = 2;
				}
				else if (GetKeyState(VK_UP)) {
				if (direction != 3)
				direction = 1;
				}
				}*/

				if (speedDt >= speed) {
					PSnakeBody head = snake->head;
					PSnakeBody tail = snake->tail;
					Pos nextPos;
					snakeDir = direction;
					if (snakeDir == 1) {
						nextPos.x = head->pos.x;
						nextPos.y = head->pos.y - 1;
					}
					else if (snakeDir == 2) {
						nextPos.x = head->pos.x + 1;
						nextPos.y = head->pos.y;
					}
					else if (snakeDir == 3) {
						nextPos.x = head->pos.x;
						nextPos.y = head->pos.y + 1;
					}
					else {
						nextPos.x = head->pos.x - 1;
						nextPos.y = head->pos.y;
					}
					if (nextPos.x == bean->x && nextPos.y == bean->y) {
						generateBean(bean);
						PSnakeBody newHead = new(SnakeBody);
						newHead->pos = nextPos;
						newHead->next = head;
						head->prev = newHead;
						newHead->prev = NULL;
						snake->head = newHead;
						writeMapDataToCoord(newHead->pos.x, newHead->pos.y, STR_SNAKE);
						map[newHead->pos.x][newHead->pos.y].block = 1;
						if (speed > 10)
							speed -= 2;
						score += 1;
						//memset(str_score, 0, sizeof(str_score));
						int2Str(score, str_score);
						writeScreenBufA(85, ROW / 2 + 1, str_score, sizeof(str_score));
					}
					else if (map[nextPos.x][nextPos.y].block) {
						printf("x:%d y:%d", nextPos.x, nextPos.y);
						break;
					}
					else {
						tail->next = head;
						head->prev = tail;
						snake->head = tail;
						snake->tail = tail->prev;
						snake->tail->next = NULL;
						tail->prev = NULL;
						writeMapDataToCoord(tail->pos.x, tail->pos.y, STR_EMPTY);
						map[tail->pos.x][tail->pos.y].block = 0;
						tail->pos = nextPos;
						map[tail->pos.x][tail->pos.y].block = 1;
						writeMapDataToCoord(tail->pos.x, tail->pos.y, STR_SNAKE);

					}


					speedDt = 0;
				}
				speedDt += dt;
			}
			//FillConsoleOutputCharacterA(hOutput, ' ', WIDTH * HEIGHT, coord, &wn);
			WriteConsoleOutputCharacterA(hOutput, screenBuf, WIDTH * HEIGHT, coord, &wn);
			now = clock();

		}
	}
	exitGame();
	return 0;
}

void generateBean(PPos bean) {
	while (1) {
		short rx = (rand() % (COLUMN - 1)) + 1;
		short ry = (rand() % (ROW - 1)) + 1;
		if (!map[rx][ry].block) {
			bean->x = rx;
			bean->y = ry;
			break;
		}
	}
	writeMapDataToCoord(bean->x, bean->y, STR_BEAN);
}

void exitGame() {
	if (snake != 0) {
		PSnakeBody body = snake->head;
		while (body != NULL) {
			PSnakeBody tmp = body;
			body = body->next;
			free(tmp);
		}
		free(snake);
	}

	if (map != 0) {
		for (short r = 0; r < ROW; r++)
			free(map[r]);
		free(map);
	}

	if (bean != 0) {
		free(bean);
	}
}

void writeMapDataToCoord(short x, short y, const char* data) {
	writeScreenBuf(x * CHAR_LENGTH, y, data);
}

void writeScreenBuf(short x, short y, const char* data) {
	writeScreenBufA(x, y, data, CHAR_LENGTH);
}

void writeScreenBufA(short x, short y, const char* data, int size) {
	unsigned int index = coordToScreenBufIndex(x, y);
	memcpy(&screenBuf[index], data, size);
}

unsigned int coordToScreenBufIndex(short x, short y) {
	return x + y * WIDTH;
}

void int2Str(int value, char* str) {
	if (str == NULL)
		return;
	char buf[11];
	int len = 0;
	int temp = value;
	do {
		buf[len] = (temp % 10) + '0';
		len += 1;
		temp = temp / 10;
	} while (temp);

	int index = 0;
	while (len) {
		str[index] = buf[len - 1];
		index += 1;
		len -= 1;
	}
}