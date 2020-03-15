#ifndef DATA_PUBLIC_KEY_H
#define DATA_PUBLIC_KEY_H

#include <iostream>
#include <memory>
#include <vector>

namespace data
{
    class PublicKeyImpl;

    /**
     * data::PublicKey represents an abstract public key. Actual implementation
     * is hidden inside impl::PublicKeyImpl. This class provides method to load
     * public key from the exponent and modulus and also method that verifies
     * that the byte array was signed with the private key.
     *
     * @author Ilia Yastrebov
     */
    class PublicKey
    {
    public:
        /**
         * Constructs public key from exponent and modulus
         *
         * @param exponent Public key exponent
         * @param modulus  Public key modulus
         */
        PublicKey(const std::string & exponent, const std::string & modulus);

        /**
         * Dtor.
         */
        ~PublicKey();

        /**
         * Verifies message signature with server's public key
         *
         * @param messageDigest Message digest
         * @param originalSignature Original signature
         * @return true is signature verified positively, false otherwise
         */
        bool verifySignature(const std::vector<signed char> & messageDigest,
                             const std::vector<signed char> & originalSignature) const;

        /**
         * Returns Public key of the authentication server.
         *
         * @return Public key of the authentication server.
         * @throw exception if unable to load server public key
         */
        static const PublicKey & getServerPublicKey();

    private:
        /** Hidden copy ctor and assignment operator */
        PublicKey(const PublicKey &);
        void operator = (const PublicKey &);

        /** Pointer to the hidden implementation */
        std::unique_ptr<data::PublicKeyImpl> impl_;
    };

} // namespace data

#endif
