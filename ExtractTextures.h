#pragma once
#include "Rx3Container.h"
#include <set>

void ExtractTexturesFromContainer(Rx3Container &container, path const &outputDir, bool mipmaps, set<string> texturesToExtract);
void ExtractTexturesFromContainer(Rx3Container &container, path const &outputDir);
void ExtractTexturesFromRX3(path const &rx3path, path const &outputPath, bool mipmaps, set<string> texturesToExtract);
void ExtractTexturesFromRX3(path const &rx3path, path const &outputPath);
