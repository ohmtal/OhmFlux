//-----------------------------------------------------------------------------
// ohmFlux TestBed
// !!! Warning ugly code style ahead !!!
//-----------------------------------------------------------------------------
#include "source/fluxMain.h"
#include "source/fluxBitmapFont.h"
#include "source/misc.h"
#include <fluxRender2D.h>
#include <fluxInput.h>
#include <fluxTilemap.h>
#include <fluxTextFile.h>
#include <fluxParticleEmitter.h>
#include <fluxParticleManager.h>
#include <fluxParticlePresets.h>
#include <fluxTrueTypeFont.h>
#include <fluxAudioStream.h>
#include <fluxScheduler.h>
#include <fluxLight.h>
#include <fluxLightManager.h>

class TestBed : public FluxMain
{
    typedef FluxMain Parent;
private:
    FluxInput mInput;

    FluxTexture* mFontTex = nullptr;
    FluxTexture* mBackgroundTex = nullptr;
    FluxBitmapFont* mStatusLabel = nullptr;

    // FluxTexture* mBasicGroundTiles = nullptr;

    FluxTilemap* mTileMap = nullptr;


    FluxParticleEmitter* mFireEmitter;
    FluxParticleEmitter* mSparkEmitter;

    FluxTrueTypeFont* mMonoFont;

    FluxAudioStream* mTomsGuitarSample = nullptr;
    FluxAudioStream* mClickSound = nullptr;
    FluxAudioStream* mBrrooiiSound = nullptr;

    FluxScheduler::TaskID mScheduleTestId = 0;


public:
    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // Use a monospace font from local assets.
        // mFontTex = loadTexture("assets/fonts/monoSpace_13x28.bmp", 10, 10, false, false);
        mFontTex = loadTexture("assets/fonts/source_code_32x60.bmp",10,10,true, true);
        if (!mFontTex)
            return false;

        mBackgroundTex = loadTexture("assets/background.bmp", 1, 1, false);


        // setup Tilemap >>>
        // fixme nicer image :P with pixel perfect on it makes tile tearing,
        // the changed getTextureRectById
        // mBasicGroundTiles = loadTexture("assets/tiles/basicGround8x1.bmp", 8, 1, false, true);
        //
        S32 squareSize = 64;
        S32 mapSize =  squareSize * squareSize;
        RectI area = { 0,0, mapSize, mapSize };


        //FluxTilemap(RectI area, F32 squareSize, U32 maxlayers,  F32 renderZ = 10.f)
        mTileMap = new FluxTilemap(area, static_cast<F32>(squareSize), 2);
        mTileMap->setAtlas( loadTexture("assets/tiles/TilesetGrass/overworld_tileset_grass.png", 12, 21, false, true ) );
        mTileMap->fillLayer(0,2);
        mTileMap->loadLayerFromText("assets/maps/ohmtal_1.map", 1, ' ' );
        queueObject(mTileMap);
        // <<< Tilemap


        mStatusLabel = new FluxBitmapFont(mFontTex, getScreen());
        //mStatusLabel->set("Player X turn", 10, 10, 26, 32, 0.9f, 0.9f, 1.f, 1.f);
        mStatusLabel->set("...STATUS....", getScreen()->getCenterX(), 50, 26, 32, { 0.9f, 0.9f, 1.f, 1.f} );
        mStatusLabel->setAlign(FontAlign_Center);
        mStatusLabel->setLayer(0.05f);
        mStatusLabel->setColor( cl_Blue );
        // mStatusLabel->setIsGuiElement(true); //should be default true
        queueObject(mStatusLabel);


        // --- Input Mapping Setup ---

        // Movement (Arrows)
        mInput.bindKey("MoveLeft",  SDL_SCANCODE_LEFT);
        mInput.bindKey("MoveRight", SDL_SCANCODE_RIGHT);
        mInput.bindKey("MoveUp",    SDL_SCANCODE_UP);
        mInput.bindKey("MoveDown",  SDL_SCANCODE_DOWN);

        // Zoom (Page keys and +/-)
        // Note: SDL_SCANCODE_EQUALS is the key usually physically associated with '+'
        mInput.bindKey("ZoomIn",    SDL_SCANCODE_PAGEUP);
        mInput.bindKey("ZoomOut",    SDL_SCANCODE_PAGEDOWN);

        mInput.bindKey("ZoomOut",   SDL_SCANCODE_PAGEDOWN);
        mInput.bindKey("ZoomOut",   SDL_SCANCODE_MINUS);

        mInput.bindKey("ResetCam",   SDL_SCANCODE_SPACE);

        // Mouse bindings
        // mInput.bindMouse("ZoomIn", SDL_BUTTON_LEFT); // Zoom in on Left Click
        // mInput.bindMouse("ZoomOut", SDL_BUTTON_RIGHT); // Move up on Right Click

        // emitter test >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        // Example: Creating a Fire Emitter
        EmitterProperties lFireProps;

        // Position and Spawn Rate
        lFireProps.position = { 400.0f, 300.0f };
        lFireProps.maxParticles = 500;
        lFireProps.spawnRate = 100.0f; // 100 particles per second

