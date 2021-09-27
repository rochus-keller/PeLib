#ifndef PEWRITER_H
#define PEWRITER_H

#include <vector>
#include <map>
#include <string>
#include <list>
#include "RSAEncoder.h"
#include "PEMetaTables.h"
#include "SEHData.h"

namespace DotNetPELib {

class TableEntryBase;
class PEMethod;
class SHA1Context;

typedef std::vector<TableEntryBase *> DNLTable;
typedef unsigned short Word; /* two bytes */
typedef unsigned DWord; /* four bytes */
typedef unsigned char Byte; /* 1 byte */

// This is the main class for generating a PE file
class PEWriter
{
public:
    // the maximum number of PE objects we will generate
    // this includes the following:
    //   .text / cildata
    //   .reloc (for the single necessary reloc entry)
    //   .rsrc (not implemented yet, will hold version info record)
    enum { MAX_PE_OBJECTS = 4 };

    // Constructor to instantiate class
    PEWriter(bool isexe, bool gui, const std::string& snkFile) : DLL_(!isexe), GUI_(gui), objectBase_(0), valueBase_(0), enumBase_(0),
        systemIndex_(0), entryPoint_(0), paramAttributeType_(0), paramAttributeData_(0),
        fileAlign_(0x200), objectAlign_(0x2000), imageBase_(0x400000), language_(0x4b0),
        peHeader_(nullptr), peObjects_(nullptr), cor20Header_(nullptr), tablesHeader_(nullptr),
        snkFile_(snkFile), snkLen_(0), outputFile_(nullptr), peBase_(0), corBase_(0), snkBase_(0) { }
    virtual ~PEWriter();
    // add an entry to one of the tables
    // note the data for the table will be a class inherited from TableEntryBase,
    // and this class will self-report the table index to use
    size_t AddTableEntry(TableEntryBase *entry);
    // add a method entry to the output list.  Note that Index_(D methods won't be added here.
    void AddMethod(PEMethod *method);
    // various functions to throw things into one of the streams, they return the stream index
    size_t HashString(const std::string& utf8);
    size_t HashUS(wchar_t* str, int len);
    size_t HashGUID(Byte *Guid);
    size_t HashBlob(Byte *blobData, size_t blobLen);
    // this is the 'cildata' contents.   Again we emit into the cildata and it returns the offset in
    // the cildata to use.  It does NOT return the rva immediately, that is calculated later
    size_t RVABytes(Byte *bytes, size_t data);

    // Set the indexes of the various classes which can be extended to make new classes
    // these are typically in the typeref table
    // Also set the index of the System namespace entry which is t
    void SetBaseClasses(size_t ObjectIndex, size_t ValueIndex, size_t EnumIndex, size_t SystemIndex);

    // this sets the data for the paramater attribute we support
    // we aren't generally supporting attributes in this version but we do need to be able to
    // set a single attribute that means a function has a variable length argument list
    void ParamAttribute(size_t paramAttributeType, size_t paramAttributeData)
    {
        paramAttributeType_ = paramAttributeType;
        paramAttributeData_ = paramAttributeData;
    }

    size_t ObjectBaseClass() const { return objectBase_; }
    size_t ValueBaseClass() const { return valueBase_; }
    size_t EnumBaseClass() const { return enumBase_; }
    size_t SystemName() const { return systemIndex_; }

    size_t ParamAttributeType() const { return paramAttributeType_; }
    size_t ParamAttributeData() const { return paramAttributeData_; }

    static void CreateGuid(Byte *Guid);

    size_t NextTableIndex(int table) const;
    bool WriteFile(int corFlags, std::iostream &out);
    void HashPartOfFile(SHA1Context &context, size_t offset, size_t len);

