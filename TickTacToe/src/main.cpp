//-----------------------------------------------------------------------------
// Simple TicTacToe demo built on ohmFlux
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include <fluxMain.h>
#include <fonts/fluxBitmapFont.h>

#include <array>
#include <render/fluxRender2D.h>

class TicTacToeGame : public FluxMain
{
    typedef FluxMain Parent;
private:
    enum Player : int { None = 0, PlayerX = 1, PlayerO = 2 };

    FluxTexture* mFontTex = nullptr;
    FluxTexture* mBackgroundTex = nullptr;
    FluxBitmapFont* mCellLabels[9] = {0};
    FluxBitmapFont* mStatusLabel = nullptr;
    FluxBitmapFont* mHintLabel = nullptr;
    // FluxBitmapFont* mGridLabels[4] = {0}; // simple text-based grid lines

    std::array<Player, 9> mBoard;
    Player mCurrentPlayer = PlayerX;
    bool mGameOver = false;
    int mMoveCount = 0;

    // Board layout
    int mCellSize = 150;
    int mBoardSize = 450;
    int mBoardLeft = 0;
    int mBoardTop = 0;

public:
    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // Use a monospace font from local assets.
        mFontTex = loadTexture("assets/fonts/monoSpace_13x28.bmp", 10, 10, false);
        if (!mFontTex)
            return false;

        mBackgroundTex = loadTexture("assets/background.bmp", 1, 1, false);

        mBoardSize = mCellSize * 3;
        mBoardLeft = (int)(getScreen()->getCenterX() - (mBoardSize / 2));
        mBoardTop  = (int)(getScreen()->getCenterY() - (mBoardSize / 2));

