#ifndef IMAGESYNC_IMAGESYNCTOOLS_H
#define IMAGESYNC_IMAGESYNCTOOLS_H

#include <boost/filesystem.hpp>
#include <Magick++.h>

namespace fs = boost::filesystem;

class ImageSyncContext {
public:
    ImageSyncContext();

    explicit ImageSyncContext(size_t initial_size);

    size_t getSize();

    void emplace_back(const std::string &key, const std::time_t &value);

    std::time_t get(const std::string &key);

    void perform_sort();

    std::vector<std::pair<std::string, std::time_t> > &get_data();

private:
    std::vector<std::pair<std::string, std::time_t> > modify_timestamps;
};

void scan_directory_to_queue(const fs::path &path, std::deque<fs::path> &queue);

void process_image(const std::string &from, const std::string &to);

void process_images_batch(const fs::path &input, const fs::path &output,
                          std::deque<fs::path> &data, ImageSyncContext &context);

#endif //IMAGESYNC_IMAGESYNCTOOLS_H
