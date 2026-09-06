#include "assetmanager.h"
#include "graphics.h"
#include <SDL3/SDL_render.h>
#include <string.h>

void AssetManagerInitialize(AssetManager *assetManager) {
  memset(assetManager, 0, sizeof(*assetManager));
}

SDL_Texture *AssetManagerLoadTexture(AssetManager *assetManager,
                                     SDL_Renderer *renderer,
                                     const char *filePath) {
  if (assetManager->numberOfAssets >= MAX_TEXTURES) {
    SDL_Log("AssetManager full, cannot load: %s", filePath);
    return NULL;
  }
  for (int i = 0; i < assetManager->numberOfAssets; i++) {
    if (strcmp(assetManager->texturePaths[i], filePath) == 0) {
      return assetManager->textures[i];
    }
  }
  SDL_Texture *texture = GraphicsLoadSpriteSheet(renderer, filePath);
  if (texture == NULL) {
    return NULL;
  }
  strncpy(assetManager->texturePaths[assetManager->numberOfAssets], filePath,
          MAX_ASSET_PATH_LENGTH - 1);
  assetManager->textures[assetManager->numberOfAssets] = texture;
  assetManager->numberOfAssets += 1;
  return texture;
}

void AssetManagerShutdown(AssetManager *assetManager) {
  for (int i = 0; i < assetManager->numberOfAssets; i++) {
    SDL_DestroyTexture(assetManager->textures[i]);
  }
  memset(assetManager, 0, sizeof(*assetManager));
}