        // Life and Movement (Angles in Radians)
        lFireProps.minLifetime = 0.5f;
        lFireProps.maxLifetime = 1.5f;
        lFireProps.minAngle = -FLUX_PI / 2.0f - 0.2f; // Upwards with slight spread
        lFireProps.maxAngle = -FLUX_PI / 2.0f + 0.2f;
        lFireProps.minSpeed = 50.0f;
        lFireProps.maxSpeed = 150.0f;

        // Visuals
        lFireProps.minScale = 0.5f;
        lFireProps.maxScale = 2.f;
        lFireProps.texture = loadTexture("assets/particles/Star-Texture.png");

        // Color Gradient: Bright Orange to Faded Red
        lFireProps.startColorMin = Color4F::FromHex(0xFFCC00FF); // Bright Orange
        lFireProps.startColorMax = Color4F::FromHex(0xFFEE00FF); // Bright Yellow
        lFireProps.endColorMin   = Color4F::FromHex(0xFF000000); // Black/Faded Red
        lFireProps.endColorMax   = Color4F::FromHex(0x55000000);
        // -----

         // mFireEmitter = new FluxParticleEmitter(lFireProps);
         mFireEmitter = ParticleManager.addEmitter(lFireProps);

         // SnowFlake1
         // init in one line !
         mSparkEmitter = ParticleManager.addEmitter(
                    ParticlePresets::sparkPreset
                        .setTexture(loadTexture( "assets/particles/SnowFlake1.png" ))
                        // .setScaleMinMax( 0.01f, 0.1f)
         );

        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        // FluxTrueTypeFont

         mMonoFont = new FluxTrueTypeFont("assets/fonts/JetBrainsMono/JetBrainsMono-Medium.ttf", 32);
         if (mMonoFont)
         {
             mMonoFont->set("Alder Babsack", { 200.f,200.f }, cl_Crimson, 2.f);
             // mMonoFont->setPos(200,200);
             // mMonoFont->setCaption("Alder Babsack!");
             queueObject(mMonoFont);
         }

         // FluxAudioStream (ogg)
         mTomsGuitarSample  = new FluxAudioStream("assets/music/sample1_loop.ogg");
         queueObject(mTomsGuitarSample);
         mTomsGuitarSample->setLooping(true);
         mTomsGuitarSample->setPositon({ 0.f, 0.f });
         mTomsGuitarSample->play();

         mClickSound = new FluxAudioStream("assets/sounds/pling.ogg");
         queueObject(mClickSound);

         queueObject(mClickSound); //<< fail safe test


         mBrrooiiSound = new FluxAudioStream("assets/sounds/brrooii.ogg");
         mBrrooiiSound->setPositon( getScreen()->getCenterF());
         queueObject(mBrrooiiSound);

         mScheduleTestId = FluxSchedule.add(3.0, [this]() {
                          this->Brrooii(); // Calls method with arguments
         });
         Log("ScheduleTestId is: %d", mScheduleTestId);

        // lights testing:

         // Render2D.setAmbientColor({ 1.f,0.5f,0.5f,0.5f });
          Render2D.setAmbientColor({ 0.01f, 0.01f, 0.01f, 1.f });
          LightManager.addLight(FluxLight({100.0f, 400.0f, 0.0f}, cl_Red , 100.0f));
          LightManager.addLight(FluxLight({250.0f, 400.0f, 0.0f}, cl_Green, 100.0f));
          LightManager.addLight(FluxLight({175.0f, 450.0f, 10.0f}, { 0.f,0.f,1.f,3.f}, 100.0f));

