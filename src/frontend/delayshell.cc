/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <vector>
#include <string>
#include <algorithm> // for std::find

#include "delay_queue.hh"
#include "util.hh"
#include "ezio.hh"
#include "packetshell.cc"

using namespace std;

int main(int argc, char *argv[]) {
    try {
        const bool passthrough_until_signal = getenv("MAHIMAHI_PASSTHROUGH_UNTIL_SIGNAL");

        /* clear environment while running as root */
        char **const user_environment = environ;
        environ = nullptr;

        check_requirements(argc, argv);

        string egress_ip;
        string ingress_ip;

        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--egress" && i + 1 < argc) {
                egress_ip = argv[i + 1];
                i++;
            } else if (arg == "--ingress" && i + 1 < argc) {
                ingress_ip = argv[i + 1];
                i++;
            }
        }

        if (argc < 2) {
            throw runtime_error("Usage: " + string(argv[0]) + " [--egress IP] [--ingress IP] delay-milliseconds [command...]");
        }

        int delay_arg_pos = 1;
        while (delay_arg_pos < argc && string(argv[delay_arg_pos]).find("--") == 0) {
            delay_arg_pos += 2;
        }

        if (delay_arg_pos >= argc) {
            throw runtime_error("Missing required delay-milliseconds parameter");
        }

        const uint64_t delay_ms = myatoi(argv[delay_arg_pos]);

        vector<string> command;

        if (delay_arg_pos == argc - 1) {
            command.push_back(shell_path());
        } else {
            for (int i = delay_arg_pos + 1; i < argc; i++) {
                command.push_back(argv[i]);
            }
        }

        // if (egress_ip.empty() || ingress_ip.empty()) {
        //     throw runtime_error("Both --egress and --ingress must be specified");
        // }

        PacketShell<DelayQueue> delay_shell_app("delay", user_environment, passthrough_until_signal, 
            egress_ip.empty() ? nullptr : egress_ip.c_str(),
            ingress_ip.empty() ? nullptr : ingress_ip.c_str());

        delay_shell_app.start_uplink("[delay " + to_string(delay_ms) + " ms] ", command, delay_ms);
        delay_shell_app.start_downlink(delay_ms);
        return delay_shell_app.wait_for_exit();
    } catch (const exception &e) {
        print_exception(e);
        return EXIT_FAILURE;
    }
}