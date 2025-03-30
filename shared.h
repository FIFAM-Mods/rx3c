#pragma once
#include "fbxsdk.h"
#include <filesystem>
#include <cstdio>
#include "D3DDevice.h"

using namespace std;
using namespace std::filesystem;

struct GlobalVars {
    D3DDevice *device = nullptr;
    FbxManager *fbxManager = nullptr;
};

GlobalVars &globalVars();

struct GlobalOptions {
    path skeletonPath;
    bool createSubDir = false;
    bool processingFolders = false;
    bool addRx3Name = false;
    bool hasInput = false;
};

GlobalOptions &options();

class UnicodeFbxStream : public FbxStream {
public:
    UnicodeFbxStream(const std::filesystem::path &path, const char *mode);
    virtual ~UnicodeFbxStream();
    virtual EState GetState() override;
    virtual bool Open(void *pStreamData) override;
    virtual bool Close() override;
    virtual size_t Write(const void *pData, FbxUInt64 pSize) override;
    virtual size_t Read(void *pData, FbxUInt64 pSize) const override;
    virtual void Seek(const FbxInt64 &pOffset, const FbxFile::ESeekPos &pSeekPos) override;
    virtual fbxsdk::FbxInt64 GetPosition() const override;
    virtual bool Flush() override;
    virtual char *ReadString(char *pBuffer, int pMaxSize, bool pStopAtFirstWhiteSpace = false) override;
    virtual int GetReaderID() const override;
    virtual int GetWriterID() const override;
    virtual void SetPosition(FbxInt64 pPosition) override;
    virtual int GetError() const override;
    virtual void ClearError() override;
private:
    std::filesystem::path filePath;
    mutable FILE *file;
    mutable int lastError;
};

FbxManager *CreateFbxManager();

void DestroyFbxManager(FbxManager *manager);

void rx3export(path const &in, path const &out);
void rx3import(path const &in, path const &out);