         // FluxLight flashlight({ 700.0f, 0.0f,0.0f}, cl_White, 500.f);
         // flashlight.setAsSpotlight({0.0f, 1.0f}, 45.0f); //
         // LightManager.addLight(flashlight);
         LightManager.addLight(
             FluxLight({ 700.0f, 0.0f,0.0f}, cl_White, 500.f)
             .setAsSpotlight({0.0f, 1.0f}, 45.0f)
         );
         LightManager.addLight(
             FluxLight({ 900.0f, 0.0f,0.0f}, cl_White, 500.f)
             .setAsSpotlight({0.0f, 1.0f}, 45.0f)
         );
         // LightManager.addLight(
         //     FluxLight({ 1100.0f, 0.0f,0.0f}, cl_White, 500.f)
         //     .setAsSpotlight({0.0f, 1.0f}, 45.0f)
         // );
         // LightManager.addLight(
         //     FluxLight({ 1300.0f, 0.0f,0.0f}, cl_White, 500.f)
         //     .setAsSpotlight({0.0f, 1.0f}, 45.0f)
         // );
         // LightManager.addLight(
         //     FluxLight({ 1500.0f, 0.0f,0.0f}, cl_White, 500.f)
         //     .setAsSpotlight({0.0f, 1.0f}, 45.0f)
         // );
         return true;
    }

    void Brrooii()
    {
        mBrrooiiSound->play();
        mScheduleTestId = FluxSchedule.add(3.0, [this]() {
            this->Brrooii(); // Calls method with arguments
        });
        Log("ScheduleTestId is: %d", mScheduleTestId);

    }

    void Deinitialize() override
    {
        Parent::Deinitialize();
    }

    void onKeyEvent(SDL_KeyboardEvent event) override
    {


        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        SDL_Keymod mods = event.mod;
        SDL_Keycode key = event.key;


        (void)mods;

        if (!isKeyUp)
            return;

        switch (key) {
            case SDLK_ESCAPE:
                TerminateApplication();
                break;

            // case SDLK_PAGEUP:
            // case SDLK_PLUS:  Render2D.getCamera()->moveZoom(0.01f);      break;
            //
            // case SDLK_PAGEDOWN:
            // case SDLK_MINUS: Render2D.getCamera()->moveZoom(-0.01f);     break;

            default:
                break;
        }
    }

    void onMouseButtonEvent(SDL_MouseButtonEvent event) override
    {
        bool isDown = event.down;
        Uint8 button = event.button;

        if (!isDown)
            return;
        switch ( button) {
            case SDL_BUTTON_LEFT:
                mSparkEmitter->setPosition({ getStatus().getWorldMousePos().x, getStatus().getWorldMousePos().y, 0.f});
                mSparkEmitter->play();
                mClickSound->play();
        }


    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                // Zoom speed is usually much higher for the wheel
                float scrollZoom = event.wheel.y *  0.001f  * getFrameTime();
                Render2D.getCamera()->moveZoom(scrollZoom);
            }

    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {

        const float camSpeed  = 0.1f * getFrameTime();
        const float zoomSpeed = 0.0001f  * getFrameTime();


        // Handle Camera Movement
        if (mInput.isActionActive("MoveLeft"))  Render2D.getCamera()->move({-camSpeed, 0.0f});
        if (mInput.isActionActive("MoveRight")) Render2D.getCamera()->move({ camSpeed, 0.0f});
        if (mInput.isActionActive("MoveUp"))    Render2D.getCamera()->move({ 0.0f, -camSpeed});
        if (mInput.isActionActive("MoveDown"))  Render2D.getCamera()->move({ 0.0f, camSpeed});

        // Handle Camera Zooming
        if (mInput.isActionActive("ZoomIn"))    Render2D.getCamera()->moveZoom(zoomSpeed);
        if (mInput.isActionActive("ZoomOut"))   Render2D.getCamera()->moveZoom(-zoomSpeed);

        // ResetCam
         if (mInput.isActionActive("ResetCam"))  {
             Render2D.getCamera()->setZoom(1.f);
             Render2D.getCamera()->setPosition( { (F32) getScreen()->getCenterX(), (F32) getScreen()->getCenterY() } );
        };

        // debug text:
        mStatusLabel->setCaption("cam pos:%4.1f,%4.1f zoom:%4.2f"
            , Render2D.getCamera()->getPosition().x, Render2D.getCamera()->getPosition().y
            , Render2D.getCamera()->getZoom()
        );


        // emitter test
        // FIXME emitter need a manager!
        mFireEmitter->getProperties().position = { getStatus().getWorldMousePos().x, getStatus().getWorldMousePos().y, 0.f};
        // mFireEmitter->update(dt / 1000.f);

        Parent::Update(dt);
    }

    void onDraw() override
    {

        // if (mBackgroundTex)
        // {
        //     //FIXME uglyDraw2DStretch
        //     Render2D.uglyDraw2DStretch(
        //         mBackgroundTex, 0,
        //         getScreen()->getCenterX(),
        //         getScreen()->getCenterY(),
        //         0.95f,
        //         getScreen()->getWidth(),
        //         getScreen()->getHeight(),
        //         0.f, false, false, 0.0f, false);
        // }
        //

        for (S32 i = 20 ; i < 100; i+=10 )
            Render2D.drawCircle(getScreen()->getCenterX(),getScreen()->getCenterY(),i);

        Render2D.drawRect(100,100,200,200, { 0.3f, 0.3f,0.3f,0.7f});
        Render2D.drawRect(200,200,300,300, { 1.f, 0.f,0.f,1.f}, false);
        Render2D.drawLine(0,0,100,100, { 1.f, 0.3f,0.3f,1.f});

        // drawTriangle(Point3F p1, Point3F p2, Point3F p3, const Color4F& color, bool filled = true);
        Render2D.drawTriangle( { 600,0 }, { 650, 20 }, { 400, 200 }, cl_Blue, false);

        DrawParams2D dp;
        dp.image = mTileMap->getImage();
        dp.imgId = 0;
        dp.x = -600;
        dp.y = 120;
        dp.z =  0.f;
        dp.w = 256.f;
        dp.h = 256.f;

        Render2D.drawSprite(dp);
        for (S32 imgId = 1; imgId < 8; imgId ++)
        {
            dp.y += dp.h;
            dp.imgId = imgId;
            Render2D.drawSprite(dp);
        }

        // FIXME emitter need a manager!
        // mFireEmitter->render();
    } //Draw


};

int main(int argc, char** argv)
{
    (void)argc; (void)argv;
    TestBed* game = new TestBed();
    game->mSettings.Caption = "TestBed";
    game->mSettings.enableLogFile = true;
    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


