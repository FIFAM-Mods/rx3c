#include "ExtractTextures.h"
#include "ExtractNames.h"
#include "shared.h"
#include "D3DDevice.h"
#include <d3dx9.h>
#include "errormsg.h"

const bool exportCubeMapToSingleFile = false;

void ExtractTexturesFromContainer(Rx3Container &container, path const &outputDir, bool mipmaps, set<string> texturesToExtract) {
    static unsigned char ddsHeaderDXT1[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderDXT3[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderDXT5[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderATI1[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x0A, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x56, 0x54, 0x54, 0x08, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x41, 0x54, 0x49, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderATI2[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x0A, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x56, 0x54, 0x54, 0x08, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x41, 0x54, 0x49, 0x32, 0x41, 0x32, 0x58, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderL8[128]   = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x08, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderA8R8G8B8[128] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static unsigned char ddsHeaderBC6[148] = { 0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x31, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    create_directories(outputDir);
    auto texNamesSection = container.FindFirstSection(RX3_SECTION_NAMES);
    vector<string> texNames;
    if (texNamesSection) {
        auto names = ExtractNamesFromSection(texNamesSection);
        for (auto const &[id, name] : names) {
            if (id == RX3_SECTION_TEXTURE)
                texNames.push_back(name);
        }
    }
    auto textureSections = container.FindAllSections(RX3_SECTION_TEXTURE);
    for (size_t i = 0; i < textureSections.size(); i++) {
        string texName;
        if (i < texNames.size() && !texNames[i].empty())
            texName = texNames[i];
        else
            texName = "unnamed_" + to_string(i);
        bool canExtract = false;
        if (texturesToExtract.empty())
            canExtract = true;
        else {
            for (auto const &i : texturesToExtract) {
                if (texName == i) {
                    texName = i;
                    canExtract = true;
                    break;
                }
            }
        }
        if (options().addRx3Name)
            texName = texName + "@" + container.name;
        //if (options().onlyColor) {
        //    if (texName.ends_with("_color"))
        //        texName = texName.substr(0, texName.length() - 6);
        //    else
        //        canExtract = false;
        //}
        if (canExtract) {
            Rx3Reader reader(textureSections[i]);
            unsigned int totalSize = reader.Read<unsigned int>();
            unsigned char type = unsigned char(reader.Read<unsigned char>());
            unsigned char *ddsHeader = nullptr;
            unsigned char format = reader.Read<unsigned char>();
            bool isDx10 = false;
            if (format == 0)
                ddsHeader = ddsHeaderDXT1;
            else if (format == 1)
                ddsHeader = ddsHeaderDXT3;
            else if (format == 2)
                ddsHeader = ddsHeaderDXT5;
            else if (format == 3)
                ddsHeader = ddsHeaderA8R8G8B8;
            else if (format == 4)
                ddsHeader = ddsHeaderL8;
            else if (format == 7)
                ddsHeader = ddsHeaderATI2;
            else if (format == 10) {
                ddsHeader = ddsHeaderBC6;
                isDx10 = true;
            }
            else if (format == 12)
                ddsHeader = ddsHeaderATI1;
            else {
                ErrorMessage(Format("Unknown texture format: %d", format));
                continue;
            }
            if (ddsHeader) {
                
                reader.Skip(2);
                unsigned short width = reader.Read<unsigned short>();
                unsigned short height = reader.Read<unsigned short>();
                unsigned short faces = reader.Read<unsigned short>();
                unsigned short levels = unsigned char(reader.Read<unsigned short>());
                if (levels > 0) {
                    if (!mipmaps)
                        levels = 1;
                    bool isCubemap = exportCubeMapToSingleFile && type == 3 && faces == 6;
                    FILE *file = nullptr;
                    path texPath;
                    bool written = false;
                    for (unsigned int f = 0; f < faces; f++) {
                        for (unsigned short l = 0; l < levels; l++) {
                            reader.Skip(8);
                            unsigned int levelSize = reader.Read<unsigned int>();
                            reader.Skip(4);
                            if (l == 0 && (f == 0 || !isCubemap)) {
                                *(unsigned short *)(&ddsHeader[12]) = height;
                                *(unsigned short *)(&ddsHeader[16]) = width;
                                *(unsigned short *)(&ddsHeader[20]) = levelSize;
                                *(unsigned char *)(&ddsHeader[28]) = unsigned char(levels);
                                *(unsigned int *)(&ddsHeader[108]) = isCubemap ? (levels > 1 ? 0x401008 : 0x1008) : (levels > 1 ? 0x401000 : 0x1000);
                                *(unsigned int *)(&ddsHeader[112]) = isCubemap ? 0xFE00 : 0;
                                if (isDx10) {
                                    if (isCubemap) {
                                        *(unsigned short *)(&ddsHeader[136]) = 0x4;
                                        *(unsigned short *)(&ddsHeader[140]) = faces;
                                    }
                                }
                                string fileName = texName;
                                if (faces > 1 && !isCubemap)
                                    fileName += "._face_" + to_string(f);
                                texPath = outputDir / (fileName + ".dds");
                                _wfopen_s(&file, texPath.c_str(), L"wb");
                                if (file)
                                    fwrite(ddsHeader, isDx10 ? 148 : 128, 1, file);
                            }
                            if (file) {
                                fwrite(reader.GetCurrentPtr(), 1, levelSize, file);
                                written = true;
                            }
                            reader.Skip(levelSize);
                        }
                        if (file) {
                            fclose(file);
                            file = nullptr;
                        }
                    }
                    if (file) {
                        fclose(file);
                        file = nullptr;
                    }
                    if (/*options().texturesToPNG &&*/ written) {
                        IDirect3DTexture9 *d3dTex = nullptr;
                        if (SUCCEEDED(D3DXCreateTextureFromFileW(globalVars().device->Interface(), texPath.c_str(), &d3dTex))) {
                            path destPath = texPath;
                            destPath.replace_extension(".png");
                            D3DXSaveTextureToFileW(destPath.c_str(), D3DXIFF_PNG, d3dTex, NULL);
                            d3dTex->Release();
                        }
                        error_code ec;
                        remove(texPath, ec);
                    }
                }
            }
        }
    }
}

void ExtractTexturesFromContainer(Rx3Container &container, path const &outputDir) {
    ExtractTexturesFromContainer(container, outputDir, true, set<string>());
}

void ExtractTexturesFromRX3(path const &rx3path, path const &outputPath, bool mipmaps, set<string> texturesToExtract) {
    Rx3Container rx3(rx3path);
    ExtractTexturesFromContainer(rx3, outputPath, mipmaps, texturesToExtract);
}

void ExtractTexturesFromRX3(path const &rx3path, path const &outputPath) {
    Rx3Container rx3(rx3path);
    ExtractTexturesFromContainer(rx3, outputPath);
}
