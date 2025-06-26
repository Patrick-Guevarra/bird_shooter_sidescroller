#include "DirectX.h"
#include <SpriteFont.h>
#include <SpriteBatch.h>
std::unique_ptr<DirectX::SpriteFont> spriteFont;

Keyboard::KeyboardStateTracker keyboardtracker;
Mouse::ButtonStateTracker mousetracker;

const int Width = 800; const int Height = 369;
const float framerate = 16; bool gameover;
int world_width = 1600, world_height = 369;

Model2D background;
RECT game_window;
int scroll_speed = 5;
float scrollX = 0;

Model2D bird;
float bird_speed = 14;
const int bird_width = 60;
const int bird_height = 55;
int bird_lives = 3;

const int enemy_max = 5; 
Model2D enemy[enemy_max];
bool enemy_valid[enemy_max];
float enemy_speed = 6;
int enemy_numbers = 1; //increment after every round
int enemy_cooling[enemy_max];
int enemy_cooldown = 3000;           // starting cooldown 
const int min_enemy_cooldown = 500;  // never go below 300
const int cooldown_decrement = 500; 
const int enemy_bullets_numbers = 100; // You can define a maximum number of enemy bullets
Model2D enemy_bullet[enemy_bullets_numbers];
bool enemy_bullet_valid[enemy_bullets_numbers];
int last_enemy_bullet = -1;  // The last fired enemy bullet index
float enemy_bullet_speed = 5;  // Initial speed for enemy bullets

Model2D   boss;
bool boss_active = false;
int boss_hp = 0;
int boss_last_shot = 0;
int boss_cooldown = 3000;    // fires every 2 seconds
float boss_bullet_speed = 10.0f;  
float boss_speed = 4;

Model2D bossDeath;
bool bossDeath_valid = false;

const  int  bullets_numbers = 100; 
Model2D bullet[bullets_numbers];
bool bullet_valid[bullets_numbers];
int last_bullet = -1;
float bullet_speed = 40;
const int cooldown = 800;  
long cooling = 0;

int game_round = 1; //increment after all current enemies are dead
bool round_in_progress = true;  // Tracks if mid round
int score = 0;

Model2D enemyDeath[enemy_max];
bool enemyDeath_valid[enemy_max];

std::unique_ptr<SoundEffect> enemyBullet;
std::unique_ptr<SoundEffect> playerBullet;
std::unique_ptr<SoundEffect> hitmarker;
std::unique_ptr<SoundEffect> enemyHit;
std::unique_ptr<SoundEffect> gameoverMusic;
std::unique_ptr<SoundEffect> newRound;

void LoadBird()
{
    bird = CreateModel2D(L"bird.png", 6, 3);
    if (bird.texture == NULL)
    {
        MessageBox(NULL, L"Loading bird.png error",
            L"Error", MB_OK | MB_ICONERROR);
    }
    bird.scale = 0.4f;

}

void UpdateBird()
{
    auto kb = keyboard->GetState();
    if (keyboard->GetState().Escape)
        gameover = true;

    bird.frame++;
    bird.move_x = 0;
    bird.move_y = 0;

    if (kb.Up || kb.W)
    {
        bird.move_y = -bird_speed;
    }
    if (kb.Down || kb.S)
    {
        bird.move_y = bird_speed;
    }
    
    if (kb.Left || kb.A)
    {
        bird.move_x = -bird_speed;
    }
    if (kb.Right || kb.D)
    {
        bird.move_x = bird_speed;
    }
    bird.x += bird.move_x;
    bird.y += bird.move_y;
   
    if (bird.x < 0)
        bird.x = 0; //Prevent bird from moving off the left edge
    if (bird.x > (Width) - bird_width)  
        bird.x = (Width) - bird_width;  

    if (bird.y < 0)
        bird.y = 0;  //Prevent bird from moving off the top edge
    if (bird.y > world_height - bird_height)
        bird.y = world_height - bird_height;  //Prevent bird from moving off the bottom edge

}

