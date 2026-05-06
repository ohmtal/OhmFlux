//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
//  FluxTexture
//-----------------------------------------------------------------------------

#include "core/fluxGlobals.h"
#include "platform/fluxGL.h"

#include <vector>
#include "core/fluxGlobals.h"
#include "core/fluxTexture.h"
#include "utils/errorlog.h"
#include "stb_image.h"
#include "stb_image_write.h"


#ifdef FLUX_GLES2
GLint internalFormat = GL_RGBA; // GLES 2.0
#else
GLint internalFormat = GL_RGBA8;
#endif

//------------------------------------------------------------------------------
//constructor
FluxTexture::FluxTexture(void) {
  mHandle = 0;
  mLoaded = false;
  mRows = 1;
  mCols = 1;

  mUseAnisotropy = true;
  mUseTrilinearFiltering = false; //looks better but have a disadvantage on pixel perfect drawing

}

//destructor
FluxTexture::~FluxTexture(void) {
	if (mLoaded) {
	   glDeleteTextures(1, &mHandle);
	   mHandle = 0;
	   mLoaded = false;
	}
}
//------------------------------------------------------------------------------
void FluxTexture::setSize( const int& lW, const int& lH )
{
  mW = lW;
  mH = lH;
  mTexSize = { (float)mW,  (float)mH };
}
//------------------------------------------------------------------------------

// STB Loader
SDL_Surface* FluxTexture::loadWithSTB(const char* filename) {
  int width, height, channels;
  size_t fileSize;

  // Read
  void* buffer = SDL_LoadFile(filename, &fileSize);
  if (!buffer) {
    SDL_Log("[error] SDL_LoadFile failed for %s: %s", filename, SDL_GetError());
    return nullptr;
  }

  // Decode via STB
  unsigned char* data = stbi_load_from_memory(
    (unsigned char*)buffer, (int)fileSize, &width, &height, &channels, 4
  );

  // Free the raw file buffer immediately
  SDL_free(buffer);

  if (!data) {
    SDL_Log("[error] STB failed to decode %s", filename);
    return nullptr;
  }

  // Create non-owning wrapper
  SDL_Surface* tempSurface = SDL_CreateSurfaceFrom(
    width, height, SDL_PIXELFORMAT_RGBA32, data, width * 4
  );

  if (!tempSurface) {
    SDL_Log("[error] SDL_CreateSurfaceFrom failed: %s", SDL_GetError());
    stbi_image_free(data);
    return nullptr;
  }

  // Create an owning copy
  // This allows us to free the 'data' pointer immediately
  SDL_Surface* finalSurface = SDL_DuplicateSurface(tempSurface);

  // Cleanup
  SDL_DestroySurface(tempSurface);
  stbi_image_free(data);

  return finalSurface;
}


//------------------------------------------------------------------------------
// Load a Texture and bind it directly
bool FluxTexture::loadTextureDirect(const char* filename)
{
  int width, height, channels;
  size_t fileSize;

  void* buffer = SDL_LoadFile(filename, &fileSize);

  if (!buffer) {
    SDL_Log("[error] FluxTexture Error: Could not load %s - %s", filename, SDL_GetError());
    return false;
  }

  unsigned char* data = stbi_load_from_memory(
    (unsigned char*)buffer,
                                              (int)fileSize,
                                              &width, &height, &channels,
                                              4 // STBI_rgb_alpha
  );

  SDL_free(buffer);

  if (!data) {
    SDL_Log("[error] STB Error: Failed to decode image %s", filename);
    return false;
  }

  mFileName = filename;
  setSize(width, height);

  bindOpenGLDirect(data, width, height);

  stbi_image_free(data);

  mLoaded = true;
  return true;
}


//------------------------------------------------------------------------------
bool FluxTexture::loadTexture(const char* filename, bool setColorKeyAtZeroPixel)
{
  SDL_Surface* lSurface = loadWithSTB(filename);

  if (!lSurface) {
    // lSurface = SDL_LoadBMP(filename); //pre SDL 3.4
    lSurface = SDL_LoadSurface(filename);
  }

  if (!lSurface) {
    SDL_Log("[error] FluxTexture Error: Failed to load %s", filename);
    return false;
  }

  mFileName = filename;
  mW = lSurface->w;
  mH = lSurface->h;

  SDL_Surface* finalSurface = nullptr;

  if (setColorKeyAtZeroPixel) {
    Uint8 r, g, b, a;
    // Safely read the top-left pixel
    if (SDL_ReadSurfacePixel(lSurface, 0, 0, &r, &g, &b, &a)) {
      // Set color key (transparency) based on that pixel
      SDL_SetSurfaceColorKey(lSurface, true, SDL_MapSurfaceRGB(lSurface, r, g, b));
    }
  }

  // Always convert to a uniform format (RGBA32) for the GPU
  // This also bakes the Color Key into the Alpha channel if set
  finalSurface = SDL_ConvertSurface(lSurface, SDL_PIXELFORMAT_RGBA32);

  if (finalSurface) {
    mLoaded = bindOpenGL(finalSurface);
    SDL_DestroySurface(finalSurface);
  } else {
    SDL_Log("[error] FluxTexture Error: Surface conversion failed for %s", filename);
    mLoaded = false;
  }

  SDL_DestroySurface(lSurface);
  return mLoaded;
}

