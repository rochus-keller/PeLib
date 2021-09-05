#ifndef STREAM_H
#define STREAM_H

#include <iostream>
#include <memory>
#include <map>

namespace DotNetPELib
{
class PEWriter;
class PELib;
class Resource;

class Stream
{
public:
    Stream(std::iostream*);
    Stream(PEWriter* w,PELib* l);

    std::iostream &Out() const { return *outputStream_; }
    void Swap(std::unique_ptr<std::iostream>& stream) { outputStream_.swap(stream); }

    PEWriter &PEOut() const { return *peWriter_; }
    void Find( const std::string& name, Resource** result );

    std::map<size_t, size_t> moduleRefs;
private:
    std::unique_ptr<std::iostream> outputStream_;
    PEWriter* peWriter_;
    PELib* peLib_;
};
}

#endif // STREAM_H
