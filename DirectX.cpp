#include "DirectX.h"

ID3D11Device* dev;                     
ID3D11DeviceContext* devcon;           
IDXGISwapChain* swapchain;             
ID3D11RenderTargetView* renderview;    
std::unique_ptr<SpriteBatch> spriteBatch;
std::unique_ptr<AudioEngine> audEngine;
std::unique_ptr<Keyboard> keyboard;
std::unique_ptr<Mouse> mouse;

bool InitD3D(HWND hWnd)
{
    HRESULT hr;

    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    // fill the swap chain description struct
    scd.BufferCount = 1;                                    // one back buffer in addition with the front buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
    scd.BufferDesc.Width = Width;                           // set the back buffer width
    scd.BufferDesc.Height = Height;                         // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.OutputWindow = hWnd;                                // the window to be used
    scd.SampleDesc.Count = 4;                               // how many multisamples
    scd.Windowed = TRUE;                                    // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0; // Use d3d11
    UINT              numLevelsRequested = 1; // Number of levels 
    D3D_FEATURE_LEVEL FeatureLevelsSupported;

    hr = D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        &FeatureLevelsRequested,
        numLevelsRequested,
        D3D11_SDK_VERSION,
        &scd,
        &swapchain,
        &dev,
        &FeatureLevelsSupported,
        &devcon);
    if (FAILED(hr))
        return false;

    // create a render target view
    ID3D11Texture2D* backbuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
    hr = dev->CreateRenderTargetView(backbuffer, nullptr, &renderview);
    if (FAILED(hr))
        return false;

    D3D11_VIEWPORT vp;
    vp.Width = Width;
    vp.Height = Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    devcon->RSSetViewports(1, &vp);

    spriteBatch.reset(new SpriteBatch(devcon));

    // bind render target view
    devcon->OMSetRenderTargets(1, &renderview, nullptr);

    return true;
}

void CleanD3D()
{
    // switch with Alt-Enter between full screen and windowed mode
    // switch to windowed mode when closing game
    swapchain->SetFullscreenState(FALSE, NULL);

    // close and release all existing resources
    mouse.release();
    keyboard.release();
    audEngine.release();
    spriteBatch.release();
    renderview->Release();
    swapchain->Release();
    devcon->Release();
    dev->Release();
}

void ClearScreen()
{
    float background_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    devcon->ClearRenderTargetView(renderview, background_color);
}


Model2D CreateModel2D(LPCWSTR filename, int frame_total, int frame_column)
{
    Model2D model;
    model.x = 0;
    model.y = 0;
    model.move_x = 0;
    model.move_y = 0;
    model.frame_total = frame_total;
    model.frame_column = frame_column;

    CreateWICTextureFromFile(dev, filename, NULL, &model.texture);
    if (model.texture != NULL)
    {
        ID3D11Resource* resource;
        model.texture->GetResource(&resource);
        ID3D11Texture2D* tex2D;
        resource->QueryInterface<ID3D11Texture2D>(&tex2D);
        D3D11_TEXTURE2D_DESC desc;
        tex2D->GetDesc(&desc);

        int row_number = ceil(float(frame_total) / float(frame_column));
        model.frame_width = int(desc.Width) / frame_column;
        model.frame_height = int(desc.Height) / row_number;
    }

    return model;
}

void DrawModel2D(Model2D model, RECT game_window)
{
    if (model.texture != NULL)
    {
        model.frame = model.frame % model.frame_total;

        RECT rect;
        rect.left = (model.frame % model.frame_column) * model.frame_width;
        rect.right = rect.left + model.frame_width;
        rect.top = (model.frame / model.frame_column) * model.frame_height;
        rect.bottom = rect.top + model.frame_height;

        XMFLOAT2 pos;
        pos.x = model.x - game_window.left;
        pos.y = model.y - game_window.top;
        //spriteBatch->Draw(model.texture, pos, &rect);

        XMFLOAT2 origin = XMFLOAT2(0.0f, 0.0f);  // top-left origin
        float rotation = 0.0f;
        float depth = 0.0f;

        spriteBatch->Draw(
            model.texture,
            pos,
            &rect,
            DirectX::Colors::White,
            rotation,
            origin,
            model.scale, // adds scaling
            DirectX::SpriteEffects_None,
            depth
        );
    }
}

bool CheckModel2DCollided(Model2D model1, Model2D model2)
{
    //adjusted collision hitbox of bird
    RECT rect1;
    rect1.left = model1.x + 25;
    rect1.top = model1.y + 20;
    rect1.right = model1.x + (int)(model1.frame_width * model1.scale) - 15; // scale frame
    rect1.bottom = model1.y + (int)(model1.frame_height * model1.scale) - 20;

    RECT rect2;
    rect2.left = model2.x + 1;
    rect2.top = model2.y + 1;
    rect2.right = model2.x + (int)(model2.frame_width * model2.scale) - 1;
    rect2.bottom = model2.y + (int)(model2.frame_height * model2.scale) - 1;

    RECT dest;
    return IntersectRect(&dest, &rect1, &rect2);  // return rect1 if rect2 has intersection 
}                                                                           


bool InitInput(HWND hwnd)
{
    try {
        keyboard = std::make_unique<Keyboard>();
        mouse = std::make_unique<Mouse>();
        mouse->SetWindow(hwnd);
        //mouse->SetVisible(false);
        return true;
    }
    catch (std::runtime_error e)
    {
        return false;
    }
}


bool InitSound()
{
    try {
        // init sound
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default | AudioEngine_Debug;
        audEngine = std::make_unique<AudioEngine>(eflags);
        return true;
    }
    catch (std::runtime_error e)
    {
        return false;
    }

}


std::unique_ptr<SoundEffect> LoadSound(LPCWSTR filename)
{
    try {
        return std::make_unique<SoundEffect>(audEngine.get(), filename);
    }
    catch (std::runtime_error e)
    {
        return NULL;
    }
    
}

