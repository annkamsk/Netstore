//
// Created by anna on 14.05.19.
//

#include <iostream>
#include <boost/program_options.hpp>
#include <filesystem>

#include "server.h"

using std::string;
namespace po = boost::program_options;
namespace fs = std::filesystem;

void invalid_option(const string &where, const string &val) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               where,
                               val);
}

void readOptions(int argc, char **argv, ServerNode &server, Group &group) {
    string MCAST_ADDR, SHRD_FLDR;
    unsigned int CMD_PORT, TIMEOUT;
    unsigned long long MAX_SPACE;

    po::options_description desc("Allowed options");
    desc.add_options()
            (",g", po::value<string>(&MCAST_ADDR)
                    ->required(), "MCAST_ADDR")
            (",p", po::value<unsigned int>(&CMD_PORT)
                    ->required(), "CMD_PORT")
            (",b", po::value<unsigned long long>(&MAX_SPACE)
                    ->default_value(52428800), "MAX_SPACE")
            (",f", po::value<string>(&SHRD_FLDR)
                    ->required(), "SHRD_FLDR")
            (",t", po::value<unsigned int>(&TIMEOUT)
                    ->default_value(5)
                    ->notifier([](unsigned int v){
                        if (v > 300)
                            invalid_option("TIMEOUT", std::to_string(v));
                    }), "TIMEOUT");

    po::variables_map vm = nullptr;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << "ERROR " << e.what() << std::endl;
        exit(1);
    }

    group = Group(MCAST_ADDR, CMD_PORT);
    server = ServerNode(group, MAX_SPACE, TIMEOUT, SHRD_FLDR);
}

void indexFiles(ServerNode server) {
    fs::path folder(server.getFolder());

    if (!fs::exists(folder) || !fs::is_directory(folder)) {
        std::cerr << "ERROR " << server.getFolder() << " is invalid path." << std::endl;
        exit(1);
    }

    for (const auto &entry : fs::directory_iterator(folder)) {
        const fs::path& file(entry.path());
        server.addFile(entry.path().filename(), fs::file_size(file));
    }
}

void syserr(const string& desc, const string& val = "") {
    std::cerr << "ERROR " << val << desc << std::endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    ServerNode server;
    Group group;
    readOptions(argc, argv, server, group);
    indexFiles(server);

    group.openSocket();
    server.addToMcast();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}