#ifndef IMAGESYNC_KEYVALUECONFIG_H
#define IMAGESYNC_KEYVALUECONFIG_H

#include <vector>
#include <boost/filesystem.hpp>
#include <map>
#include <iostream>
#include <sstream>

namespace fs = boost::filesystem;

template<typename T>
std::function<T(T)> identity() {
    return [](T v) -> T { return v; };
}

namespace KeyValueConfig {
    static const char SPLIT_CHAR = '=';

    template<class KeyType, class ValueType>
    void write_to_file(const std::vector<std::pair<KeyType, ValueType> > &data,
                       const fs::path &path,
                       std::function<std::string(ValueType)> stringifyValue = identity<ValueType>(),
                       std::function<std::string(KeyType)> stringifyKey = identity<KeyType>()) {
        fs::ofstream config_file(path);
        for (size_t i = 0; i < data.size(); i++) {
            config_file << stringifyKey(data[i].first) << SPLIT_CHAR << stringifyValue(data[i].second) << "\n";
        }
        config_file.close();
    }

    template<class KeyType, class ValueType>
    void read_from_file(std::vector<std::pair<KeyType, ValueType> > &data,
                        const fs::path &path,
                        std::function<ValueType(std::string)> destringifyValue = identity<ValueType>(),
                        std::function<KeyType(std::string)> destringifyKey = identity<KeyType>()) {
        fs::ifstream config_file(path);
        data.clear();
        std::string line;
        while (getline(config_file, line)) {
            std::istringstream iss(line);
            std::string token;
            KeyType key;
            ValueType value;
            if (!std::getline(iss, token, SPLIT_CHAR)) {
                return;
            }
            key = destringifyKey(token);
            if (!std::getline(iss, token, SPLIT_CHAR)) {
                return;
            }
            value = destringifyValue(token);
            data.emplace_back(key, value);
        }
        config_file.close();
    }
}

#endif //IMAGESYNC_KEYVALUECONFIG_H