//------------------------------------------------------------------------------
// for truetype fonts

void FluxTexture::bindOpenGLAlphaDirect(unsigned char* pixels, int w, int h)
{
  if (!pixels) return;

  // Create a temporary RGBA buffer (Emscripten needs explicit Alpha)
  std::vector<unsigned char> rgba(w * h * 4);
  for (int i = 0; i < w * h; ++i) {
    rgba[i * 4 + 0] = 255;    // R (White)
    rgba[i * 4 + 1] = 255;    // G (White)
    rgba[i * 4 + 2] = 255;    // B (White)
    rgba[i * 4 + 3] = pixels[i]; // A (Transparency from stb_truetype)
  }

  if (mHandle == 0) glGenTextures(1, &mHandle);
  glBindTexture(GL_TEXTURE_2D, mHandle);

  // Upload as RGBA (Works everywhere including WebGL)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

  // Remove the swizzleMask calls as they will cause GL errors on Emscripten/WebGL
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
//------------------------------------------------------------------------------
void FluxTexture::bindOpenGLDirect(unsigned char* pixels, int w, int h)
{

  if (!pixels) {
    Log("[error] FluxTexture::bindOpenGLDirect - Invalid Pixel Data!");
    return;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  if (mHandle == 0) glGenTextures(1, &mHandle);
  glBindTexture(GL_TEXTURE_2D, mHandle);

  // Tiling
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Filtering
  if (mUseTrilinearFiltering) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  // Upload DIRECTLY from STB
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Mipmaps
  if (mUseTrilinearFiltering) {
    glGenerateMipmap(GL_TEXTURE_2D);
    #if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    if (mUseAnisotropy) {
      GLfloat max_aniso;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (max_aniso > 8.0f) ? 8.0f : max_aniso);
    }
    #endif
  }

  if (mHandle == 0) Log("[error] Failed to bind OpenGL Texture: %s", mFileName);
}
//------------------------------------------------------------------------------
bool FluxTexture::bindOpenGL(SDL_Surface* lSurface)
{
  if (!lSurface) {
    Log("[error] FluxTexture::bindOpenGL - Invalid Surface!");
    return false;
  }

  //  Convert to a standard format for OpenGL
  SDL_Surface* formattedSurface = SDL_ConvertSurface(lSurface, SDL_PIXELFORMAT_RGBA32);

  if (!formattedSurface) {
    Log("[error] Failed to convert surface for OpenGL upload");
    return false;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glGenTextures(1, &mHandle);
  glBindTexture(GL_TEXTURE_2D, mHandle);

  //  Base Tiling
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //  Conditional Filtering & Mipmaps
  if (mUseTrilinearFiltering)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  else
  {
    // Sharp "Pixel-Perfect" mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  //  Upload Texture Data
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
               formattedSurface->w, formattedSurface->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);

  //  Generate Mipmaps ONLY if using a Mipmap Filter
  if (mUseTrilinearFiltering)
  {
    glGenerateMipmap(GL_TEXTURE_2D);

    #if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    // Anisotropy ONLY makes sense if we have mipmaps
    if (mUseAnisotropy) {
      GLfloat max_aniso;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
      // Clamp to a reasonable 8.0f to avoid over-sampling performance hits
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (max_aniso > 8.0f) ? 8.0f : max_aniso);
    }
    #endif
  }

  //  Memory Cleanup
  SDL_DestroySurface(formattedSurface);

  if (mHandle == 0) {
    Log("[error] Failed to bind OpenGL Texture: %s", mFileName);
    return false;
  }
  return true;
}