        // Create cell labels
        int idx = 0;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                FluxBitmapFont* lbl = new FluxBitmapFont(mFontTex);
                lbl->setCharSize(64, 64);
                int x = mBoardLeft + col * mCellSize + (mCellSize / 2) - 20;
                int y = mBoardTop + row * mCellSize + (mCellSize / 2) - 32;
                lbl->set(" ", x, y, 64, 64);
                lbl->setLayer(0.1f);
                queueObject(lbl);
                mCellLabels[idx++] = lbl;
            }
        }

        // Grid lines (text-based)

        mStatusLabel = new FluxBitmapFont(mFontTex);
        //mStatusLabel->set("Player X turn", 10, 10, 26, 32, 0.9f, 0.9f, 1.f, 1.f);
        mStatusLabel->set("Player X turn", getScreen()->getCenterX(), mBoardTop - 50, 26, 32, { 0.9f, 0.9f, 1.f, 1.f} );
        mStatusLabel->setAlign(FontAlign_Center);
        mStatusLabel->setLayer(0.05f);
        queueObject(mStatusLabel);

        mHintLabel = new FluxBitmapFont(mFontTex);
        mHintLabel->set("Click cells to play. Press R to reset.", getScreen()->getCenterX() , mBoardTop + mBoardSize + 30,
                        18, 24, { 0.8f, 0.8f, 0.8f, 1.f } );
        mHintLabel->setAlign(FontAlign_Center);
        mHintLabel->setLayer(0.05f);
        queueObject(mHintLabel);

        resetBoard();
        return true;
    }

    void Deinitialize() override
    {
        Parent::Deinitialize();
    }

    void resetBoard()
    {
        mBoard.fill(Player::None);
        mCurrentPlayer = PlayerX;
        mGameOver = false;
        mMoveCount = 0;
        updateCellLabels();
        mStatusLabel->setCaption("Player X turn");
    }


    void updateCellLabels()
    {
        for (int i = 0; i < 9; i++) {
            const char* text = " ";
            float r = 0.8f, g = 0.8f, b = 0.8f;
            Color4F lColor = { 0.8f, 0.8f, 0.8f, 1.f};
            if (mBoard[i] == PlayerX) {
                text = "X";
                lColor.r = 0.1f; lColor.g = 0.7f; lColor.b = 1.f;
            } else if (mBoard[i] == PlayerO) {
                text = "O";
                lColor.r = 1.f; lColor.g = 0.4f; lColor.b = 0.2f;
            }
            mCellLabels[i]->set(text, mCellLabels[i]->getX(), mCellLabels[i]->getY(),
                                 64, 64, lColor);
        }
    }

    Player checkWinner() const
    {
        const int wins[8][3] = {
            {0,1,2}, {3,4,5}, {6,7,8}, // rows
            {0,3,6}, {1,4,7}, {2,5,8}, // cols
            {0,4,8}, {2,4,6}           // diags
        };
        for (auto& w : wins) {
            Player a = mBoard[w[0]];
            if (a != None && a == mBoard[w[1]] && a == mBoard[w[2]])
                return a;
        }
        return None;
    }

    int findLineMove(Player target) const
    {
        const int wins[8][3] = {
            {0,1,2}, {3,4,5}, {6,7,8},
            {0,3,6}, {1,4,7}, {2,5,8},
            {0,4,8}, {2,4,6}
        };
        for (auto& w : wins) {
            int a = w[0], b = w[1], c = w[2];
            Player pa = mBoard[a], pb = mBoard[b], pc = mBoard[c];
            int emptyIdx = -1;
            int countTarget = 0;
            if (pa == None) emptyIdx = a; else if (pa == target) countTarget++;
            if (pb == None) emptyIdx = b; else if (pb == target) countTarget++;
            if (pc == None) emptyIdx = c; else if (pc == target) countTarget++;
            if (countTarget == 2 && emptyIdx != -1)
                return emptyIdx;
        }
        return -1;
    }

    void performAIMove()
    {
        if (mGameOver || mCurrentPlayer != PlayerO)
            return;

        // 1) Win if possible
        int move = findLineMove(PlayerO);
        // 2) Block opponent
        if (move == -1)
            move = findLineMove(PlayerX);
        // 3) Take center
        if (move == -1 && mBoard[4] == None)
            move = 4;
        // 4) Take a corner
        if (move == -1) {
            const int corners[4] = {0,2,6,8};
            for (int c : corners) {
                if (mBoard[c] == None) { move = c; break; }
            }
        }
        // 5) Take a side
        if (move == -1) {
            const int sides[4] = {1,3,5,7};
            for (int s : sides) {
                if (mBoard[s] == None) { move = s; break; }
            }
        }
        // 6) Fallback
        if (move == -1) {
            for (int i = 0; i < 9; i++) {
                if (mBoard[i] == None) { move = i; break; }
            }
        }

        if (move != -1)
            placeMove(move);
    }

    void placeMove(int cell)
    {
        if (mGameOver || cell < 0 || cell >= 9)
            return;
        if (mBoard[cell] != None)
            return;

        mBoard[cell] = mCurrentPlayer;
        mMoveCount++;
        updateCellLabels();

        Player winner = checkWinner();
        if (winner != None) {
            mStatusLabel->setCaption(winner == PlayerX ? "Player X wins!" : "Player O wins!");
            mGameOver = true;
            return;
        }
        if (mMoveCount >= 9) {
            mStatusLabel->setCaption("Draw! Press R to reset.");
            mGameOver = true;
            return;
        }

        mCurrentPlayer = (mCurrentPlayer == PlayerX) ? PlayerO : PlayerX;
        mStatusLabel->setCaption(mCurrentPlayer == PlayerX ? "Player X turn" : "Player O turn");


        // AI plays as Player O.
        if (!mGameOver && mCurrentPlayer == PlayerO)
            performAIMove();
    }

    void handleClick(int mouseX, int mouseY)
    {
        if (mGameOver || mCurrentPlayer != PlayerX)
            return;
        if (mouseX < mBoardLeft || mouseX >= mBoardLeft + mBoardSize
            || mouseY < mBoardTop || mouseY >= mBoardTop + mBoardSize)
            return;

        int col = (mouseX - mBoardLeft) / mCellSize;
        int row = (mouseY - mBoardTop) / mCellSize;
        int cell = row * 3 + col;
        placeMove(cell);
    }

    void onKeyEvent(SDL_KeyboardEvent event) override
    {
        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        SDL_Keymod mods = event.mod;
        SDL_Keycode key = event.key;
        const SDL_Keycode KEY_R = SDLK_R;

        (void)mods;

        if (!isKeyUp)
            return;

        switch (key) {
            case SDLK_ESCAPE:
                TerminateApplication();
                break;
            case KEY_R:
                resetBoard();
                break;
            default:
                break;
        }
    }

    void onMouseButtonEvent(SDL_MouseButtonEvent event) override
    {
        bool isDown = event.down;
        Uint8 button = event.button;
        int mx = (int)event.x;
        int my = (int)event.y;
        if (!isDown)
            return;
        if (button != SDL_BUTTON_LEFT)
            return;

        // Convert to logical coords using screen scale
        float lX = (float)mx / getScreen()->getScaleX();
        float lY = (float)my / getScreen()->getScaleY();

        handleClick((int)lX, (int)lY);
    }

    void onDraw() override
    {
        // Log("DEBUG screen: center: %d,%d size: %dx%d", getScreen()->getCenterX(),
        //     getScreen()->getCenterY(), getScreen()->getWidth(), getScreen()->getHeight());


        if (mBackgroundTex && getScreen()) {
            //FIXME uglyDraw2DStretch
            Render2D.uglyDraw2DStretch(
                mBackgroundTex, 0,
                getScreen()->getCenterX(),
                getScreen()->getCenterY(),
                0.95f,
                getScreen()->getWidth(),
                getScreen()->getHeight(),
                0.f, false, false, 0.0f, false);
        }

    }

};

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    TicTacToeGame* game = new TicTacToeGame();
    game->mSettings.Caption = "TicTacToe";
    game->mSettings.Version = "0.2";
    game->mSettings.ScreenWidth = 900;
    game->mSettings.ScreenHeight = 600;
    game->mSettings.ScaleScreen = true;
    game->mSettings.initialVsync = true;
    game->mSettings.minWindowSize = { 300, 200 };
    game->Execute();
    SAFE_DELETE(game);
    return 0;
}
