#ifndef ENTRY_H
#define ENTRY_H

#include <string>

struct Entry {
    std::string service;
    std::string username;
    std::string password;
    std::string notes;

    // Serialization / Deserialization helpers
    std::string serialize() const {
        // Proper escaping should be used in a real app, but this works for simple splits
        return service + "\n" + username + "\n" + password + "\n" + notes;
    }
};

#endif