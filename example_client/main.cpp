
/* todo:
- logic for ambient effects could be improved

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


float clamp(float val, float low, float high) {
    if (val < low) return low;
    else if (val > high) return high;
    else return val;
}


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

        bool viscosity_on = false;
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetGain", 0.8));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetMagnitudeCap", 1.0));

        bool friction_on = false;
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientFriction/SetGain", 0.1));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientFriction/SetMagnitudeCap", 0.1));

        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetStiffness", 0.8));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetDamping", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", 0.2));
        MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", 10.0));

        // a set of parallel lines to demonstrate line constraints
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/LineConstraint/BeginLines"));
        float left = -240;
        float right = -20;
        float xinc = 20;
        float top = 150;
        float bottom = -150;
        for (float x = left; x <= right; x += xinc) {
            MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/LineConstraint/AddVertex", x, top, 0));
            MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/LineConstraint/AddVertex", x, bottom, 0));
        }
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/LineConstraint/EndLines"));
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/LineConstraint/Start"));


        // a simple surface to demonstrate surface constraints
        left = 50;
        right = 175;
        top = 150;
        bottom = 50;
        float back = -25;
        float front = 25;
        // signal start of mesh data
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceConstraint/BeginGeometry"));
        // fill up vertex buffer
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", left, top, back));     // v0
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", left, bottom, front));  // v1
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", right, bottom, front)); // v2
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", right, top, back));    // v3
        // fill up indices buffer
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddIndices", 0, 1, 3));  // triangle 0 = v0, v1, v3
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceConstraint/AddIndices", 3, 1, 2));  // triangle 1 = v3, v1, v2
        // signal end of mesh data
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceConstraint/EndGeometry"));
        // start applying forces
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceConstraint/Start"));

        // a simple surface to demonstrate surface contact
        left = 50;
        right = 175;
        top = -50;
        bottom = -150;
        back = -25;
        front = 25;
        // signal start of mesh data
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceContact/BeginGeometry"));
        // fill up vertex buffer
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddVertex", left, top, back));     // v0
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddVertex", left, bottom, front));  // v1
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddVertex", right, bottom, front)); // v2
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddVertex", right, top, back));    // v3
        // fill up indices buffer
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddIndices", 0, 1, 3));  // triangle 0 = v0, v1, v3
        MinVR3Net::SendVREvent(&server_fd, VREventVector3("ForceEffect/SurfaceContact/AddIndices", 3, 1, 2));  // triangle 1 = v3, v1, v2
        // signal end of mesh data
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceContact/EndGeometry"));
        // start applying forces
        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/SurfaceContact/Start"));

        bool done = false;
        while (!done) {
            while (MinVR3Net::IsReadyToRead(&server_fd)) {
                VREvent* e = MinVR3Net::ReceiveVREvent(&server_fd);
                if (e->get_name() == "Phantom/Position") {
                    VREventVector3* epos = dynamic_cast<VREventVector3*>(e);
                    pos[0] = epos->x();
                    pos[1] = epos->y();
                    pos[2] = epos->z();
                   
                    std::cout << "Stylus pos: " << epos->x() << " " << epos->y() << " " << epos->z() << std::endl;
                    
                    // apply ambient viscosity whenever the stylus is above the Y = 0mm plane, turn off when below
                    if ((epos->y() > 0) && (!viscosity_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientViscous/Start"));
                        viscosity_on = true;
                    }
                    else if ((epos->y() < 0) && (viscosity_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientViscous/Stop"));
                        viscosity_on = false;
                    }

                    // apply ambient friction whenever the stylus is below the Y = 0mm plane, turn off when above
                    if ((epos->y() < 0) && (!friction_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientFriction/Start"));
                        friction_on = true;
                    }
                    else if ((epos->y() > 0) && (friction_on)) {
                        MinVR3Net::SendVREvent(&server_fd, VREvent("ForceEffect/AmbientFriction/Stop"));
                        friction_on = false;
                    }

                    // change the gain of both ambient effects so they increase left to right
                    // should feel little or no effect when moving the stylus on the left and big effect on the right
                    float alpha = clamp((epos->x() + 300.0f) / 600.0f, 0.0f, 1.0f);
                    MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientViscous/SetGain", alpha));
                    MinVR3Net::SendVREvent(&server_fd, VREventFloat("ForceEffect/AmbientFriction/SetGain", alpha));
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
