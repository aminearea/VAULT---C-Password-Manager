#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>

namespace crypto {
    using Bytes = std::vector<unsigned char>;

    Bytes random_bytes(size_t length);
    Bytes derive_key(const std::string& password, const Bytes& salt);
    Bytes encrypt(const Bytes& plaintext, const Bytes& key, const Bytes& iv);
    Bytes decrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& iv);
    std::string base64_encode(const Bytes& data);
    Bytes base64_decode(const std::string& data);
    std::string sha256(const Bytes& data);
}

// Declared outside the namespace so main.cpp can access it directly
std::string generate_password(int length);

#endif