    // another thing that makes this lib not thread safe, the RVA for
    // the beginning of the .data section gets put here after it is calculated
    static DWord cildata_rva_;
protected:
    // this calculates various addresses and offsets that will be used and referenced
    // when we actually generate the data.   This must be kept in sync with the code to
    // generate data
    void CalculateObjects(int corFlags);
    // These functions put various information into the PE file
    bool WriteMZData() const; //
    bool WritePEHeader();//
    bool WritePEObjects() const;//
    bool WriteIAT() const;
    bool WriteCoreHeader();//
    bool WriteHashData();
    bool WriteStaticData() const;
    bool WriteMethods() const;
    bool WriteMetadataHeaders() const;//
    bool WriteTables() const;
    bool WriteStrings() const;
    bool WriteUS() const;
    bool WriteGUID() const;
    bool WriteBlob() const;
    bool WriteImports() const;
    bool WriteEntryPoint() const;

    bool WriteVersionInfo(const std::string& fileName) const;
    bool WriteRelocs() const;

    // a helper to put a string into the string area of the version information
    void VersionString(const wchar_t *name, const char *value) const;

    // Various helpers to put data to the output
    void put(const void *data, size_t size) const { outputFile_->write((char *)data, size); }
    std::streamoff offset() const { return outputFile_->tellp(); }
    void seek(size_t offset) const { outputFile_->seekp(offset); }
    void align(size_t offset) const;
private:
    std::iostream *outputFile_;
    std::string snkFile_;
    // a reflection of the String stream so that we can keep from doing duplicates.
    // right now we don't check duplicates on any of the other streams...
    std::map<std::string, size_t> stringMap_;
    struct pool
    {
        pool() : size(0), maxSize(200), base(nullptr) { base = (Byte *)calloc(1, maxSize); }
        ~pool() { free(base); }
        size_t size;
        size_t maxSize;
        Byte *base;
        void Ensure(size_t newSize);
    };
    DNLTable tables_[MaxTables];
    size_t entryPoint_;
    std::list<PEMethod *> methods_;
    size_t objectBase_;
    size_t valueBase_;
    size_t enumBase_;
    size_t systemIndex_;
    size_t paramAttributeType_, paramAttributeData_;
    bool DLL_;
    bool GUI_;
    pool strings_;
    pool us_;
    pool blob_;
    pool guid_;
    pool rva_;
    size_t fileAlign_;
    size_t objectAlign_;
    size_t imageBase_;
    DWord language_;
    Word assemblyVersion_[4];
    Word fileVersion_[4];
    Word productVersion_[4];
    struct PEHeader *peHeader_;
    struct PEObject *peObjects_;
    struct DotNetCOR20Header *cor20Header_;
    struct DotNetMetaTablesHeader *tablesHeader_;
    size_t streamHeaders_[5][2];
    size_t snkLen_;
    unsigned peBase_;
    unsigned corBase_;
    unsigned snkBase_;
    RSAEncoder rsaEncoder;
    static Byte MZHeader_[];
    static struct DotNetMetaHeader *metaHeader_;
    static const char *streamNames_[];
    static Byte defaultUS_[];
};

// this class holds the data for a method
// right now it holds redundant data for the function body
class PEMethod
{
public:
    enum Flags
    {
        TinyFormat = 2, // no local variables, MAXstack <=8, size < 64;
        FatFormat = 3,
        // more flags only availble for FAT format
        MoreSects = 8,
        InitLocals = 0x10,

        CIL = 0x4000, // not a real flag either
        EntryPoint = 0x8000 // not a real flag that goes in the PE file
    };
    PEMethod(bool hasSEH, int Flags, size_t MethodDef, int MaxStack, int localCount, int CodeSize, size_t signature);
    ~PEMethod();

    enum {
        EHTable = 1,
        OptILTable = 2,
        EHFatFormat = 0x40,
        EHMoreSects = 0x80,
    };
    std::vector<SEHData> sehData_;
    int flags_;
    int hdrSize_; /* = 3 */
    Word maxStack_;
    size_t codeSize_;
    Byte *code_;
    size_t signatureToken_;
    size_t rva_;
    size_t methodDef_;
    size_t Write(size_t sizes[MaxTables + ExtraIndexes], std::iostream &out) const;
private:
    PEMethod( const PEMethod& rhs );
    PEMethod& operator=( const PEMethod& rhs );
};

}
#endif // PEWRITER_H

