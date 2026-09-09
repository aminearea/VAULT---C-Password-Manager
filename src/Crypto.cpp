#include "Crypto.h"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdexcept>
#include <algorithm>

namespace crypto {

Bytes random_bytes(size_t length) {
    Bytes bytes(length);
    if (RAND_bytes(bytes.data(), static_cast<int>(length)) != 1) {
        throw std::runtime_error("Failed to generate random bytes");
    }
    return bytes;
}

Bytes derive_key(const std::string& password, const Bytes& salt) {
    // Using PBKDF2 with SHA256
    Bytes key(32); // 32 bytes for AES-256
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), 10000, EVP_sha256(), key.size(), key.data()) != 1) {
        throw std::runtime_error("Failed to derive key");
    }
    return key;
}

Bytes encrypt(const Bytes& plaintext, const Bytes& key, const Bytes& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    Bytes ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
    int len1 = 0, len2 = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1 ||
        EVP_EncryptUpdate(ctx, ciphertext.data(), &len1, plaintext.data(), plaintext.size()) != 1 ||
        EVP_EncryptFinal_ex(ctx, ciphertext.data() + len1, &len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Encryption failed");
    }

    ciphertext.resize(len1 + len2);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

Bytes decrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create cipher context");

    Bytes plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
    int len1 = 0, len2 = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()) != 1 ||
        EVP_DecryptUpdate(ctx, plaintext.data(), &len1, ciphertext.data(), ciphertext.size()) != 1 ||
        EVP_DecryptFinal_ex(ctx, plaintext.data() + len1, &len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed (wrong password?)");
    }

    plaintext.resize(len1 + len2);
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

std::string base64_encode(const Bytes& data) {
    if (data.empty()) return "";
    std::string encoded(4 * ((data.size() + 2) / 3), '\0');
    int len = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&encoded[0]), data.data(), data.size());
    encoded.resize(len);
    return encoded;
}

Bytes base64_decode(const std::string& data) {
    if (data.empty()) return {};
    Bytes decoded(3 * (data.size() / 4));
    int len = EVP_DecodeBlock(decoded.data(), reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    // Remove padding
    if (data.size() >= 1 && data[data.size() - 1] == '=') len--;
    if (data.size() >= 2 && data[data.size() - 2] == '=') len--;
    decoded.resize(len);
    return decoded;
}

std::string sha256(const Bytes& data) {
    Bytes hash(SHA256_DIGEST_LENGTH);
    SHA256(data.data(), data.size(), hash.data());
    
    // Convert to hex string
    static const char hex[] = "0123456789abcdef";
    std::string res;
    for (unsigned char c : hash) {
        res += hex[c >> 4];
        res += hex[c & 0xf];
    }
    return res;
}

} // namespace crypto

// Implementation of the standalone function
std::string generate_password(int length) {
    const std::string charset = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    crypto::Bytes random = crypto::random_bytes(length);
    std::string password;
    for (unsigned char c : random) {
        password += charset[c % charset.size()];
    }
    return password;
}