void LoadEnemies()
{
    for (int i = 0; i < enemy_numbers; i++)
    {
        enemy[i] = CreateModel2D(L"enemy.png", 5, 5);
        if (enemy[i].texture == NULL)
        {
            MessageBox(NULL, L"Loading enemy.png error",
                L"Error", MB_OK | MB_ICONERROR);
        }
        enemy[i].scale = 1.2f;
        enemy_valid[i] = true;

        //initial position of enemies
        enemy[i].x = Width - enemy[i].frame_width - (rand() % 300);
        enemy[i].y = rand() % (Height - enemy[i].frame_height);
        //enemy moves up and down
        if (rand() % 2 == 0)
            enemy[i].move_y = 6.0f;
        else
            enemy[i].move_y = -6.0f;
    }
}

void UpdateEnemies()
{
    bool allEnemiesDead = true;

    for (int i = 0; i < enemy_numbers; ++i)
    {
        if (enemy_valid[i])
        {
            allEnemiesDead = false;

            enemy[i].frame++;
            enemy[i].y += enemy[i].move_y;

            // Bounce off top/bottom edges
            if (enemy[i].y < 0)
            {
                enemy[i].y = 0;
                enemy[i].move_y = enemy_speed;
            }
            else if (enemy[i].y > Height - enemy[i].frame_height)
            {
                enemy[i].y = Height - enemy[i].frame_height;
                enemy[i].move_y = -enemy_speed;
            }
            // Reverse direction on collision with other enemies
            for (int j = i + 1; j < enemy_numbers; ++j)
            {
                if (enemy_valid[j] && CheckModel2DCollided(enemy[i], enemy[j]))
                {
                    enemy[i].move_y *= -1;
                    enemy[j].move_y *= -1;
                }
            }
            // Check for hits by player bullets
            for (int j = 0; j < bullets_numbers; ++j)
            {
                if (bullet_valid[j] && CheckModel2DCollided(enemy[i], bullet[j]))
                {
                    // Destroy the bullet
                    bullet[j].move_x = 0;
                    bullet[j].move_y = 0;
                    bullet_valid[j] = false;
                    // Kill the enemy
                    enemy_valid[i] = false;
                    score += 10;
                    // Trigger its death animation
                    enemyDeath_valid[i] = true;
                    enemyDeath[i].x = enemy[i].x;
                    enemyDeath[i].y = enemy[i].y;
                    enemyDeath[i].frame = 0;
                    // Play hit sound
                    enemyHit->Play();
                }
            }
        }
    }

    if (allEnemiesDead && round_in_progress && !boss_active)
    {
        newRound->Play();
        game_round++;

        if (game_round % 5 == 0)
        {
            // spawn boss every 5 rounds
            boss_active = true;
            boss_hp = 10;
            boss_last_shot = GetTickCount();

            // Reset the next regular enemy wave to 1 and increase difficulty
            enemy_numbers = 1;
            enemy_bullet_speed += 8;    // faster bullets
            enemy_cooldown = max(min_enemy_cooldown, enemy_cooldown - cooldown_decrement);

            // position boss
            boss.x = Width - boss.frame_width * boss.scale - 50.0f;
            boss.y = float(rand() % (Height - int(boss.frame_height * boss.scale)));
            boss.move_y = (rand() % 2 == 0) ? boss_speed : -boss_speed;

            //increase boss difficulty
            boss_cooldown = max(500, boss_cooldown - 500);
            boss_bullet_speed += 3.0f;
        }
        else
        {
            if (enemy_numbers < enemy_max)
                enemy_numbers++;
        }

        round_in_progress = false;
    }
}

void LoadEnemyBullets()
{
    for (int i = 0; i < enemy_bullets_numbers; i++)
    {
        enemy_bullet[i] = CreateModel2D(L"fireball.png", 6, 6);
        if (enemy_bullet[i].texture == NULL)
        {
            MessageBox(NULL, L"Loading fireball.png error",
                L"Error", MB_OK | MB_ICONERROR);
        }
        enemy_bullet[i].scale = 0.3f;
        enemy_bullet[i].move_x = 0;
        enemy_bullet[i].move_y = 0;
        enemy_bullet_valid[i] = false;
    }
}

