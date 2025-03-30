#include "shared.h"
#include "commandline.h"
#include "errormsg.h"
#include "ExtractTextures.h"

const char *RX3C_VERSION = "0.001";
const unsigned int RX3C_VERSION_INT = 1;

enum ErrorType {
    NONE = 0,
    NOT_ENOUGHT_ARGUMENTS = 1,
    UNKNOWN_OPERATION_TYPE = 2,
    NO_INPUT_PATH = 3,
    INVALID_INPUT_PATH = 4,
    ERROR_OTHER = 5
};

int main(int argc, wchar_t *argv[]) {
    CommandLine cmd(argc, argv, { L"i", L"o", L"game" }, { L"recursive", L"createSubDir", L"silent", L"addRx3Name" });
    if (cmd.HasOption(L"silent"))
        SetErrorDisplayType(ErrorDisplayType::ERR_NONE);
    else {
        if (!cmd.HasOption(L"console"))
            SetErrorDisplayType(ErrorDisplayType::ERR_MESSAGE_BOX);
        else
            SetErrorDisplayType(ErrorDisplayType::ERR_CONSOLE);
    }
    enum OperationType {
        NOTSET, EXPORT, IMPORT
    } opType = OperationType::NOTSET;
    void (*callback)(path const &, path const &) = nullptr;
    bool isCustom = false;
    bool createDevice = true;
    bool createFbxManager = true;
    bool convertFbx = true;
    bool convertRx3 = true;
    if (argc >= 2) {
        wstring opTypeStr = argv[1];
        if (opTypeStr == L"export" || opTypeStr == L"unpack") {
            opType = OperationType::EXPORT;
            convertRx3 = false;
        }
        else if (opTypeStr == L"import" || opTypeStr == L"pack") {
            opType = OperationType::IMPORT;
            convertFbx = false;
        }
    }
    path i;
    options().hasInput = cmd.HasArgument(L"i");
    if (!options().hasInput)
        i = current_path();
    else {
        i = cmd.GetArgumentString(L"i");
        if (!exists(i)) {
            ErrorMessage("Input path does not exist");
            return ErrorType::INVALID_INPUT_PATH;
        }
    }
    path o;
    bool hasOutput = cmd.HasArgument(L"o");
    if (hasOutput)
        o = cmd.GetArgumentString(L"o");
    options().createSubDir = cmd.HasOption(L"createSubDir");
    options().addRx3Name = !options().hasInput || cmd.HasOption(L"addRx3Name");

    if (createDevice)
        globalVars().device = new D3DDevice();
    if (createFbxManager)
        globalVars().fbxManager = CreateFbxManager();

    if (!isCustom) {
        auto processFile = [=](path const &in, bool inDir) {
            try {
                string ext = ToLower(in.extension().string());
                if (!inDir || (is_regular_file(in) && (ext == ".rx3" || ext == ".fbx"))) {
                    string targetExt;
                    if (ext == ".rx3") {
                        if (!options().hasInput)
                            targetExt += "_unpacked";
                        targetExt = ".fbx";
                    }
                    else if (ext == ".fbx") {
                        if (!options().hasInput)
                            targetExt += "_packed";
                        targetExt = ".rx3";
                    }
                    path out;
                    if (hasOutput)
                        out = o;
                    else
                        out = in.parent_path();
                    if (!hasOutput || inDir) {
                        string targetFileName = in.stem().string();
                        string targetFileNameWithExt = targetFileName + targetExt;
                        if (options().createSubDir)
                            out = out / targetFileName / targetFileNameWithExt;
                        else
                            out = out / targetFileNameWithExt;
                    }
                    if (opType != EXPORT)
                        create_directories(out.parent_path());
                    if (ext == ".rx3") {
                        rx3export(in, out);
                        ExtractTexturesFromRX3(in, out.parent_path());
                    }
                }
            }
            catch (exception &e) {
                ErrorMessage(in.filename().string() + ": " + e.what());
            }
        };

        if (is_directory(i)) {
            options().processingFolders = true;
            vector<path> filesToProcess;
            if (cmd.HasOption(L"recursive")) {
                for (auto const &p : recursive_directory_iterator(i))
                    filesToProcess.push_back(p.path());
            }
            else {
                for (auto const &p : directory_iterator(i))
                    filesToProcess.push_back(p.path());
            }
            for (auto const &p : filesToProcess)
                processFile(p, true);
        }
        else
            processFile(i, false);
    }
    else
        callback(o, i);

    if (createFbxManager)
        DestroyFbxManager(globalVars().fbxManager);
    if (createDevice)
        delete globalVars().device;

    return ErrorType::NONE;
}
