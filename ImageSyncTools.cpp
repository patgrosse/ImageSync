#include "ImageSyncTools.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <iostream>

using namespace std;
using namespace Magick;

namespace fs = boost::filesystem;
namespace clk = chrono;

template<typename Loggable>
void log_msg_for_file(const string &file, const Loggable &msg) {
    cout << "[" << file << "] " << msg << "\n";
}


void process_images_batch(const fs::path &input, const fs::path &output,
                          deque<fs::path> &data, ImageSyncContext &context) {
    ImageSyncContext temp_context(context.getSize());
#pragma omp parallel
    {
        while (!data.empty()) {
            const auto time_start = clk::system_clock::now();
            fs::path path_absolute;
            bool empty = true;
#pragma omp critical(queue_lock)
            {
                if (!data.empty()) {
                    path_absolute = data.front();
                    data.pop_front();
                    empty = false;
                }
            };
            if (empty) {
                break;
            }
            const string path_absolute_str = path_absolute.string();
            const fs::path path_relative = relative(path_absolute, input);
            const fs::path path_to = absolute(path_relative, output);
            const string &path_to_str = path_to.string();
            log_msg_for_file(path_absolute_str, boost::format("Started processing to %1%") % path_to_str);
            if (exists(path_absolute)) {
                time_t cached_timestamp = context.get(path_absolute_str);
                time_t read_timestamp = fs::last_write_time(path_absolute);
                if (cached_timestamp != -1) {
                    bool not_modified = false;
                    if (read_timestamp == cached_timestamp) {
                        log_msg_for_file(path_absolute_str, "File was not modified since last run");
                        not_modified = true;
                    } else if (read_timestamp < cached_timestamp) {
                        log_msg_for_file(path_absolute_str,
                                         "Modify timestamp is older than the one read during the last run!");
                        not_modified = true;
                    }
                    if (not_modified) {
                        if (fs::exists(path_to)) {
                            temp_context.emplace_back(path_absolute_str, cached_timestamp);
                            continue;
                        } else {
                            log_msg_for_file(path_absolute_str, "File was not modified but is missing in destination");
                        }
                    }
                }
                temp_context.emplace_back(path_absolute_str, read_timestamp);
                create_directories(path_to.parent_path());
                process_image(path_absolute_str, path_to_str);
                const auto time_finish = clk::system_clock::now();
                const clk::duration<double, milli> time_span = time_finish - time_start;
                log_msg_for_file(path_absolute_str,
                                 boost::format("Successfully processed file in %1% ms") % time_span.count());
            } else {
                log_msg_for_file(path_absolute_str, boost::format("Could not read missing file"));
            }
        }
    }
    temp_context.perform_sort();
    context = temp_context;
}

void scan_directory_to_queue(const fs::path &path, deque<fs::path> &queue) {
    cout << "Start file discovery in folder " << path.string() << "\n";
    {
        fs::recursive_directory_iterator end, dir(path);
        while (dir != end) {
            if (boost::iequals(dir->path().extension().string(), ".jpg")) {
                queue.emplace_back(dir->path());
            }
            ++dir;
        }
    }
    cout << "Finished file discovery in folder " << path.string() << "\n";
    cout << "Found " << queue.size() << " image file(s)\n";
}

void process_image(const string &from, const string &to) {
    Image image;
    image.read(from);
    image.resize(Geometry(1920, 1080));
    image.write(to);
}

ImageSyncContext::ImageSyncContext() : modify_timestamps{} {
};

ImageSyncContext::ImageSyncContext(size_t initial_size) : modify_timestamps{initial_size} {
}

size_t ImageSyncContext::getSize() {
    return modify_timestamps.size();
}

void ImageSyncContext::emplace_back(const string &key, const time_t &value) {
    modify_timestamps.emplace_back(key, value);
}

struct TimestampMapComp {
    bool operator()(const pair<string, time_t> &p, const string &s) const {
        return p.first < s;
    }

    bool operator()(const string &s, const pair<string, time_t> &p) const {
        return s < p.first;
    }
};

template<class ForwardIt, class T, class Compare>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T &value, Compare comp) {
    first = lower_bound(first, last, value, comp);
    return first != last && !comp(value, *first) ? first : last;
}

time_t ImageSyncContext::get(const string &key) {
    auto it = binary_find(modify_timestamps.begin(), modify_timestamps.end(), key, TimestampMapComp());
    if (it != modify_timestamps.cend()) {
        return it.base()->second;
    } else {
        return -1;
    }
}

void ImageSyncContext::perform_sort() {
    sort(modify_timestamps.begin(), modify_timestamps.end(),
              [](pair<string, time_t> left, pair<string, time_t> right) {
                  return left.first < right.first;
              });
}

vector<pair<string, time_t> > &ImageSyncContext::get_data() {
    return modify_timestamps;
}