void UpdateEnemyBullets()
{
    for (int i = 0; i < enemy_numbers; i++)
    {
        for (int i = 0; i < enemy_bullets_numbers; i++)
        {
            enemy_bullet[i].frame++;
        }
        if (enemy_valid[i])
        {
            // Enemy shoots automatically after some cooldown
            if (GetTickCount() - enemy_cooling[i] > enemy_cooldown)
            {
                // Enemy bullet fires
                enemy_cooling[i] = GetTickCount();  // Reset cooldown
                enemyBullet->Play();
                last_enemy_bullet++;  // A new bullet is fired

                if (last_enemy_bullet >= enemy_bullets_numbers)
                    last_enemy_bullet = 0;

                //Set the bullet's position based on the enemy's position
                enemy_bullet[last_enemy_bullet].x = enemy[i].x + (enemy[i].frame_width*enemy[i].scale) / 2
                    - enemy_bullet[last_enemy_bullet].frame_width / 2 + 50.0f; //+50.0f is y offset to center where bullet fires
                enemy_bullet[last_enemy_bullet].y = enemy[i].y + (enemy[i].frame_height*enemy[i].scale) / 2
                    - enemy_bullet[last_enemy_bullet].frame_height / 2 + 60.0f; //+60.0f is y offset to center where bullet fires

                enemy_bullet_valid[last_enemy_bullet] = true;

                enemy_bullet[last_enemy_bullet].move_x = -enemy_bullet_speed;  // Shoot to the left
                enemy_bullet[last_enemy_bullet].move_y = 0;  // Only horizontal movement

                // Update the bullet's position
                enemy_bullet[last_enemy_bullet].x += enemy_bullet[last_enemy_bullet].move_x;
            }
        }
    }
    
    for (int i = 0; i < enemy_bullets_numbers; i++)
    {
        if (enemy_bullet_valid[i])
        {
            enemy_bullet[i].x += enemy_bullet[i].move_x;  // Move the bullet horizontally
            if (enemy_bullet[i].x < 0)  // Bullet goes offscreen
            {
                enemy_bullet_valid[i] = false;  // Mark bullet as invalid
            }
            if (enemy_bullet_valid[i] && CheckModel2DCollided(enemy_bullet[i], bird))
            {
                hitmarker->Play();
                enemy_bullet_valid[i] = false;  // Mark the bullet as invalid
                bird_lives--;                    // lose one life
                if (bird_lives <= 0)
                {
                    gameoverMusic->Play();
                    MessageBox(NULL, L"Defeated", L"Game Over", MB_OK);
                    gameover = true;
                                 
                }
            }
        }
    }
}

void LoadBullets()
{
    for (int i = 0; i < bullets_numbers; i++)
    {
        bullet[i] = CreateModel2D(L"bullet.png");
        bullet[i].move_x = 0;
        bullet[i].move_y = 0;
        bullet_valid[i] = false;
    }
}

void UpdateBullets()
{
    if (keyboard->GetState().Space && GetTickCount() - cooling > cooldown)
    {
        playerBullet->Play();
        cooling = GetTickCount();
        last_bullet++;  // a new bullet is fired

        // if has reached the end of the bullet array
        // go back to the beginning the array and restart
        if (last_bullet >= bullets_numbers)
            last_bullet = 0;

        // the new bullet is fired from the center of the tank
        bullet[last_bullet].x = bird.x + (bird.frame_width*bird.scale) / 2
            - bullet[last_bullet].frame_width / 2 - 50.0f; //-50.0f is y offset to center where bullet fires
        bullet[last_bullet].y = bird.y + (bird.frame_height*bird.scale) / 2
            - bullet[last_bullet].frame_height / 2;
        bullet_valid[last_bullet] = true;

         // Set the bullet's direction to the right
        bullet[last_bullet].move_x = bullet_speed;
        bullet[last_bullet].move_y = 0;

        // Update the bullet's position based on direction (only horizontal movement to the right)
        bullet[last_bullet].x += bullet[last_bullet].move_x;
    }
    for (int i = 0; i < bullets_numbers; i++)
    {
        if (bullet_valid[i]) 
        {
            bullet[i].x += bullet[i].move_x;  // Update the bullet's horizontal position
            
            if (bullet[i].x > world_width)  // If the bullet moves off the right side of the screen
            {
                bullet_valid[i] = false;  
            }
        }
    }
}

void LoadEnemyDeath()
{
    for (int i = 0; i < enemy_max; i++)
    {
        enemyDeath[i] = CreateModel2D(L"enemy_death.png", 8, 8);
        enemyDeath[i].scale = 1.2f;
        enemyDeath_valid[i] = false;
    }
}

