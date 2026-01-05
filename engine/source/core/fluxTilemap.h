//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//FIXME add a cpp when done

#pragma once
#include <sstream>

#include "core/fluxGlobals.h"
#include "fluxBaseObject.h"
#include "core/fluxTexture.h"
#include "render/fluxRender2D.h"
#include "utils/fluxFile.h"
#include "grid/basicgrid.h"



class FluxTilemap : public FluxBaseObject {
private:
    BasicGrid* mGrid;          // Logic/Pathfinding
    FluxTexture* mAtlas;      // Visuals

    U32 mMaxLayers = 1;

    F32 mRenderLayer = 10.f; //background
    F32 mSquareSize;
    F32 mHalfSquareSize;
public:
    FluxTilemap(RectI area, F32 squareSize, U32 maxlayers,  F32 renderZ = 10.f)
    {
        mGrid = new BasicGrid();
        mGrid->init(area, squareSize);
        mAtlas = nullptr;
        mRenderLayer = renderZ; //z - sorting
        mSquareSize = squareSize;
        mHalfSquareSize = squareSize / 2.f;
        setMaxLayers(maxlayers, true);

    }
    //--------------------------------------------------------------------------
    ~FluxTilemap() {
        SAFE_DELETE(mGrid);
    }
    //--------------------------------------------------------------------------
    void setAtlas ( FluxTexture* lAtlas ) {
        mAtlas = lAtlas;
    }
    //--------------------------------------------------------------------------
    void setImage ( FluxTexture* lAtlas ) {
        mAtlas = lAtlas;
    }
    //--------------------------------------------------------------------------
    FluxTexture* getImage ( ) {
        return mAtlas;
    }
    //--------------------------------------------------------------------------
    bool fillLayer(U32 lLayer, S32 lDefaultValue = -99 )
    {

        // dLog("fillLayer( %d , %d )", lLayer, lDefaultValue);

        if (!mGrid || mGrid->getNodeCount() < 1 || lLayer >  BASIC_GRID_NODE_INTVALUES_COUNT)
            return false;

        for (U32 i = 0; i < mGrid->getNodeCount(); i++ )
        {
            mGrid->getNodeById(i)->setIntValue(lLayer,lDefaultValue);
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool setMaxLayers( U32 value , bool doInitNodes = false)
    {
        if ( value > BASIC_GRID_NODE_INTVALUES_COUNT )
        {
            Log("Tilemap::setMaxLayers failed to set Maxlayers to %d exceed %d!", value, BASIC_GRID_NODE_INTVALUES_COUNT);
            return false;
        }

        mMaxLayers = value;

        if ( doInitNodes ){
            for ( S32 i = 0; i < mMaxLayers; i++ )
                fillLayer(i);
        }

        return true;
    }
    //--------------------------------------------------------------------------
    bool setTile(S32 x, S32 y, S32 lLayer, S32 ltileID)
    {
        if ( lLayer >= mMaxLayers )
        {
            dLog("Tilemap::setTile layer > mMaxLayers! adjusting mMaxLayers to %d!", lLayer+1);
            if (!setMaxLayers( lLayer + 1 ))
                return false;
            else
                fillLayer(lLayer);
        }


        S32 idx = mGrid->getNodeIndex(x, y);
        BasicGridNode* node = mGrid->getNodeById(idx);

        if (node)
        {
            node->setIntValue(lLayer, ltileID);
        }
        return true;
    }
    //--------------------------------------------------------------------------
    // Delegate important logic to the grid
    S32 getNodeAt(F32 x, F32 y) {
        return mGrid->getNodeIndex(x, y);
    }
    //--------------------------------------------------------------------------
    void Draw() override
    {
        if (!mAtlas)
            return;

        RectF view = Render2D.getCamera()->getVisibleWorldRect();
        std::vector<S32> visibleIndices;

        mGrid->getNodesByRect(view, visibleIndices, true);

        DrawParams2D dp;
        dp.image = mAtlas;
        dp.w     = mSquareSize;
        dp.h     = mSquareSize;

        F32 lRenderLayer = mRenderLayer ;

        for (S32 layer = 0; layer < mMaxLayers; layer++)
        {
            lRenderLayer =  mRenderLayer - (static_cast<F32>(layer) * 0.1f);
            for (S32 idx : visibleIndices)
            {
                // 3. Get node for position
                BasicGridNode* node = mGrid->getNodeById(static_cast<U32>(idx));
                if (node)
                {
                    S32 tileId = node->getIntValue(layer);


                    if ( tileId >= 0 )
                    {
                        Point2F worldPos = node->getPos2F() + mHalfSquareSize;
                        dp.x = worldPos.x;
                        dp.y = worldPos.y;
                        dp.z = lRenderLayer;
                        dp.imgId = tileId;
                        Render2D.drawSprite(dp);
                    }
                }
            } //for idx
        } //for layer
    }
    //--------------------------------------------------------------------------
    bool loadLayerFromText(const std::string& filename, U32 lLayer, char lSeparator = ',')
    {
        // 1. Use your helper to get all lines from the file
        std::vector<std::string> lines;

        if (!FluxFile::LoadTextFile(filename, lines)) {
            Log("Tilemap::load failed to open file: %s", filename.c_str());
            return false;
        }

        // 2. Ensure the layer is ready in our grid and initialized with -1

        if ( lLayer >= mMaxLayers )
        {
            dLog("Tilemap::setTile layer > mMaxLayers! adjusting mMaxLayers to %d!", lLayer+1);
            if (!setMaxLayers( lLayer + 1 ))
                return false;
        }
        fillLayer(lLayer);

        // 3. Parse the data row by row
        S32 currentRow = 0;
        S32 totalTilesLoaded = 0;

        for (const std::string& line : lines) {
            // Safety: Don't read more rows than the grid has height
            if (currentRow >= mGrid->getNodesY()) break;

            std::stringstream ss(line);
            std::string value;
            S32 currentCol = 0;

            // 4. Split by separator (comma, space, etc.)
            while (std::getline(ss, value, lSeparator) && currentCol < mGrid->getNodesX()) {

                // 2025 Fix: Skip empty strings (prevents multiple spaces from shifting columns)
                if (value.empty()) continue;

                try {
                    // Convert string to integer (handles -1 correctly)
                    S32 tileID = std::stoi(value);


                    // Calculate the absolute index directly: X + (Y * Width)
                    S32 idx = currentCol + (currentRow * mGrid->getNodesX());

                    // Fetch the node directly by its ID
                    BasicGridNode* node = mGrid->getNodeById(static_cast<U32>(idx));

                    if (node) {
                        node->setIntValue(lLayer, tileID);
                        totalTilesLoaded++;
                    }

                    // // Calculate the absolute index directly: X + (Y * Width)
                    // S32 idx = currentCol + (currentRow * mGrid->getNodesX());
                    // // Fetch the node directly by its ID
                    // BasicGridNode* node = mGrid->getNodeById(static_cast<U32>(idx));
                    // // Safe method to set data in the grid
                    // if (setTile(currentCol, currentRow, lLayer, tileID)) {
                    //     totalTilesLoaded++;
                    // }

                    // Increment column ONLY after a valid tile is processed
                    currentCol++;
                } catch (...) {
                    // Skip values that aren't numbers (like text or weird characters)
                    continue;
                }
            }

            // Warning if the text row didn't fill the grid width
            if (currentCol < mGrid->getNodesX()) {
                Log("loadLayerFromText Warning: Row %d in %s is shorter (%d) than grid width (%d).",
                    currentRow, filename.c_str(), currentCol, mGrid->getNodesX());
            }

            currentRow++;
        }

        // Warning if the file didn't have enough lines for the grid height
        if (currentRow < mGrid->getNodesY()) {
            Log("loadLayerFromText Warning: %s has fewer rows (%d) than the grid height (%d).",
                filename.c_str(), currentRow, mGrid->getNodesY());
        }

        Log("Tilemap: Loaded %d tiles into %d rows from %s into layer %d",
            totalTilesLoaded, currentRow, filename.c_str(), lLayer);

        return true;
    }
    //--------------------------------------------------------------------------
    bool saveLayerToText(const char* filename, U32 layer , char lSeparator=',')
    {
        if (layer >= mMaxLayers) return false;

        std::vector<std::string> output;

        for (S32 y = 0; y < mGrid->getNodesY(); y++) {
            std::stringstream ss;
            for (S32 x = 0; x < mGrid->getNodesX(); x++) {
                S32 idx = mGrid->getNodeIndex(x, y);
                BasicGridNode* node = mGrid->getNodeById(idx);

                S32 tileID = node ? node->getIntValue(layer) : -1;

                ss << tileID;
                if (x < mGrid->getNodesX() - 1) ss << lSeparator; // Comma separated
            }
            output.push_back(ss.str());
        }

        return FluxFile::SaveTextFile(filename, output);
    }
}; //Class
