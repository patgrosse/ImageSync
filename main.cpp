#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <Magick++.h>
#include <iostream>

#include "ImageSyncTools.h"
#include "KeyValueConfig.h"

using namespace std;
using namespace Magick;

namespace fs = boost::filesystem;
namespace po = boost::program_options;

void perform_conversion(const fs::path &config_file, const fs::path &input,
                        const fs::path &output);

int main(const int argc, const char **argv) {
    cout << "ImageSync v0.1\n==============\n";
    po::options_description desc{"Options"};
    desc.add_options()("help,h", "Help screen")
            ("from,f", po::value<string>()->required(), "Read from this folder")
            ("to,t", po::value<string>()->required(), "Output to this folder")
            ("config,c", po::value<string>()->default_value("ImageSync.cfg"), "Config file location");

    po::variables_map vm;
    po::store(parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << '\n';
        return 0;
    }
    try {
        po::notify(vm);
    } catch (const po::error &ex) {
        cerr << ex.what() << '\n';
        cout << desc << '\n';
        return -1;
    }
    fs::path input(vm["from"].as<string>());
    fs::path output(vm["to"].as<string>());
    fs::path config_file(vm["config"].as<string>());
    InitializeMagick(*argv);
    perform_conversion(config_file, input, output);
    return 0;
}

void perform_conversion(const fs::path &config_file, const fs::path &input,
                        const fs::path &output) {
    cout << "Reading config\n";
    ImageSyncContext context{};
    if (fs::exists(config_file)) {
        function<long(string)> krams = [](string input) -> long {
            return atol(input.c_str());
        };
        KeyValueConfig::read_from_file(context.get_data(), config_file, krams);
    }
    context.perform_sort();
    deque<fs::path> data;
    scan_directory_to_queue(input, data);
    process_images_batch(input, output, data, context);
    cout << "Finished converting images\n";
    cout << "Saving config\n";
    function<string(long)> krams2 = [](long input) -> string {
        return to_string(input);
    };
    KeyValueConfig::write_to_file(context.get_data(), config_file, krams2);
    cout << "Successfully finished\n";
}

