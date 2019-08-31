#include <iostream>
#include <boost/program_options.hpp>
#include <csignal>

#include "client.h"

using std::string;
namespace po = boost::program_options;

void invalid_option(const string &where, const string &val) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               where,
                               val);
}

ClientNode readOptions(int argc, char **argv) {
    string MCAST_ADDR, SHRD_FLDR;
    unsigned int CMD_PORT;
    int TIMEOUT;
    std::string OUT_FLDR;

    po::options_description desc("Allowed options");
    desc.add_options()
            (",g", po::value<string>(&MCAST_ADDR)
                    ->required(), "MCAST_ADDR")
            (",p", po::value<unsigned int>(&CMD_PORT)
                    ->required(), "CMD_PORT")
            (",o", po::value<std::string>(&OUT_FLDR)
                    ->required(), "OUT_FLDR")
            (",t", po::value<int>(&TIMEOUT)
                    ->default_value(5)
                    ->notifier([](unsigned int v){
                        if (v > 300 || v < 1)
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

    return ClientNode(MCAST_ADDR, CMD_PORT, TIMEOUT, SHRD_FLDR);
}

int main(int argc, char *argv[]) {
    ClientNode client = readOptions(argc, argv);
    client.startConnection();

    for (;;) {
        try {
            std::cout <<"Write a command: \n";
            client.listen();

        } catch (NetstoreException &e) {
            std::cout << e.what() << " " << e.details() << "\n";
        }
    }

    return 0;
}