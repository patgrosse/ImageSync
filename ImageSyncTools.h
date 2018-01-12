#ifndef IMAGESYNC_IMAGESYNCTOOLS_H
#define IMAGESYNC_IMAGESYNCTOOLS_H

#include <boost/filesystem.hpp>
#include <Magick++.h>

using namespace std;

namespace fs = boost::filesystem;

class ImageSyncContext {
public:
    ImageSyncContext();

    explicit ImageSyncContext(size_t initial_size);

    size_t getSize();

    void emplace_back(const string &key, const time_t &value);

    time_t get(const string &key);

    void perform_sort();

    vector<pair<string, time_t> > &get_data();

private:
    vector<pair<string, time_t> > modify_timestamps;
};

void scan_directory_to_queue(const fs::path &path, deque<fs::path> &queue);

void process_image(const string &from, const string &to);

void process_images_batch(const fs::path &input, const fs::path &output,
                          deque<fs::path> &data, ImageSyncContext &context);

#endif //IMAGESYNC_IMAGESYNCTOOLS_H
