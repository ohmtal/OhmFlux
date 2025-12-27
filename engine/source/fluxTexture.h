//-----------------------------------------------------------------------------
// Copyright (c) 2012 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Flux Game Engine
//
// Author: T.Huehn (XXTH)
// Desc  : Texture Class
//-----------------------------------------------------------------------------
#pragma once
#ifndef _FLUXTEXTURE_H_
#define _FLUXTEXTURE_H_

#include "fluxGL.h"
#include "fluxGlobals.h"
#include <SDL3/SDL.h>

#include <vector>

class FluxTexture
{
private:
	GLuint  mHandle;
	const char* mFileName;
	bool mLoaded;
	bool mUseAnisotropy;
	bool mUseTrilinearFiltering;
	int mW = 0;
	int mH = 0;
	int mRows;
	int mCols;
	Point2F mTexSize = { 0.f, 0.f }; //size of Texture or part if we have rows and or cols
	std::vector<Point2F> mTexturePosition;

	// for atlas generation
	std::vector<std::string> mPendingFiles;

protected:
	void setSize( const int& lW, const int& lH );

public:
	FluxTexture();
	~FluxTexture();
	SDL_Surface* loadWithSTB(const char* filename);
	bool loadTextureDirect(const char* filename); //using STB
	bool loadTexture(const char* filename, bool setColorKeyAtZeroPixel = false);
	void bindOpenGLAlphaDirect(unsigned char* pixels, int w, int h); //for TruetypeFonts
	void bindOpenGLDirect(unsigned char* pixels, int w, int h); //from loadTextureDirect using STB
	void bindOpenGL(SDL_Surface* lSurface);
	GLuint getHandle() { return mHandle; }

    void setParts(const int& cols, const int& rows);
    int getCols() { return ( mCols ); }
    int getRows() { return ( mRows ); }
	int getWidth() const       { if (mLoaded) return(mW); else return -1; }
    int getHeight() const      { if (mLoaded) return(mH); else return -1; }


    bool getTextureRectById( Uint32 lImgId, Point2F& position, Point2F& size );
	void setManual(GLuint handle, int w, int h);

	void setUseTrilinearFiltering();

	// Atlas generation
	void addToAtlas(const std::string& filename);
	void generateAtlas(int maxRows, bool setColorKeyAtZeroPixel, bool usePixelPerfect);

	// save a texture to PNG
	bool savePNGToFile(const char* filename);

};
#endif //_FLUXTEXTURE_H_