//------------------------------------------------------------------------------
void FluxTexture::setParts(const int& cols, const int& rows) {

   if (cols <1 || rows < 1 ) return;
   mCols = cols;
   mRows = rows;

   // update mTexSize
   mTexSize =  { 1 / (float) mCols , 1 / (float) mRows };

   // clear the list
   std::vector<Point2F>().swap(mTexturePosition);

   // fill the list
   int maxCells =  mCols * mRows;
   int lImgId = 0;
   Point2F position = { 0.f, 0.f };

   for ( lImgId = 0;  lImgId < maxCells ; lImgId++ )
   {

     int aColIdx = lImgId  % mCols;
     int aRowIdx = lImgId  / mCols;

     position.x = (float)aColIdx * mTexSize.x;
     position.y = (float)aRowIdx * mTexSize.y;

     mTexturePosition.push_back( position );
   }
}
//------------------------------------------------------------------------------
bool FluxTexture::getTextureRectById( Uint32 lImgId, Point2F& position, Point2F& size )
{
  position = { 0.f, 0.f };
  size = mTexSize;

  //we have no parts, return here
  if (mCols == 1 && mRows == 1)
  {
    position = { 0.f, 0.f };
    size = mTexSize;
    return true;
  }
  lImgId =  lImgId % mTexturePosition.size() ;
  position = mTexturePosition.at(lImgId);
  return true;
}
//------------------------------------------------------------------------------
void FluxTexture::setManual(GLuint handle, int w, int h)
{
  if (mLoaded && mHandle != 0) {
    glDeleteTextures(1, &mHandle);
  }
  mHandle = handle;
  mW = w;
  mH = h;
  mTexSize = { (float)w, (float)h };
  mLoaded = true;
}
//------------------------------------------------------------------------------
void FluxTexture::setUseTrilinearFiltering()
{
  mUseTrilinearFiltering = true;
  if (mLoaded && mHandle) {
    // Bind the existing texture ID
    glBindTexture(GL_TEXTURE_2D, mHandle);

    // Set the Trilinear Filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Ensure Mipmaps are generated
    glGenerateMipmap(GL_TEXTURE_2D);
  }
}
//------------------------------------------------------------------------------
// Atlas generation
//------------------------------------------------------------------------------
void FluxTexture::addToAtlas(const std::string& filename)
{
  mPendingFiles.push_back(filename);
}
//------------------------------------------------------------------------------
void FluxTexture::generateAtlas(int maxRows, bool setColorKeyAtZeroPixel, bool usePixelPerfect) {
  if (mPendingFiles.empty()) return;

  // Get uniform dimensions using your existing Android-safe helper
  SDL_Surface* firstSurf = loadWithSTB(mPendingFiles[0].c_str());
  if (!firstSurf) return;
  int imgW = firstSurf->w;
  int imgH = firstSurf->h;
  SDL_DestroySurface(firstSurf);

  // Calculate Grid Layout
  int totalImages = (int)mPendingFiles.size();
  mRows = std::min(totalImages, maxRows);
  mCols = (totalImages + mRows - 1) / mRows; // Ceiling division

  // Set total atlas size
  setSize(mCols * imgW, mRows * imgH);

  // Initialize OpenGL Texture
  glGenTextures(1, &mHandle);
  glBindTexture(GL_TEXTURE_2D, mHandle);

  if (usePixelPerfect) {
    mUseTrilinearFiltering = false;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else {
    setUseTrilinearFiltering(); // Set your internal bool
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  // Allocate empty GPU canvas
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  // Load and Stitch
  for (int i = 0; i < totalImages; ++i) {
    int col = i % mCols;
    int row = i / mCols;

    SDL_Surface* lSurface = loadWithSTB(mPendingFiles[i].c_str());
    if (lSurface) {
      SDL_Surface* finalSurface = nullptr;

      if (setColorKeyAtZeroPixel) {
        Uint8 r, g, b, a;
        if (SDL_ReadSurfacePixel(lSurface, 0, 0, &r, &g, &b, &a)) {
          SDL_SetSurfaceColorKey(lSurface, true, SDL_MapSurfaceRGB(lSurface, r, g, b));
        }
        // Convert to bake the colorkey into the Alpha channel
        finalSurface = SDL_ConvertSurface(lSurface, SDL_PIXELFORMAT_RGBA32);
      } else {
        // Ensure format matches OpenGL expectations
        finalSurface = SDL_ConvertSurface(lSurface, SDL_PIXELFORMAT_RGBA32);
      }

      if (finalSurface) {
        // Upload this sub-tile to the GPU atlas
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        col * imgW, row * imgH,
                        imgW, imgH,
                        GL_RGBA, GL_UNSIGNED_BYTE,
                        finalSurface->pixels);

        SDL_DestroySurface(finalSurface);
      }
      SDL_DestroySurface(lSurface);
    }
  }

  // Finalize
  if (mUseTrilinearFiltering) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  mLoaded = true;
  mPendingFiles.clear();
  setParts(mCols, mRows);
}

//------------------------------------------------------------------------------
// Save PNG To File
//------------------------------------------------------------------------------
/*
 Example:
 ========
    char* prefPath = SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ) ;
    std::string fullPath = std::string(prefPath) + "myPicture.png";
    myTexture->savePNGToFile(fullPath.c_str());
    SDL_free(prefPath);
*/
bool FluxTexture::savePNGToFile(const char* filename)
{
  if (!mLoaded || mHandle == 0) return false;

  // Create a buffer to hold the pixel data
  int channels = 4; // RGBA
  std::vector<unsigned char> pixels(mW * mH * channels);

  // Read pixels from GPU
  // On Android/GLES, glGetTexImage is not available, so we use an FBO
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mHandle, 0);

  // Ensure alignment is 1-byte to avoid row-padding issues
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, mW, mH, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  // Cleanup FBO
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);

  // Flip pixels vertically (OpenGL is bottom-to-top, PNG is top-to-bottom)
  stbi_flip_vertically_on_write(true);

  // Save to file using STB
  // Note: On Android, saving to the app's internal storage or SD card
  // requires a path where the app has write permissions (e.g., SDL_GetPrefPath)
  int success = stbi_write_png(filename, mW, mH, channels, pixels.data(), mW * channels);

  if (!success) {
    SDL_Log("[error] STB Error: Could not save image to %s", filename);
    return false;
  }

  return true;
}