void UpdateEnemyDeath()
{
    for (int i = 0; i < enemy_max; i++)
    {
        if (enemyDeath_valid[i])
            enemyDeath[i].frame++;

        if (enemyDeath[i].frame >= enemyDeath[i].frame_total)
            enemyDeath_valid[i] = false;
    }
}

void LoadBoss()
{
    boss = CreateModel2D(L"enemy.png", 5, 5);
    boss.scale = 3.0f;              
    boss_active = false;
    boss_last_shot = 0;
    boss_hp = 0;
}

void UpdateBoss()
{
    if (!boss_active) return;

    boss.frame++;
    boss.y += boss.move_y;

    // bounce top/bottom
    float bottom = boss.y + boss.frame_height * boss.scale;
    if (boss.y < 0)
    {
        boss.y = 0;
        boss.move_y = boss_speed;
    }
    else if (bottom > Height)
    {
        boss.y = Height - boss.frame_height * boss.scale;
        boss.move_y = -boss_speed;
    }

    // shooting on cooldown
    if (GetTickCount() - boss_last_shot > boss_cooldown)
    {
        boss_last_shot = GetTickCount();

        last_enemy_bullet++;
        if (last_enemy_bullet >= enemy_bullets_numbers)
            last_enemy_bullet = 0;

        enemy_bullet[last_enemy_bullet] = CreateModel2D(L"fireball.png", 6, 6);

        // position it at the center of the boss
        enemy_bullet[last_enemy_bullet].x = boss.x;
        enemy_bullet[last_enemy_bullet].y = boss.y + (boss.frame_height * boss.scale) / 2 
            - (enemy_bullet[last_enemy_bullet].frame_height * enemy_bullet[last_enemy_bullet].scale) / 2;

        // fire to the left
        enemy_bullet[last_enemy_bullet].move_x = -boss_bullet_speed;
        enemy_bullet[last_enemy_bullet].move_y = 0;

        enemy_bullet_valid[last_enemy_bullet] = true;
        enemyBullet->Play();
    }

    for (int i = 0; i < bullets_numbers; ++i)
    {
        if (bullet_valid[i] && CheckModel2DCollided(boss, bullet[i]))
        {
            bullet_valid[i] = false;
            enemyHit->Play();
            boss_hp--;
            if (boss_hp <= 0)
            {
                boss_active = false;
                bossDeath_valid = true;
                bossDeath.x = boss.x;
                bossDeath.y = boss.y;
                bossDeath.frame = 0;

                score += 50;
                bird_lives++;
                game_round++;
                newRound->Play();
                round_in_progress = false;
            }
        }
    }
}

void LoadBossDeath()
{
    bossDeath = CreateModel2D(L"enemy_death.png", 8, 8);
    bossDeath.scale = boss.scale;   
    bossDeath_valid = false;
}

void UpdateBossDeath()
{
    if (!bossDeath_valid) return;
    bossDeath.frame++;

    if (bossDeath.frame >= bossDeath.frame_total)
        bossDeath_valid = false;
}


bool Game_Init(HWND hwnd)
{
    gameover = false;
    InitD3D(hwnd);
    InitInput(hwnd);
    InitSound();

    try
    {
        spriteFont = std::make_unique<DirectX::SpriteFont>(dev, L"RoundNumber.spritefont");
    }
    catch (std::runtime_error&)
    {
        MessageBox(NULL,
            L"Failed to load RoundNumber.spritefont",
            L"Error",
            MB_OK | MB_ICONERROR);
        return false;
    }

    srand(GetTickCount());

    LoadBird();
    LoadEnemyDeath();
    LoadEnemies();
    LoadBoss();
    LoadBossDeath();
    LoadEnemyBullets();
    LoadBullets();

    background = CreateModel2D(L"game_background.bmp");
    if (background.texture == NULL)
        return false;

    background.frame_width = 1600;
    background.frame_height = 369;

    game_window.left = 0;
    game_window.top = 0;
    game_window.right = Width;  // 800
    game_window.bottom = Height; // 369
    
    playerBullet = LoadSound(L"player_bullet.wav");
    enemyBullet = LoadSound(L"enemy_bullet.wav");
    hitmarker = LoadSound(L"hitmarker.wav");
    enemyHit = LoadSound(L"enemy_hit.wav");
    gameoverMusic = LoadSound(L"gameover_music.wav");
    newRound = LoadSound(L"new_round.wav");

    return true;
}

