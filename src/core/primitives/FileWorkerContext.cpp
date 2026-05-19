#include "core/primitives/FileWorkerContext.h"
#include "core/primitives/File.h"

namespace slr {

FileWorkerContext::
FileWorkerContext(FileWorker *ptr, std::vector<std::unique_ptr<File>> &files) :
    worker(ptr),
    files(files)
{

}

FileWorkerContext::~FileWorkerContext() {

}

}