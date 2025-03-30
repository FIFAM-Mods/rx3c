#include "shared.h"
#include "Rx3Container.h"
#include "ExtractNames.h"
#include "rx3utils.h"
#include <fbxsdk.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <map>
#include "errormsg.h"

using namespace std;
using namespace std::filesystem;

template <typename T>
inline double Normalize(T value, T minVal, T maxVal) {
    return (static_cast<double>(value) - minVal) / (maxVal - minVal);
}

float UnpackFloatFrom10Bit(int value) {
    return (value < 0) ? float(value) / 512.0f : float(value) / 511.0f;
}

float UnpackFloatFrom11Bit(int value) {
    return (value < 0) ? float(value) / 1024.0f : float(value) / 1023.0f;
}

FbxVector4 UncompressVertexAttribute(const char *format, const unsigned char *data) {
    if (!format || !data) return FbxVector4(0, 0, 0, 1);

    if (strcmp(format, "1f32") == 0) {
        float v; memcpy(&v, data, sizeof(v));
        return FbxVector4(v, 0, 0, 1);
    }
    if (strcmp(format, "1s32") == 0) {
        int32_t v; memcpy(&v, data, sizeof(v));
        return FbxVector4(v, 0, 0, 1);
    }
    if (strcmp(format, "1s16") == 0) {
        int16_t v; memcpy(&v, data, sizeof(v));
        return FbxVector4(v, 0, 0, 1);
    }
    if (strcmp(format, "1s8") == 0) {
        int8_t v; memcpy(&v, data, sizeof(v));
        return FbxVector4(v, 0, 0, 1);
    }

    if (strcmp(format, "2f32") == 0) {
        float v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], 0, 1);
    }
    if (strcmp(format, "2s32") == 0) {
        int32_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], 0, 1);
    }
    if (strcmp(format, "2s16") == 0) {
        int16_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], 0, 1);
    }
    if (strcmp(format, "2s8") == 0) {
        int8_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], 0, 1);
    }

    if (strcmp(format, "3f32") == 0) {
        float v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1);
    }
    if (strcmp(format, "3s32") == 0) {
        int32_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1);
    }
    if (strcmp(format, "3s16") == 0) {
        int16_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1);
    }
    if (strcmp(format, "3s8") == 0) {
        int8_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1);
    }

    if (strcmp(format, "4f32") == 0) {
        float v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }
    if (strcmp(format, "4s32") == 0) {
        int32_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }
    if (strcmp(format, "4s16") == 0) {
        int16_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }
    if (strcmp(format, "4s8") == 0) {
        int8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }

    // Unsigned 8-bit integer color
    if (strcmp(format, "4u8") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }

    // Normalized unsigned 8-bit color
    if (strcmp(format, "4u8n") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[0] / 255.0, (FbxDouble)v[1] / 255.0, (FbxDouble)v[2] / 255.0, (FbxDouble)v[3] / 255.0);
    }

    // Unsigned 8-bit integer color endian swap
    if (strcmp(format, "4u8endianswapp") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[3], v[2], v[1], v[0]);
    }

    // Normalized unsigned 8-bit color endian swap
    if (strcmp(format, "4u8nendianswap") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[3] / 255.0, (FbxDouble)v[2] / 255.0, (FbxDouble)v[1] / 255.0, (FbxDouble)v[0] / 255.0);
    }

    // 10-bit unsigned integer vector (e.g., GL_RGB10)
    if (strcmp(format, "3u10") == 0) {
        uint32_t packed; memcpy(&packed, data, sizeof(packed));
        uint32_t x = (packed >> 0) & 0x3FF;
        uint32_t y = (packed >> 10) & 0x3FF;
        uint32_t z = (packed >> 20) & 0x3FF;
        return FbxVector4((FbxDouble)x / 1023.0, (FbxDouble)y / 1023.0, (FbxDouble)z / 1023.0, 1);
    }

    // 10-bit signed normalized vector
    if (strcmp(format, "3s10n") == 0) {
        uint32_t packed;
        memcpy(&packed, data, sizeof(packed));
        int32_t x = ((packed >> 0) & 0x3FF) - ((packed & 0x200) ? 1024 : 0);
        int32_t y = ((packed >> 10) & 0x3FF) - ((packed & 0x80000) ? 1024 : 0);
        int32_t z = ((packed >> 20) & 0x3FF) - ((packed & 0x20000000) ? 1024 : 0);
        return FbxVector4(UnpackFloatFrom10Bit(x), UnpackFloatFrom10Bit(y), UnpackFloatFrom10Bit(z), 1);
    }

    // Half-float (16-bit float)
    if (strcmp(format, "2f16") == 0) {
        FbxHalfFloat v[2]; memcpy(v, data, sizeof(v));
        float x = v[0].value();
        float y = v[1].value();
        return FbxVector4(x, y, 0, 1);
    }
    if (strcmp(format, "4f16") == 0) {
        FbxHalfFloat v[4]; memcpy(v, data, sizeof(v));
        float x = v[0].value();
        float y = v[1].value();
        float z = v[2].value();
        float w = v[3].value();
        return FbxVector4(x, y, z, w);
    }

    // RGB565 format
    if (strcmp(format, "1u16rgb565") == 0) {
        uint16_t packed; memcpy(&packed, data, sizeof(packed));
        uint8_t r = (packed >> 11) & 0x1F;
        uint8_t g = (packed >> 5) & 0x3F;
        uint8_t b = (packed >> 0) & 0x1F;
        return FbxVector4((FbxDouble)r / 31.0, (FbxDouble)g / 63.0, (FbxDouble)b / 31.0, 1);
    }

    // RGBA4 format
    if (strcmp(format, "1u16rgba4") == 0) {
        uint16_t packed; memcpy(&packed, data, sizeof(packed));
        uint8_t r = (packed >> 12) & 0xF;
        uint8_t g = (packed >> 8) & 0xF;
        uint8_t b = (packed >> 4) & 0xF;
        uint8_t a = (packed >> 0) & 0xF;
        return FbxVector4((FbxDouble)r / 15.0, (FbxDouble)g / 15.0, (FbxDouble)b / 15.0, (FbxDouble)a / 15.0);
    }

    if (strcmp(format, "3s11n") == 0) {
        uint32_t packed;
        memcpy(&packed, data, sizeof(packed));
        int32_t x = ((packed >> 0) & 0x7FF) - ((packed & 0x400) ? 2048 : 0);
        int32_t y = ((packed >> 11) & 0x7FF) - ((packed & 0x200000) ? 2048 : 0);
        int32_t z = ((packed >> 22) & 0x3FF) - ((packed & 0x80000000) ? 1024 : 0);  // Last 10 bits
        return FbxVector4(UnpackFloatFrom11Bit(x), UnpackFloatFrom11Bit(y), UnpackFloatFrom10Bit(z), 1);
    }

    // 16-bit signed scaled values
    if (strcmp(format, "2s16s") == 0) {
        int16_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(static_cast<double>(v[0]), static_cast<double>(v[1]), 0, 1);
    }
    if (strcmp(format, "3s16s") == 0) {
        int16_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(static_cast<double>(v[0]), static_cast<double>(v[1]), static_cast<double>(v[2]), 1);
    }

    // 8-bit unsigned integer RGB
    if (strcmp(format, "3u8rgb8") == 0) {
        uint8_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1);
    }

    // 8-bit unsigned integer RGBX
    if (strcmp(format, "4u8rgbx8") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], 1); // Ignore X channel
    }

    // 6-bit-per-channel RGBA
    if (strcmp(format, "3u8rgba6") == 0) {
        uint8_t v[3]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[0] / 63.0, (FbxDouble)v[1] / 63.0, (FbxDouble)v[2] / 63.0, 1);
    }

    // Standard 8-bit RGBA
    if (strcmp(format, "4u8rgba8") == 0) {
        uint8_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[0] / 255.0, (FbxDouble)v[1] / 255.0, (FbxDouble)v[2] / 255.0, (FbxDouble)v[3] / 255.0);
    }

    // 16-bit unsigned integers
    if (strcmp(format, "2u16") == 0) {
        uint16_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], 0, 1);
    }
    if (strcmp(format, "4u16") == 0) {
        uint16_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4(v[0], v[1], v[2], v[3]);
    }

    // 16-bit unsigned normalized
    if (strcmp(format, "2u16n") == 0) {
        uint16_t v[2]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[0] / 65535.0, (FbxDouble)v[1] / 65535.0, 0, 1);
    }
    if (strcmp(format, "4u16n") == 0) {
        uint16_t v[4]; memcpy(v, data, sizeof(v));
        return FbxVector4((FbxDouble)v[0] / 65535.0, (FbxDouble)v[1] / 65535.0, (FbxDouble)v[2] / 65535.0, (FbxDouble)v[3] / 65535.0);
    }

    // Default case: return zero vector
    return FbxVector4(0, 0, 0, 1);
}

