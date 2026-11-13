#include "core/utility/FileWorkerContext.h"
#include "core/primitives/File.h"

namespace slr {

FileWorkerContext::FileWorkerContext(
    FileWorker *ptr, 
    std::vector<std::unique_ptr<File>> &files, 
    IdGenerator *gen, 
    AudioCopyContext *actx) 
    :
    worker(ptr),
    files(files),
    gen(gen),
    _aCopyCtx(actx)
{

}

FileWorkerContext::~FileWorkerContext() {

}

File * FileWorkerContext::findFile(ID fileId) {
    File *ret = nullptr;

    auto it = std::find_if(
        files.begin(),
        files.end(),
        [fileId](const std::unique_ptr<File> &f) {
            return f->id() == fileId;
        }
    );

    if(it != files.end()) 
        ret = it->get();

    return ret;
}

}