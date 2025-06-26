#include <d3d11.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include <SimpleMath.h>
#include <Audio.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <stdexcept>

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

struct Model2D
{
    ID3D11ShaderResourceView* texture = NULL;
    int x = 0, y = 0;
    int move_x = 0, move_y = 0;
    int frame = 0;              // the current frame index
    int frame_total = 0;        // how many frames totally in the frame sheet
    int frame_column = 0;       // how many columns in the frame sheet
    int frame_width = 0;        // one frame width
    int frame_height = 0;       // one frame height
    float scale = 1.0f;
};

// global declarations
extern const int Width;
extern const int Height;
extern bool gameover;

extern ID3D11Device* dev;                     // the pointer to our Direct3D device interface
extern ID3D11DeviceContext* devcon;           // the pointer to our Direct3D device context
extern IDXGISwapChain* swapchain;             // the pointer to the swap chain interface
extern ID3D11RenderTargetView* renderview;    // the pointer to the render target view
extern std::unique_ptr<SpriteBatch> spriteBatch;  // the pointer to the sprite batch


//Function Prototypes
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

bool Game_Init(HWND hwnd);
void Game_Run();
void Game_End();

bool InitD3D(HWND hWnd);    // sets up and initializes Direct3D
void CleanD3D();        // closes Direct3D and releases memory
void ClearScreen();

// load sprite sheet into a model
// frame_total is how many frames in the frame sheet
// frame_column is how many columns in the frame sheet
Model2D CreateModel2D(LPCWSTR filename, int frame_total = 1, int frame_column = 1);
void DrawModel2D(Model2D model, RECT game_window = RECT());
bool CheckModel2DCollided(Model2D model1, Model2D model2);

extern std::unique_ptr<Keyboard> keyboard;
extern std::unique_ptr<Mouse> mouse;
bool InitInput(HWND hwnd);

bool InitSound();
std::unique_ptr<SoundEffect> LoadSound(LPCWSTR filename);

