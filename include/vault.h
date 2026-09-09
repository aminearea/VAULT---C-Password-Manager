#ifndef VAULT_H
#define VAULT_H

#include <string>
#include <vector>
#include "Entry.h"

class Vault {
private:
    std::string user_id;
    std::string master_password;
    std::vector<Entry> entries_;

    static std::vector<Entry> deserialize(const std::string& data);
    static std::string serialize(const std::vector<Entry>& entries);

public:
    Vault(const std::string& user_id, const std::string& master_password);
    static Vault load(const std::string& path, const std::string& user_id, const std::string& master_password);

    void save(const std::string& path) const;
    void add_entry(const Entry& entry);
    void remove_entry(size_t index);
    void update_entry(size_t index, const Entry& entry);
    std::vector<Entry> entries() const;
    size_t size() const;
    bool empty() const;
    std::vector<size_t> search(const std::string& query) const;
    void change_master_password(const std::string& new_master_password);
};

#endif