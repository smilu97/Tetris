#include <hge.h>
#include <hgefont.h>
#include <time.h>
#include <Windows.h>

#include "FBStruct.h"

HGE *hge = 0;
hgeFont* fnt = 0;
hgeQuad blockQuad;
hgeQuad sblockQuad;

#define MAP_WIDTH 10
#define MAP_HEIGHT 20
#define PRINTMAP_WIDTH 12
#define PRINTMAP_HEIGHT 22
#define DEFAULT_X ((MAP_WIDTH)/2)
#define DEFAULT_Y 0
#define SCORE_UNIT 10
#define BLOCK_SIZE (40.0f)

inline void LoadResources();
inline void DeleteResources();
inline int factorial(int x)
{
	int ret = 1;
	int xplus1 = x + 1;
	for (int j = 2; j != xplus1; ++j)
	{
		ret *= j;
	}
	return ret;
}
inline int myrand(int s, int d)
{
	srand((unsigned int)time(NULL));
	return (rand() % (d + 1)) + s;
}

float timer = 0.0f;
bool gaming = false;
bool gb_renderline = true;

class _Tetris
{
public:
	bool map[MAP_WIDTH][MAP_HEIGHT];
	bool premap[MAP_WIDTH][MAP_HEIGHT];
	int score;
	int speed;
	bool IsGameOver;
public:
	void copymaptopremap() // map[][] 의 정보를 printedmap[][] 에 반영(복사)
	{
		for (int i = 0; i != MAP_WIDTH; ++i)
		for (int j = 0; j != MAP_HEIGHT; ++j)
			premap[i][j] = map[i][j];
	}
	void PrintMap();
	inline void CleanMap()
	{
		memset(map, 0, MAP_HEIGHT*MAP_WIDTH);
	}
	inline void CleanPremap()
	{
		memset(premap, 0, MAP_HEIGHT*MAP_WIDTH);
	}
	_Tetris()
	{
		CleanMap();
		// 맵 테두리 채우기
		CleanPremap();
		// 초기화
		score = 0;
		speed = 1000;
		IsGameOver = false;
	}
};

class _FBI : public _Tetris
{
public:
	int x;
	int y;
	int kind;
	int rot;
	int serial;
public:
	void Addrot()
	{
		if (rot != 3) ++rot;
		else rot = 0;
	}
	void Subrot()
	{
		if (rot != 0) --rot;
		else rot = 3;
	}
	void Set(bool _set)
	{
		char *fbsinfo;
		for (int i = 0; i != 4; ++i)
		{
			fbsinfo = fbs[kind][(4 * rot) + i];
			map[x + fbsinfo[0]][y + fbsinfo[1]] = _set;
		}
	}
	bool IsAppearOK()
	{
		char *fbsinfo;
		int tx, ty;
		for (int i = 0; i != 4; ++i)
		{
			fbsinfo = fbs[kind][(4 * rot) + i];
			tx = x + fbsinfo[0];
			ty = y + fbsinfo[1];
l2:
			if (ty < 0)
			{
				++y;
				++ty;
				goto l2;
			}
			if (tx < 0 || MAP_WIDTH - 1 < tx || ty < 0 || MAP_HEIGHT - 1 < ty) return false;
			if (map[tx][ty]) return false;
		}
		return true;
	}
	bool Move(int _x, int _y)
	{
		bool wasOK = true;
		Set(0);
		x += _x;
		y += _y;
		if (!IsAppearOK())
		{
			x -= _x;
			y -= _y;
			wasOK = false;
		}
		Set(1);
		return wasOK;
	}
	void Rotate(bool IsClockwise)
	{
		int prerot = rot;
		Set(0);
		if (IsClockwise) Addrot();
		else Subrot();
		if (!IsAppearOK()) rot = prerot;
		Set(1);
	}
	void NewFB()
	{
		x = DEFAULT_X;
		y = DEFAULT_Y;
		kind = myrand(0, FB_KIND - 1);
		rot = 0;
		if (IsAppearOK())
		{
			Set(true);
		}
		else
		{
			IsGameOver = true;
		}
	}
	void Scoring()
	{
		int NumToDown = 0;
		for (int i = MAP_HEIGHT - 1; i != -1; --i)
		{
			for (int j = 0; j != MAP_WIDTH; ++j)
			{
				map[j][i + NumToDown] = map[j][i];
			}
			bool isFull = true;
			for (int j = 0; j != MAP_WIDTH; ++j)
			{
				if (!map[j][i])
				{
					isFull = false;
					break;
				}
			}
			if (isFull)
			{
				++NumToDown;
			}
		}
		if (NumToDown)
		{
			++serial;
			score += factorial(NumToDown) * SCORE_UNIT * serial;
		}
		else
		{
			serial = 0;
		}
	}
	_FBI()
	{
		serial = 0;
	}
};_FBI fbi;