struct Matrix {
    float f[16];

    FbxAMatrix ToFbxMatrix() {
        FbxAMatrix transform;
        transform[0][0] = f[0];
        transform[0][1] = f[1];
        transform[0][2] = f[2];
        transform[0][3] = f[3];
        transform[1][0] = f[4];
        transform[1][1] = f[5];
        transform[1][2] = f[6];
        transform[1][3] = f[7];
        transform[2][0] = f[8];
        transform[2][1] = f[9];
        transform[2][2] = f[10];
        transform[2][3] = f[11];
        transform[3][0] = f[12];
        transform[3][1] = f[13];
        transform[3][2] = f[14];
        transform[3][3] = f[15];
        return transform;
    }
};

struct Bone {
    FbxNode *node = nullptr;
    string name;
    FbxAMatrix matrix;
    FbxAMatrix inversedMatrix;
    int parent = 0;
    int sibling = 0;
    vector<unsigned char> children;
};

struct VertexSkinning {
    FbxVector4 indices[2];
    FbxVector4 weights[2];
};

void rx3export(path const &in, path const &out) {
    Rx3Container rx3(in);
    auto indexBufferSections = rx3.FindAllSections(RX3_SECTION_INDEX_BUFFER);
    auto vertexBufferSections = rx3.FindAllSections(RX3_SECTION_VERTEX_BUFFER);
    auto vertexDeclarationSections = rx3.FindAllSections(RX3_SECTION_VERTEX_DECLARATION);
    auto boneMatricesSection = rx3.FindFirstSection(RX3_SECTION_BONE_MATRICES);
    auto skeletonsSections = rx3.FindAllSections(RX3_SECTION_PARTIAL_SKELETON);
    auto nodeSections = rx3.FindAllSections(RX3_SECTION_NODE);
    auto collisionSections = rx3.FindAllSections(RX3_SECTION_COLLISION);
    auto nodeNameSections = rx3.FindAllSections(RX3_SECTION_NODE_NAME);
    auto materialSections = rx3.FindAllSections(RX3_SECTION_MATERIAL);

    if (vertexBufferSections.empty())
        return;

    unsigned int numBones = 0;
    vector<Bone> bones;
    //Rx3Section *boneMatricesSection = nullptr;
    //vector<Rx3Section *> skeletonsSections;
    bool hasSkeleton = !options().skeletonPath.empty() && exists(options().skeletonPath);
    //if (hasSkeleton) {
    //    skeletonRx3.Load(skeletonPath);
    //    auto skeletonBoneMatricesSection = skeletonRx3.FindFirstSection(RX3_SECTION_BONE_MATRICES);
    //    if (skeletonBoneMatricesSection)
    //        boneMatricesSection = skeletonBoneMatricesSection;
    //    auto skeletonSkeletonsSections = skeletonRx3.FindAllSections(RX3_SECTION_SKELETON);
    //    if (!skeletonSkeletonsSections.empty())
    //        skeletonsSections = skeletonSkeletonsSections;
    //    auto boneNamesSection = skeletonRx3.FindFirstSection(RX3_SECTION_NAMES);
    //    if (boneNamesSection) {
    //        auto names = ExtractNamesFromSection(boneNamesSection);
    //        for (auto const &[id, name] : names) {
    //            if (id == RX3_SECTION_BONE)
    //                boneNames.push_back(name);
    //        }
    //    }
    //}
    if (hasSkeleton) {
        if (ToLower(options().skeletonPath.extension().string()) == ".txt") {
            auto lines = FileToLinesA(options().skeletonPath);
            for (auto const &line : lines) {
                if (!line.empty()) {
                    auto parts = Split(line, ',');
                    Bone bone;
                    if (parts.size() == 3) {
                        bone.name = parts[1];
                        bone.parent = SafeConvertInt<int>(parts[2]);
                    }
                    bones.push_back(bone);
                }
            }
        }
    }

    FbxScene *scene = FbxScene::Create(globalVars().fbxManager, in.stem().string().c_str());
    FbxNode *root = FbxNode::Create(scene, in.stem().string().c_str());
    scene->GetRootNode()->AddChild(root);

    if (hasSkeleton) {
        Rx3Reader boneMatricesReader(boneMatricesSection);
        boneMatricesReader.Skip(4);
        numBones = boneMatricesReader.Read<unsigned int>();
        boneMatricesReader.Skip(8);
        if (numBones > 0) {
            vector<Matrix> boneInversedMatrices;
            boneInversedMatrices.resize(numBones);
            memcpy(boneInversedMatrices.data(), boneMatricesReader.GetCurrentPtr(), boneInversedMatrices.size() * sizeof(Matrix));
            for (unsigned int b = 0; b < numBones; b++) {
                bones[b].inversedMatrix = boneInversedMatrices[b].ToFbxMatrix();
                bones[b].matrix = bones[b].inversedMatrix.Inverse();
            }
        }
        for (unsigned int b = 0; b < numBones; b++) {
            string boneName;
            if (!bones[b].name.empty())
                boneName = bones[b].name;
            else
                boneName = "unnamed_bone_" + to_string(b);
            bones[b].node = FbxNode::Create(scene, boneName.c_str());
            FbxSkeleton *boneAttribute = FbxSkeleton::Create(scene, (boneName + "_attr").c_str());
            boneAttribute->SetSkeletonType(bones[b].parent == -1 ? FbxSkeleton::eRoot : FbxSkeleton::eLimb);
            bones[b].node->SetNodeAttribute(boneAttribute);
            FbxAMatrix mat = bones[b].matrix;
            if (bones[b].parent != -1)
                mat = bones[bones[b].parent].inversedMatrix * bones[b].matrix;
            FbxDouble3 translation = mat.GetT();
            FbxDouble3 rotation = mat.GetR();
            FbxDouble3 scaling = mat.GetS();
            bones[b].node->LclTranslation.Set(translation);
            bones[b].node->LclRotation.Set(rotation);
            bones[b].node->LclScaling.Set(scaling);
        }
        for (unsigned int b = 0; b < numBones; b++) {
            if (bones[b].parent == -1)
                root->AddChild(bones[b].node);
            else
                bones[bones[b].parent].node->AddChild(bones[b].node);
        }
    }

    vector<string> primNames;
    vector<string> propNames;
    vector<string> texNames;
    auto geometryNameSections = rx3.FindFirstSection(RX3_SECTION_NAMES);
    if (geometryNameSections) {
        auto names = ExtractNamesFromSection(geometryNameSections);
        for (auto const &[id, name] : names) {
            if (id == RX3_SECTION_PRIMITIVE_TYPE)
                primNames.push_back(name);
            else if (id == RX3_SECTION_PROP)
                propNames.push_back(name);
            else if (id == RX3_SECTION_TEXTURE)
                texNames.push_back(name);
        }
    }
    unsigned int numPrimitives = indexBufferSections.size();
    unsigned int numMeshNodes = 1;
    primNames.resize(numPrimitives);
    vector<int> primTypes;
    auto geometryTypes = rx3.FindAllSections(RX3_SECTION_PRIMITIVE_TYPE);
    for (auto const &gt : geometryTypes) {
        Rx3Reader gtReader(gt);
        primTypes.push_back(gtReader.Read<int>());
    }
    primNames.resize(numPrimitives);
    for (unsigned int i = 0; i < numPrimitives; i++) {
        Rx3Reader vertexDeclReader(vertexDeclarationSections[i]);
        Rx3Reader vertexBufferReader(vertexBufferSections[i]);
        Rx3Reader indexBufferReader(indexBufferSections[i]);
        vertexDeclReader.Skip(4);
        unsigned int declStrLen = vertexDeclReader.Read<unsigned int>();
        if (declStrLen > 0) {
            vertexDeclReader.Skip(8);
            string decl = vertexDeclReader.GetString();
            //if (options().fifa16 && decl == "p0:00:00:0001:4f16 n0:08:00:0001:3s10n g0:0C:00:0001:3s10n t0:10:00:0001:2f16 t1:14:00:0001:2f16 i0:18:00:0001:4u8 w0:1C:00:0001:4u8n")
            //    decl = "p0:00:00:0001:4f16 n0:08:00:0001:3s10n g0:0C:00:0001:3s10n t0:10:00:0001:2f16 t1:14:00:0001:2f16 i0:18:00:0001:4u16 w0:20:00:0001:4u8n";
            auto declElements = Split(decl, ' ');
            if (!declElements.empty()) {
                vertexBufferReader.Skip(4);
                unsigned int numVertices = vertexBufferReader.Read<unsigned int>();
                unsigned int vertexStride = vertexBufferReader.Read<unsigned int>();
                vertexBufferReader.Skip(4);
                auto vertexBufferData = vertexBufferReader.GetCurrentPtr();
                indexBufferReader.Skip(4);
                unsigned int numIndices = indexBufferReader.Read<unsigned int>();
                unsigned char indexStride = indexBufferReader.Read<unsigned char>();
                indexBufferReader.Skip(7);
                void *indexBufferData = (void *)indexBufferReader.GetCurrentPtr();
                vector<FbxCluster *> clusters;
                FbxMesh *mesh = FbxMesh::Create(scene, "Mesh");
                mesh->InitControlPoints(numVertices);
                FbxVector4 *controlPoints = mesh->GetControlPoints();
                FbxLayer *layer = mesh->GetLayer(0);
                if (!layer) {
                    mesh->CreateLayer();
                    layer = mesh->GetLayer(0);
                }
                FbxLayerElementNormal *layerNormal = nullptr;
                map<unsigned int, FbxLayerElementUV *> layersUV;
                map<unsigned int, FbxLayerElementVertexColor *> layersColor;
                FbxLayerElementTangent *layerTangent = nullptr;
                FbxLayerElementBinormal *layerBinormal = nullptr;
                vector<VertexSkinning> skinning;
                if (hasSkeleton) {
                    skinning.resize(numVertices);
                    for (unsigned int v = 0; v < numVertices; v++) {
                        skinning[v].indices[0] = FbxVector4(0, 0, 0, 0);
                        skinning[v].indices[1] = FbxVector4(0, 0, 0, 0);
                        skinning[v].weights[0] = FbxVector4(0, 0, 0, 0);
                        skinning[v].weights[1] = FbxVector4(0, 0, 0, 0);
                    }
                }

                for (unsigned int d = 0; d < declElements.size(); d++) {
                    auto elementInfo = Split(declElements[d], ':');
                    if (elementInfo.size() == 5 && elementInfo[0].size() == 2 && elementInfo[1].size() == 2) {
                        string format = elementInfo[4];
                        unsigned int offset = 0;
                        sscanf_s(elementInfo[1].c_str(), "%02X", &offset);
                        if (elementInfo[0] == "p0") {
                            for (unsigned int vi = 0; vi < numVertices; vi++) {
                                unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                controlPoints[vi] = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                            }
                        }
                        else if (elementInfo[0] == "n0") {
                            layerNormal = FbxLayerElementNormal::Create(mesh, "");
                            layerNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
                            layerNormal->SetReferenceMode(FbxLayerElement::eDirect);
                            for (unsigned int vi = 0; vi < numVertices; vi++) {
                                unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                auto normal = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                layerNormal->GetDirectArray().Add(FbxVector4(normal[0], normal[1], normal[2]));
                            }
                            layer->SetNormals(layerNormal);
                        }
                        else if (elementInfo[0] == "g0") {
                            layerTangent = FbxLayerElementTangent::Create(mesh, "");
                            layerTangent->SetMappingMode(FbxLayerElement::eByControlPoint);
                            layerTangent->SetReferenceMode(FbxLayerElement::eDirect);
                            for (unsigned int vi = 0; vi < numVertices; vi++) {
                                unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                auto tangent = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                layerTangent->GetDirectArray().Add(FbxVector4(tangent[0], tangent[1], tangent[2]));
                            }
                            layer->SetTangents(layerTangent);
                        }
                        else if (elementInfo[0][0] == 't') {
                            unsigned int index = SafeConvertInt<unsigned int>(string(1, elementInfo[0][1]));
                            layersUV[index] = FbxLayerElementUV::Create(mesh, ("UV" + to_string(index + 1)).c_str());
                            layersUV[index]->SetMappingMode(FbxLayerElement::eByControlPoint);
                            layersUV[index]->SetReferenceMode(FbxLayerElement::eDirect);
                            for (unsigned int vi = 0; vi < numVertices; vi++) {
                                unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                auto uv = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                layersUV[index]->GetDirectArray().Add(FbxVector2(uv[0], uv[1]));
                            }
                            layer->SetUVs(layersUV[index], FbxLayerElement::eTextureDiffuse);
                        }
                        else if (elementInfo[0][0] == 'c') {
                            unsigned int index = SafeConvertInt<unsigned int>(string(1, elementInfo[0][1]));
                            layersColor[index] = FbxLayerElementVertexColor::Create(mesh, ("Color" + to_string(index + 1)).c_str());
                            layersColor[index]->SetMappingMode(FbxLayerElement::eByControlPoint);
                            layersColor[index]->SetReferenceMode(FbxLayerElement::eDirect);
                            for (unsigned int vi = 0; vi < numVertices; vi++) {
                                unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                auto color = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                layersColor[index]->GetDirectArray().Add(color);
                            }
                            layer->SetVertexColors(layersColor[index]);
                        }
                        else if (elementInfo[0][0] == 'i' && hasSkeleton) {
                            unsigned int index = SafeConvertInt<unsigned int>(string(1, elementInfo[0][1]));
                            if (index < 2) {
                                for (unsigned int vi = 0; vi < numVertices; vi++) {
                                    unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                    skinning[vi].indices[index] = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                }
                            }
                        }
                        else if (elementInfo[0][0] == 'w' && hasSkeleton) {
                            unsigned int index = SafeConvertInt<unsigned int>(string(1, elementInfo[0][1]));
                            if (index < 2) {
                                for (unsigned int vi = 0; vi < numVertices; vi++) {
                                    unsigned char const *vertexData = (unsigned char const *)vertexBufferData + vi * vertexStride;
                                    skinning[vi].weights[index] = UncompressVertexAttribute(format.c_str(), &vertexData[offset]);
                                }
                            }
                        }
                    }
                }
                auto GetIndex = [](void *buffer, int index, int stride) -> int {
                    uint8_t *base = static_cast<uint8_t *>(buffer) + index * stride;
                    switch (stride) {
                    case 1: return *reinterpret_cast<uint8_t *>(base);
                    case 2: return *reinterpret_cast<uint16_t *>(base);
                    case 4: return *reinterpret_cast<uint32_t *>(base);
                    }
                    return 0;
                };
                if (primTypes[i] == 6) {  // TriStrip
                    for (int i = 2; i < numIndices; i++) {
                        int i0 = GetIndex(indexBufferData, i - 2, indexStride);
                        int i1 = GetIndex(indexBufferData, i - 1, indexStride);
                        int i2 = GetIndex(indexBufferData, i, indexStride);
                        mesh->BeginPolygon();
                        if (i % 2 == 0) {
                            mesh->AddPolygon(i0);
                            mesh->AddPolygon(i1);
                            mesh->AddPolygon(i2);
                        }
                        else {
                            mesh->AddPolygon(i1);
                            mesh->AddPolygon(i0);
                            mesh->AddPolygon(i2);
                        }
                        mesh->EndPolygon();
                    }
                }
                else {
                    for (int i = 0; i < numIndices; i += 3) {
                        int i0 = GetIndex(indexBufferData, i, indexStride);
                        int i1 = GetIndex(indexBufferData, i + 1, indexStride);
                        int i2 = GetIndex(indexBufferData, i + 2, indexStride);
                        mesh->BeginPolygon();
                        mesh->AddPolygon(i0);
                        mesh->AddPolygon(i1);
                        mesh->AddPolygon(i2);
                        mesh->EndPolygon();
                    }
                }
                string nodeName;
                if (i < primNames.size() && !primNames[i].empty()) {
                    nodeName = primNames[i];
                    auto dp = nodeName.find("_.");
                    if (dp != string::npos)
                        nodeName = nodeName.substr(0, dp);
                }
                else
                    nodeName = "unnamed_" + to_string(i);
                FbxNode *node = FbxNode::Create(scene, nodeName.c_str());
                node->SetNodeAttribute(mesh);
                root->AddChild(node);
                if (hasSkeleton) {
                    FbxSkin *skin = FbxSkin::Create(scene, "Skin");
                    clusters.resize(numBones);
                    for (unsigned int b = 0; b < numBones; b++) {
                        clusters[b] = FbxCluster::Create(scene, ("Cluster_" + bones[b].name).c_str());
                        clusters[b]->SetLink(bones[b].node);
                        clusters[b]->SetLinkMode(FbxCluster::eNormalize);
                    }
                    for (unsigned int v = 0; v < skinning.size(); v++) {
                        for (unsigned int index = 0; index < 2; index++) {
                            for (unsigned int component = 0; component < 4; component++) {
                                FbxDouble weight = skinning[v].weights[index][component];
                                if (weight != 0.0) {
                                    int b = (int)skinning[v].indices[index][component];
                                    if (b >= 0 && b < numBones)
                                        clusters[b]->AddControlPointIndex(v, weight);
                                }
                            }
                        }
                    }
                    for (unsigned int b = 0; b < numBones; b++) {
                        FbxAMatrix meshBindMatrix = node->EvaluateGlobalTransform();
                        FbxAMatrix boneBindMatrix = bones[b].node->EvaluateGlobalTransform();
                        clusters[b]->SetTransformMatrix(meshBindMatrix);
                        clusters[b]->SetTransformLinkMatrix(boneBindMatrix);
                        skin->AddCluster(clusters[b]);
                    }
                    mesh->AddDeformer(skin);
                }
            }
        }
    }
    FbxExporter *exporter = FbxExporter::Create(globalVars().fbxManager, "");
    //UnicodeFbxStream *ustream = new UnicodeFbxStream(out, "wb");
    //if (exporter->Initialize(ustream, nullptr, -1, globalVars().fbxManager->GetIOSettings())) {
    //    exporter->Export(scene);
    //}
    if (exporter->Initialize(out.string().c_str(), -1, globalVars().fbxManager->GetIOSettings())) {
        exporter->Export(scene);
    }
    exporter->Destroy();
    //delete ustream;
    scene->Destroy();
}
