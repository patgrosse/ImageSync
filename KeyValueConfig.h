#ifndef IMAGESYNC_KEYVALUECONFIG_H
#define IMAGESYNC_KEYVALUECONFIG_H

#include <vector>
#include <boost/filesystem.hpp>
#include <map>
#include <iostream>
#include <sstream>

namespace fs = boost::filesystem;

struct identity {
    template<typename U>
    constexpr auto operator()(U &&v) const noexcept
    -> decltype(std::forward<U>(v)) {
        return std::forward<U>(v);
    }
};

namespace KeyValueConfig {
    template<class KeyType, class ValueType, typename StringifyKey = identity, typename StringifyValue = identity>
    void write_to_file(const std::vector<std::pair<KeyType, ValueType> > &data,
                       const fs::path &path,
                       StringifyKey stringifyKey = StringifyKey(),
                       StringifyValue stringifyValue = StringifyValue()) {
        fs::ofstream config_file(path);
        for (size_t i = 0; i < data.size(); i++) {
            config_file << stringifyKey(data[i].first) << ":" << stringifyValue(data[i].second) << "\n";
        }
        config_file.close();
    }

    template<class KeyType, class ValueType, typename DestringifyKey = identity, typename DestringifyValue = identity>
    void read_from_file(std::vector<std::pair<KeyType, ValueType> > &data,
                        const fs::path &path,
                        DestringifyKey destringifyKey = DestringifyKey(),
                        DestringifyValue destringifyValue = DestringifyValue()) {
        fs::ifstream config_file(path);
        data.clear();
        std::string line;
        while (getline(config_file, line)) {
            std::istringstream iss(line);
            std::string token;
            KeyType key;
            ValueType value;
            if (!std::getline(iss, token, ':')) {
                return;
            }
            key = destringifyKey(token);
            if (!std::getline(iss, token, ':')) {
                return;
            }
            value = destringifyValue(token);
            data.emplace_back(key, value);
        }
        config_file.close();
    }
}

#endif //IMAGESYNC_KEYVALUECONFIG_H
