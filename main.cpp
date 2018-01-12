#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <Magick++.h>
#include <iostream>

#include "ImageSyncTools.h"
#include "KeyValueConfig.h"

using namespace std;
using namespace Magick;
namespace fs = boost::filesystem;

int main(int argc, char **argv) {
    ImageSyncContext context{};
    context.emplace_back("a", 1);
    context.emplace_back("b", 2);
    context.emplace_back("c", 3);
    context.emplace_back("d", 4);
    context.emplace_back("f", 5);
    context.emplace_back("g", 6);
    context.emplace_back("h", 7);
    context.emplace_back("h", 8);
    fs::path config_file{"ImageSync.cfg"};
    KeyValueConfig::write_to_file(context.get_data(), config_file);
    ImageSyncContext context2{};
    KeyValueConfig::read_from_file(context2.get_data(), config_file, identity(), [](std::string input) {
        return atol(input.c_str());
    });
    context2.perform_sort();
    /*InitializeMagick(*argv);
    std::cout << "ImageSync v0.1\n==============\n";
    fs::path input("/home/patrick/Nextcloud/Shares");
    fs::path output("/home/patrick/Nextcloud2/Shares");
    deque<fs::path> data;
    scan_directory_to_queue(input, data);
    ImageSyncContext context;
    process_images_batch(input, output, data, context);
    std::cout << "Finished converting images\n";*/
    return 0;
}