void Game_Run()
{
    static long start = 0;
    float frame_interval = 1000.0f / framerate;

    if (keyboard->GetState().Escape)
        gameover = true;

    if (GetTickCount() - start >= frame_interval)
    {
        start = GetTickCount();
        ClearScreen();

        spriteBatch->Begin();

        // Update background scroll
        scrollX += scroll_speed;
        if (scrollX >= background.frame_width)
            scrollX = 0;

        // Draw 2 backgrounds to simulate looping scroll
        RECT srcRect;
        srcRect.top = 0;
        srcRect.bottom = background.frame_height;

        // First part
        srcRect.left = (int)scrollX;
        srcRect.right = (int)scrollX + Width;
        if (srcRect.right <= background.frame_width) {
            // Draw in one piece
            XMFLOAT2 pos(0, 0);
            spriteBatch->Draw(background.texture, pos, &srcRect);
        }
        else {
            // Draw split — right edge of image and wrap around
            int firstWidth = background.frame_width - (int)scrollX;
            RECT firstSrc = { (int)scrollX, 0, background.frame_width, background.frame_height };
            XMFLOAT2 firstPos(0, 0);
            spriteBatch->Draw(background.texture, firstPos, &firstSrc);

            RECT secondSrc = { 0, 0, Width - firstWidth, background.frame_height };
            XMFLOAT2 secondPos((float)firstWidth, 0);
            spriteBatch->Draw(background.texture, secondPos, &secondSrc);
        }

        UpdateBird();
        UpdateEnemyDeath();
        UpdateEnemies();
        // Check if round is over and reload enemies
        if (!round_in_progress)
        {
            if (!boss_active)
                LoadEnemies();
            round_in_progress = true;
        }

        UpdateEnemyBullets();
        UpdateBullets();
        UpdateBoss();
        UpdateBossDeath();
        // Draw bird relative to screen
        DrawModel2D(bird, {0, 0, Width, Height});
        for (int i = 0; i < enemy_max; i++)
            if (enemy_valid[i])
            {
                DrawModel2D(enemy[i]);
            }
        for (int i = 0; i < bullets_numbers; i++)
            if (enemy_bullet_valid[i])
            {
                DrawModel2D(enemy_bullet[i]);
            }
        for (int i = 0; i < bullets_numbers; i++)
            if (bullet_valid[i])
            {
                DrawModel2D(bullet[i]);
            }
        for (int i = 0; i < enemy_numbers; i++)
            if (enemyDeath_valid[i]) DrawModel2D(enemyDeath[i]);
        if (boss_active)
            DrawModel2D(boss);
        if (bossDeath_valid)
            DrawModel2D(bossDeath);

        {
            //Round Text
            std::wstring roundText = L"Round: " + std::to_wstring(game_round);
            spriteFont->DrawString(
                spriteBatch.get(),
                roundText.c_str(),
                XMFLOAT2(10.0f, 10.0f),
                Colors::Black);

            //Lives Text
            std::wstring lifeTxt = L"     | Lives: " + std::to_wstring(bird_lives);
            spriteFont->DrawString(spriteBatch.get(),
                lifeTxt.c_str(),
                XMFLOAT2(175.0f,  10.0f),
               Colors::Red);

            //Score Text
            std::wstring scoreTxt = L"  | Score: " + std::to_wstring(score);
            spriteFont->DrawString(spriteBatch.get(),
                scoreTxt.c_str(),
                DirectX::XMFLOAT2(85.0, 10),
                DirectX::Colors::Yellow);
        }

        spriteBatch->End();
        swapchain->Present(0, 0);
    }
}

void Game_End()
{
    background.texture->Release();
    bird.texture->Release();
    playerBullet.release();
    enemyBullet.release();
    hitmarker.release();
    gameoverMusic.release();
    newRound.release();
    enemyHit.release();
    boss.texture->Release();
    bossDeath.texture->Release();
    for (int i = 0; i < enemy_numbers; i++)
        enemy[i].texture->Release();
    for (int i = 0; i < enemy_bullets_numbers; i++)
        enemy_bullet[i].texture->Release();
    for (int i = 0; i < bullets_numbers; i++)
        bullet[i].texture->Release();
    for (int i = 0; i < enemy_max; i++)
        enemyDeath[i].texture->Release();
	CleanD3D();
}