inline void setquadcoor(float _x, float _y, float width, float height, hgeQuad &quad);
inline void setquadtcoor(float _x, float _y, float width, float height, hgeQuad &quad);
inline void printquad(float _x, float _y, float width, float height, hgeQuad &quad);

void _Tetris::PrintMap()
{
	for (int i = 0; i != MAP_WIDTH; ++i)
	{
		for (int j = 0; j != MAP_HEIGHT; ++j)
		{
			if (map[i][j]) printquad((i + 1) * BLOCK_SIZE, (j + 1)*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, blockQuad);
		}
	}
}

inline void setquadcoor(float _x, float _y, float width, float height, hgeQuad &quad)
{
	quad.v[0].x = _x;
	quad.v[1].x = _x + width;
	quad.v[2].x = _x + width;
	quad.v[3].x = _x;
	quad.v[0].y = _y;
	quad.v[1].y = _y;
	quad.v[2].y = _y + height;
	quad.v[3].y = _y + height;
}

inline void setquadtcoor(float _x, float _y, float width, float height, hgeQuad &quad)
{
	quad.v[0].tx = _x;
	quad.v[1].tx = _x + width;
	quad.v[2].tx = _x + width;
	quad.v[3].tx = _x;
	quad.v[0].ty = _y;
	quad.v[1].ty = _y;
	quad.v[2].ty = _y + height;
	quad.v[3].ty = _y + height;
}

inline void printquad(float _x, float _y, float width, float height, hgeQuad &quad)
{
	setquadcoor(_x, _y, width, height, quad);
	hge->Gfx_RenderQuad(&quad);
}

bool FrameFunc()
{
	static bool keys_left;
	static bool keys_right;
	static bool keys_down;
	static bool keys_up;
	static bool keys_space;
	static bool keys_z;
	static bool keys_l;

	float dt = hge->Timer_GetDelta();
	if (!gaming)   {
		if (hge->Input_GetKeyState(HGEK_S))
		{
			gaming = true;
			fbi.NewFB();
		}
		else return false;
	}
	if (fbi.IsGameOver){
		if (hge->Input_GetKeyState(HGEK_S))
		{
			fbi.IsGameOver = false;
			fbi.CleanMap();
			fbi.NewFB();
		}
		else return false;
	}
	timer += dt;
	if (hge->Input_GetKeyState(HGEK_LEFT)){
		if (!keys_left) {
			fbi.Move(-1, 0);  keys_left = true;
		}
	}
	else keys_left = false;
	if (hge->Input_GetKeyState(HGEK_RIGHT)){
		if (!keys_right) {
			fbi.Move(1, 0);  keys_right = true;
		}
	}
	else keys_right = false;
	if (hge->Input_GetKeyState(HGEK_DOWN)){
		if (!keys_down) {
			fbi.Move(0, 1);  keys_down = true;
		}
	}
	else keys_down = false;
	if (hge->Input_GetKeyState(HGEK_UP)){
		if (!keys_up)	{
			fbi.Rotate(true);	keys_up = true;
		}
	}
	else keys_up = false;
	if (hge->Input_GetKeyState(HGEK_SPACE)){
		if (!keys_space) { 
			while (fbi.Move(0, 1));
			fbi.Scoring(); fbi.NewFB(); 
			timer = 0.0f;
			keys_space = true; 
		}
	}
	else keys_space = false;
	if (hge->Input_GetKeyState(HGEK_Z)){ if (!keys_z)	{ fbi.Rotate(false); keys_z = true; } }
	else keys_z = false;

	if (hge->Input_GetKeyState(HGEK_L)) {
		if (!keys_l) { gb_renderline = !gb_renderline; keys_l = true; }
	}
	else keys_l = false;

	if (timer > 1.0f)
	{
		timer -= 1.0f;
		if (!fbi.Move(0, 1)) {
			fbi.Scoring();
			fbi.NewFB();
			timer = 0.0f;
		}
	}

	return false;
}

