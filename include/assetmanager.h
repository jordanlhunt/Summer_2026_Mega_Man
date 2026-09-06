#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#define MAX_TEXTURES 16
#define MAX_ASSET_PATH_LENGTH 256
#include "common.h"
typedef struct AssetManager {
  SDL_Texture *textures[MAX_TEXTURES];
  char texturePaths[MAX_TEXTURES][MAX_ASSET_PATH_LENGTH];
  int numberOfAssets;
} AssetManager;

void AssetManagerInitialize(AssetManager *assetManager);

/**
 * Returns the cashed texture for 'path', loading it from the disk on first
 * request. Returns NULL if loading fails or cache is full
 */
SDL_Texture *AssetManagerLoadTexture(AssetManager *assetManager,
                                     SDL_Renderer *renderer,
                                     const char *filePath);
void AssetManagerShutdown(AssetManager *assetManager);
#endif