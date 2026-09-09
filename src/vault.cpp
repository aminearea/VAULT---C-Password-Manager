#include "vault.h"
#include "crypto.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

const std::string MAGIC = "VAULT_FORMAT_1";

std::vector<Entry> Vault::deserialize(const std::string& data) {
    std::vector<Entry> entries;
    std::istringstream iss(data);
    std::string line;
    std::vector<std::string> fields;
    
    while (std::getline(iss, line)) {
        fields.push_back(line);
    }

    for (size_t i = 0; i + 3 < fields.size(); i += 4) {
        Entry e{fields[i], fields[i+1], fields[i+2], fields[i+3]};
        entries.push_back(e);
    }
    return entries;
}

std::string Vault::serialize(const std::vector<Entry>& entries) {
    std::string data;
    for (const auto& e : entries) {
        data += e.service + "\n" + e.username + "\n" + e.password + "\n" + e.notes + "\n";
    }
    return data;
}

Vault::Vault(const std::string& user_id, const std::string& master_password) 
    : user_id(user_id), master_password(master_password) {}

Vault Vault::load(const std::string& path, const std::string& user_id, const std::string& master_password) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open vault file: " + path);
    }

    std::string magic, salt_b64, iv_b64, ct_b64;
    std::getline(file, magic);
    std::getline(file, salt_b64);
    std::getline(file, iv_b64);
    std::getline(file, ct_b64);

    if (magic != MAGIC) {
        throw std::runtime_error("Invalid vault format");
    }

    auto salt = crypto::base64_decode(salt_b64);
    auto iv = crypto::base64_decode(iv_b64);
    auto ciphertext = crypto::base64_decode(ct_b64);

    auto key = crypto::derive_key(user_id + "::" + master_password, salt);
    auto plaintext_bytes = crypto::decrypt(ciphertext, key, iv); // Returns Bytes

    // FIX 1: Convert Bytes to std::string before deserializing
    std::string plaintext_str(plaintext_bytes.begin(), plaintext_bytes.end());
    auto entries = deserialize(plaintext_str);

    Vault vault(user_id, master_password);
    vault.entries_ = entries;
    return vault;
}

void Vault::save(const std::string& path) const {
    auto salt = crypto::random_bytes(16);
    auto iv = crypto::random_bytes(16);
    
    auto key = crypto::derive_key(user_id + "::" + master_password, salt);
    std::string plaintext_str = serialize(entries_);

    // FIX 2: Convert std::string to Bytes before encrypting
    crypto::Bytes plaintext_bytes(plaintext_str.begin(), plaintext_str.end());
    auto ciphertext = crypto::encrypt(plaintext_bytes, key, iv);

    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open vault file for writing: " + path);
    }

    file << MAGIC << "\n";
    file << crypto::base64_encode(salt) << "\n";
    file << crypto::base64_encode(iv) << "\n";
    file << crypto::base64_encode(ciphertext) << "\n";
}

void Vault::add_entry(const Entry& entry) {
    entries_.push_back(entry);
}

void Vault::remove_entry(size_t index) {
    if (index < entries_.size()) {
        entries_.erase(entries_.begin() + index);
    }
}

void Vault::update_entry(size_t index, const Entry& entry) {
    if (index < entries_.size()) {
        entries_[index] = entry;
    }
}

std::vector<Entry> Vault::entries() const {
    return entries_;
}

size_t Vault::size() const {
    return entries_.size();
}

bool Vault::empty() const {
    return entries_.empty();
}

std::vector<size_t> Vault::search(const std::string& query) const {
    std::vector<size_t> results;
    std::string lower_query = query;
    std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

    for (size_t i = 0; i < entries_.size(); ++i) {
        std::string haystack = entries_[i].service + " " + entries_[i].username + " " + entries_[i].notes;
        std::transform(haystack.begin(), haystack.end(), haystack.begin(), ::tolower);
        if (haystack.find(lower_query) != std::string::npos) {
            results.push_back(i);
        }
    }
    return results;
}

void Vault::change_master_password(const std::string& new_master_password) {
    master_password = new_master_password;
}