bool RenderFunc()
{
	hge->Gfx_BeginScene();
	hge->Gfx_Clear(0);

	float tmp1;

	for (int i = 0; i != MAP_WIDTH + 2; ++i)  // 테두리 위아래
	{
		tmp1 = i * BLOCK_SIZE;
		printquad(tmp1, 0.0f, BLOCK_SIZE, BLOCK_SIZE, sblockQuad);
		printquad(tmp1, (MAP_HEIGHT + 1) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, sblockQuad);
	}
	for (int i = 0; i != MAP_HEIGHT; ++i)	 //테두리 왼쪽 위
	{
		tmp1 = (i + 1) * BLOCK_SIZE;
		printquad(0.0f, tmp1, BLOCK_SIZE, BLOCK_SIZE, sblockQuad);
		printquad((MAP_WIDTH + 1) *BLOCK_SIZE, tmp1, BLOCK_SIZE, BLOCK_SIZE, sblockQuad);
	}

	if (!gaming) fnt->printf(0.0f, 0.0f, HGETEXT_LEFT, "Press S to start");
	if (fbi.IsGameOver) fnt->printf(0.0f, 0.0f, HGETEXT_LEFT, "GameOver\nPress S to restart");
	fnt->printf(0.0f, (MAP_HEIGHT + 1) * BLOCK_SIZE, HGETEXT_LEFT, "Score : %d", fbi.score);

	fbi.PrintMap();	  // 블럭 프린트
	if (gb_renderline)
	{
		for (int i = 2; i != MAP_WIDTH + 1; ++i)   // 세로선
		{
			tmp1 = i * BLOCK_SIZE;
			hge->Gfx_RenderLine(tmp1, BLOCK_SIZE, tmp1, (MAP_HEIGHT + 1) * BLOCK_SIZE, 0xFFFF0000);
		}
		for (int i = 2; i != MAP_HEIGHT + 1; ++i)  // 가로선
		{													
			tmp1 = i * BLOCK_SIZE;
			hge->Gfx_RenderLine(BLOCK_SIZE, tmp1, (MAP_WIDTH + 1) * BLOCK_SIZE, tmp1, 0xFFFF0000);
		}
	}

	hge->Gfx_EndScene();
	return false;
}

inline void LoadResources()
{
	// blockQuad
	blockQuad.tex = hge->Texture_Load("block.png");
	setquadtcoor(0.0f, 0.0f, 1.0f, 1.0f, blockQuad);
	blockQuad.blend = BLEND_DEFAULT;
	for (int i = 0; i != 4; ++i)
	{
		blockQuad.v[i].col = 0xFFFFFFFF;
		blockQuad.v[i].z = 0.5f;
	}
	// sblockQuad
	sblockQuad.tex = hge->Texture_Load("sblock.png");
	setquadtcoor(0.0f, 0.0f, 1.0f, 1.0f, sblockQuad);
	sblockQuad.blend = BLEND_DEFAULT;
	for (int i = 0; i != 4; ++i)
	{
		sblockQuad.v[i].col = 0xFFFFFFFF;
		sblockQuad.v[i].z = 0.5f;
	}
	// fnt
	fnt = new hgeFont("font1.fnt");
}

inline void DeleteResources()
{
	hge->Texture_Free(blockQuad.tex);
	hge->Texture_Free(sblockQuad.tex);
	delete fnt;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	hge = hgeCreate(HGE_VERSION);

	hge->System_SetState(HGE_LOGFILE, "Hotris.log");
	hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
	hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
	hge->System_SetState(HGE_TITLE, "Hotris");
	hge->System_SetState(HGE_FPS, 100);
	hge->System_SetState(HGE_WINDOWED, true);
	hge->System_SetState(HGE_SCREENWIDTH, (MAP_WIDTH + 2) * BLOCK_SIZE);
	hge->System_SetState(HGE_SCREENHEIGHT, (MAP_HEIGHT + 2) * BLOCK_SIZE);
	hge->System_SetState(HGE_SCREENBPP, 32);

	hge->System_SetState(HGE_SHOWSPLASH, false); // Starting splash delete

	if (hge->System_Initiate()) {
		LoadResources();
		if (!blockQuad.tex | !sblockQuad.tex) MessageBox(0, "Resource Load Error", "Error", MB_OK); // Resource Loading Check

		hge->System_Start(); // HGE Routine Start

		DeleteResources();
	}
	// Clean up and shut down
	hge->System_Shutdown();
	hge->Release();

	return 0;
}