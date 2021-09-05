#include "Stream.h"
#include "PELib.h"
using namespace DotNetPELib;

Stream::Stream(std::iostream* s):outputStream_(s),peWriter_(0),peLib_(0)
{

}

Stream::Stream(PEWriter* w, PELib* l):peWriter_(w),peLib_(l)
{

}

void Stream::Find(const std::string& name, Resource** result)
{
    peLib_->Find(name, result, nullptr, peLib_->MSCorLibAssembly() );
}

