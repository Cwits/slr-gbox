#include "core/utility/FileWorkerContext.h"
#include "core/primitives/File.h"

namespace slr {

FileWorkerContext::FileWorkerContext(FileWorker *ptr, std::vector<std::unique_ptr<File>> &files, IdGenerator *gen) :
    worker(ptr),
    files(files),
    gen(gen)
{

}

FileWorkerContext::~FileWorkerContext() {

}

}