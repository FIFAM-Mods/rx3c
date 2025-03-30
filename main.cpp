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

int wmain(int argc, wchar_t *argv[]) {
    CommandLine cmd(argc, argv, { L"i", L"o", L"game", L"skeleton" }, { L"recursive", L"createSubDir", L"silent", L"addRx3Name" });
    if (cmd.HasOption(L"silent"))
        SetErrorDisplayType(ErrorDisplayType::ERR_NONE);
    else {
        if (!cmd.HasOption(L"console"))
            SetErrorDisplayType(ErrorDisplayType::ERR_MESSAGE_BOX);
        else
            SetErrorDisplayType(ErrorDisplayType::ERR_CONSOLE);
    }
    path i;
    options().hasInput = cmd.HasArgument(L"i");
    if (!options().hasInput)
        i = current_path();
    else {
        i = cmd.GetArgumentPath(L"i");
        if (!exists(i)) {
            ErrorMessage("Input path does not exist");
            return ErrorType::INVALID_INPUT_PATH;
        }
    }
    path o;
    bool hasOutput = cmd.HasArgument(L"o");
    if (hasOutput)
        o = cmd.GetArgumentPath(L"o");
    options().createSubDir = cmd.HasOption(L"createSubDir");
    options().addRx3Name = !options().hasInput || cmd.HasOption(L"addRx3Name");
    if (cmd.HasArgument(L"skeleton"))
        options().skeletonPath = cmd.GetArgumentPath(L"skeleton");

    globalVars().device = new D3DDevice();
    globalVars().fbxManager = CreateFbxManager();

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
                if (out.has_parent_path())
                    create_directories(out.parent_path());
                if (ext == ".rx3") {
                    rx3export(in, out);
                    path texturesOutPath;
                    if (out.has_parent_path())
                        texturesOutPath = out.parent_path();
                    else
                        texturesOutPath = current_path();
                    ExtractTexturesFromRX3(in, texturesOutPath);
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

    DestroyFbxManager(globalVars().fbxManager);
    delete globalVars().device;
    return ErrorType::NONE;
}
