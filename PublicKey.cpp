#include <PublicKey.h>

#include <fstream>

#include <openssl/rsa.h>
#include <openssl/engine.h>

using namespace std;

namespace
{
    /** Mutex to protect the singleton public key from multiple loading */
    std::mutex g_serverKeyMutex;
}

namespace data
{
    /**
     * Implementation of the PublicKey - internally uses OpenSSL RSA public key.
     * It keeps the state of the public key and provides method to verify digital
     * signature.
     *
     * @author Ilia Yastrebov
     */
    class PublicKeyImpl
    {
    public:
        /**
         * Ctor.
         *
         * @param exponent Exponent of the public key
         * @param modulus Modulus of the public key
         */
        PublicKeyImpl(const string & exponent, const string & modulus)
        {
            BIGNUM * exp = BN_new();
            int len = BN_hex2bn(&exp, exponent.c_str());

            BIGNUM * mod = BN_new();
            len = BN_hex2bn(&mod, modulus.c_str());

            publicKey_ = RSA_new();
            publicKey_->n = mod;
            publicKey_->e = exp;
        }

        /**
         * Dtor.
         */
        ~PublicKeyImpl()
        {
            RSA_free(publicKey_);
        }

        /**
         * Verifies message signature with server's public key
         *
         * @param messageDigest Message digest
         * @param originalSignature Original signature
         * @return true is signature verified positively, false otherwise
         */
        bool verifySignature(const vector<signed char> & messageDigest,
                             const vector<signed char> & originalSignature) const
        {
            const unsigned char * signature = reinterpret_cast<const unsigned char *>(&originalSignature[0]);
            const unsigned char * message = reinterpret_cast<const unsigned char *>(&messageDigest[0]);
            int res = RSA_verify(NID_sha1, message, messageDigest.size(),
                    const_cast<unsigned char *>(signature), static_cast<unsigned int>(SIGNATURE_SIZE), publicKey_);
            return res == 1;
        }

    private:
        /** Hidden copy ctor and assignment operator */
        PublicKeyImpl(const PublicKeyImpl &);
        void operator = (const PublicKeyImpl &);

        /** OpenSSL public key */
        RSA * publicKey_;
    };

    /**
     * The method loads the public key exponent and modulus from the file
     *
     * @param fileName File name to read PK from.
     * @return Pair of exponent, modulus
     * @throw runtime_error in case of errors
     */
    pair<string, string> loadPublicKey(const string & fileName)
    {
        ifstream infile;
        infile.open(fileName.c_str(), std::ifstream::in);

        if (infile.is_open() == false) {
            throw std::runtime_error("Error opening file with public key: " + fileName);
        }

        string line;
        string exponent;
        string modulus;
        if (getline(infile, line)) {
            while (infile.peek() == '\r' || infile.peek() == '\n') {
                infile.get();
            }
            // line contains the public exponent
            exponent = line;
        } else {
            throw std::runtime_error("Error reading exponent from public key file: " + fileName);
        }

        if (getline(infile, line)) {
            // line contains the modulus
            modulus = line;
        } else {
            throw std::runtime_error("Error reading modulus from public key file: " + fileName);
        }
        return make_pair(exponent, modulus);
    }
}

PublicKey::PublicKey(const string & exponent, const string & modulus)
{
    impl_.reset(new PublicKeyImpl(exponent, modulus));
}

PublicKey::~PublicKey()
{
}

bool PublicKey::verifySignature(const vector<signed char> & messageDigest,
                                const vector<signed char> & originalSignature) const
{
    if (messageDigest.empty() || originalSignature.empty()) {
        return false;
    }

    // Signature must be exactly 64 bytes long.
    if (originalSignature.size() != SIGNATURE_SIZE) {
        return false;
    }

    if (NULL == impl_.get()) {
        return false;
    }

    return impl_->verifySignature(messageDigest, originalSignature);
}

const PublicKey & PublicKey::getServerPublicKey()
{
    IceUtil::Mutex::Lock lock(g_serverKeyMutex);
    static unique_ptr<PublicKey> serverPublicKey;
    if (NULL == serverPublicKey.get()) {
        string pkeyPath = Config::getPublicKeyPath();
        pair<string, string> data = loadPublicKey(pkeyPath);
        serverPublicKey.reset(new PublicKey(data.first, data.second));
    }

    return *serverPublicKey;
}
