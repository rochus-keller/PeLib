#include "bigdigits.h"
namespace DotNetPELib {

typedef unsigned char Byte; /* 1 byte */

class ByteArray
{
public:
    ByteArray(size_t len);
    ~ByteArray();
    
    Byte *operator()() { return mem_; }
    const Byte *operator()() const { return mem_; }
    Byte &operator[](size_t pos) { return *(mem_ + pos); }
    size_t size() { return len_; }

    Byte *begin() { return mem_; }
    Byte *end() { return mem_ + len_; }
private:
    Byte *mem_;
    size_t len_;
};
// format a message hash into PKCS1 format
// assumes SHA-1 hashing
class PKCS1Formatter
{
    public:
        PKCS1Formatter(const Byte *msg) : msg_(msg) { }
        void Calculate(ByteArray &result);
    private:
        const Byte *msg_;
        static Byte DerHeader[];
};

// Manage SNK file & perform RSA signature
class RSAEncoder
{
    public:
	RSAEncoder() : modulusBits(0), privateExponent(0), publicExponent(0), keyPair(0), modulus(0) { }
    virtual ~RSAEncoder();
	size_t LoadStrongNameKeys(const std::string & file);
	void GetPublicKeyData(Byte *key, size_t *keySize);
	void GetStrongNameSignature(Byte *sig, size_t *sigSize, const Byte *hash, size_t hashSize );

//private:
	size_t modulusBits;
    int publicExponent;
	Byte *privateExponent;
	Byte *keyPair;
	Byte *modulus;
};
}
