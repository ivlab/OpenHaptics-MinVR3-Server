
/* todo:
- line, surf constraints following same style
- surf contact following similar style
- examples of each
- remove tmp sphere from scene
- reinstate workspace and cursor drawing
..done..
..future..
- flow/follow effect
- drawing on air
*/
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

        bool effects_on = false;
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetGain", 0.8));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetMagnitudeCap", 1.0));

        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientFriction/SetGain", 0.1));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientFriction/SetMagnitudeCap", 0.1));

        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetStiffness", 0.8));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetDamping", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", 10.0));

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
                    
                    if ((epos->y() < 0) && (!effects_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientViscous/Start"));
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientFriction/Start"));
                        effects_on = true;
                    }
                    else if ((epos->y() > 0) && (effects_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientViscous/Stop"));
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientViscous/Stop"));
                        effects_on = false;
                    }

                    if (epos->x() < 0) {
                        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetGain", 0.3));
                    }
                    else {
                        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetGain", 0.9));
                    }
                }
                else if (e->get_name() == "Phantom/Primary DOWN") {
                    MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/PointConstraint/BeginPoints"));
                    MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/PointConstraint/AddVertex", pos[0], pos[1], pos[2]));
                    MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/PointConstraint/EndPoints"));
                    MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/PointConstraint/Start"));
                }
                else if (e->get_name() == "Phantom/Primary UP") {
                    MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/PointConstraint/Stop"));
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
