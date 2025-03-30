#include "shared.h"

FbxManager *CreateFbxManager() {
    FbxManager *lSdkManager = FbxManager::Create();
    FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);
    return lSdkManager;
}

void DestroyFbxManager(FbxManager *manager) {
    manager->Destroy();
}

GlobalVars &globalVars() {
    static GlobalVars g;
    return g;
}

GlobalOptions &options() {
    static GlobalOptions o;
    return o;
}

UnicodeFbxStream::UnicodeFbxStream(const std::filesystem::path &path, const char *mode) : filePath(path), file(nullptr), lastError(0) {
    file = _wfopen(filePath.c_str(), std::wstring(mode, mode + strlen(mode)).c_str());
    if (!file)
        lastError = 1;
}

UnicodeFbxStream::~UnicodeFbxStream() {
    Close();
}

FbxStream::EState UnicodeFbxStream::GetState() {
    return (file && !feof(file)) ? eOpen : eClosed;
}

bool UnicodeFbxStream::Open(void *pStreamData) {
    return file != nullptr;
}

bool UnicodeFbxStream::Close() {
    bool result = false;
    if (file) {
        result = fclose(file) == 0;
        file = nullptr;
    }
    return result;
}

size_t UnicodeFbxStream::Write(const void *pData, FbxUInt64 pSize) {
    return file ? fwrite(pData, 1, pSize, file) : 0;
}

size_t UnicodeFbxStream::Read(void *pData, FbxUInt64 pSize) const {
    return file ? fread(pData, 1, pSize, file) : 0;
}

void UnicodeFbxStream::Seek(const FbxInt64 &pOffset, const FbxFile::ESeekPos &pSeekPos) {
    if (!file)
        return;
    int origin = (pSeekPos == FbxFile::eBegin) ? SEEK_SET : (pSeekPos == FbxFile::eCurrent ? SEEK_CUR : SEEK_END);
    _fseeki64(file, pOffset, origin);
}

fbxsdk::FbxInt64 UnicodeFbxStream::GetPosition() const {
    return file ? ftell(file) : -1;
}

bool UnicodeFbxStream::Flush() {
    if (file)
        return fflush(file) == 0;
    return false;
}

char *UnicodeFbxStream::ReadString(char *pBuffer, int pMaxSize, bool pStopAtFirstWhiteSpace) {
    if (!file || !pBuffer || pMaxSize <= 0) return nullptr;
    if (fgets(pBuffer, pMaxSize, file)) {
        if (pStopAtFirstWhiteSpace) {
            char *p = pBuffer;
            while (*p && !isspace(*p)) ++p;
            *p = '\0';
        }
        return pBuffer;
    }
    return nullptr;
}

int UnicodeFbxStream::GetReaderID() const {
    return -1;
}

int UnicodeFbxStream::GetWriterID() const {
    return -1;
}

void UnicodeFbxStream::SetPosition(FbxInt64 pPosition) {
    if (file) _fseeki64(file, pPosition, SEEK_SET);
}

int UnicodeFbxStream::GetError() const {
    return lastError;
}

void UnicodeFbxStream::ClearError() {
    lastError = 0;
}
