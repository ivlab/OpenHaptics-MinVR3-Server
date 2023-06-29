
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <minvr3.h>

int main(int argc, char** argv) {
    
    // default MinVR3 Connection server
    std::string ip = "localhost";
    int port = 9034;
    
    // optionally, override defaults with command line options
    if (argc > 1) {
        std::string arg = argv[1];
        if ((arg == "help") || (arg == "-h") || (arg == "-help") || (arg == "--help")) {
            std::cout << "Usage: force_server_example_client [ip-address] [port]" << std::endl;
            std::cout << "  * ip-address defaults to " << ip << std::endl;
            std::cout << "  * port defaults to " << port << std::endl;
            std::cout << "  * Quits if an event named 'Shutdown' is received, or press Ctrl-C" << std::endl;
            exit(0);
        }
        ip = arg;
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    
    
    MinVR3Net::Init();
    
    SOCKET server_fd;
    if (MinNet::ConnectTo(ip, port, &server_fd)) {

        float pos[3];
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;
        bool done = false;
        while (!done) {
            while (MinVR3Net::IsReadyToRead(&server_fd)) {
                VREvent* e = MinVR3Net::ReceiveVREvent(&server_fd);
                if (e->get_name() == "Phantom/Position") {
                    VREventVector3* epos = dynamic_cast<VREventVector3*>(e);
                    pos[0] = epos->x();
                    pos[1] = epos->y();
                    pos[2] = epos->z();
                    std::cout << epos->x() << " " << epos->y() << " " << epos->z() << std::endl;
                    /*
                    if (epos->y() < 0) {
                        MinVR3Net::SendVREvent(&server_fd, VREventString("ForceEffect/Start", "AmbientViscous"));
                    }
                    else {
                        MinVR3Net::SendVREvent(&server_fd, VREventString("ForceEffect/Stop", "AmbientViscous"));
                    }*/
                }
                else if (e->get_name() == "Phantom/Primary DOWN") {
                    MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/Param/PointConstraint/Point", pos[0], pos[1], pos[2]));
                    MinVR3Net::SendVREvent(&server_fd, VREventString("ForceEffect/Start", "PointConstraint"));
                }
                else if (e->get_name() == "Phantom/Primary UP") {
                    MinVR3Net::SendVREvent(&server_fd, VREventString("ForceEffect/Stop", "PointConstraint"));
                }

                //std::cout << *e << std::endl;
                if (e->get_name() == "Shutdown") {
                    done = true;
                }
                delete e;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        MinNet::CloseSocket(&server_fd);
    }
        
    MinVR3Net::Shutdown();
    return 0;
}
