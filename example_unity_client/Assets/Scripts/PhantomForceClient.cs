using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using UnityEngine;



namespace IVLab.MinVR3
{

    public class PhantomForceClient : MonoBehaviour, IVREventProducer
    {
        public bool primaryBtnDown {
            get => m_PrimaryBtnDown;
        }

        public Vector3 stylusPosition {
            get => m_StylusPosition;
        }

        public Quaternion stylusRotation {
            get => m_StylusRotation;
        }


        void Reset()
        {
            m_ServerPhantomBtnDownEventName = "Phantom/Primary DOWN";
            m_ServerPhantomBtnUpEventName = "Phantom/Primary UP";
            m_ServerPhantomPositionEventName = "Phantom/Position";
            m_ServerPhantomRotationEventName = "Phantom/Rotation";

            m_PhantomBtnDownEventName = "Phantom/PrimaryBtn/Down";
            m_PhantomBtnUpEventName = "Phantom/PrimaryBtn/Up";
            m_PhantomPositionEventName = "Phantom/Position";
            m_PhantomRotationEventName = "Phantom/Rotation";
        }

        // Start is called before the first frame update
        void Start()
        {
            m_PrimaryBtnDown = false;
            m_ServerConnection.OnVREventReceived += OnVREventReceivedFromServer;
            SendTestStartCommands();
        }

        // Update is called once per frame
        void Update()
        {
        }

        public Vector3 SwapCoordSystem(Vector3 v)
        {
            // negate the z coord of the axis because Unity is +Z forward and the Phantom is -Z forward
            return new Vector3(v.x, v.y, -v.z);
        }

        Quaternion SwapCoordSystem(Quaternion q)
        {
            float angle;
            Vector3 axis;
            q.ToAngleAxis(out angle, out axis);
            // negate the z coord of the axis because Unity is +Z forward and the Phantom is -Z forward
            axis[2] = -axis[2];
            // negate the angle because Unity is Left-Handed and the Phantom is Right-Handed
            angle = -angle;
            return Quaternion.AngleAxis(angle, axis);
        }

        void OnVREventReceivedFromServer(VREvent vrEvent)
        {
            if (vrEvent.name == m_ServerPhantomBtnDownEventName) {
                m_PrimaryBtnDown = true;
                VREngine.instance.eventManager.QueueEvent(new VREvent(m_PhantomBtnDownEventName));

                SendTestButtonDownCommands();
            } 
            else if (vrEvent.name == m_ServerPhantomBtnUpEventName) {
                m_PrimaryBtnDown = false;
                VREngine.instance.eventManager.QueueEvent(new VREvent(m_PhantomBtnUpEventName));

                SendTestButtonUpCommands();
            }
            else if (vrEvent.name == m_ServerPhantomPositionEventName)
            {
                m_StylusPosition = SwapCoordSystem((vrEvent as VREventVector3).GetData());

                VREngine.instance.eventManager.QueueEvent(new VREventVector3(m_PhantomPositionEventName, m_StylusPosition));

                SendTestPositionMoveCommands();
            }
            else if (vrEvent.name == m_ServerPhantomRotationEventName)
            {
                m_StylusRotation = SwapCoordSystem((vrEvent as VREventQuaternion).GetData());

                VREngine.instance.eventManager.QueueEvent(new VREventQuaternion(m_PhantomRotationEventName, m_StylusRotation));
            }
        }

        public void Send(VREvent vrEvent)
        {
            m_ServerConnection.Send(vrEvent);
        }

        public List<IVREventPrototype> GetEventPrototypes()
        {
            List<IVREventPrototype> eventPrototypes = new List<IVREventPrototype>();
            eventPrototypes.Add(VREventPrototype.Create(m_PhantomBtnDownEventName));
            eventPrototypes.Add(VREventPrototype.Create(m_PhantomBtnUpEventName));
            eventPrototypes.Add(VREventPrototypeVector3.Create(m_PhantomPositionEventName));
            eventPrototypes.Add(VREventPrototypeQuaternion.Create(m_PhantomRotationEventName));
            return eventPrototypes;
        }


        [Header("Connection to ForceServer")]
        [SerializeField] private TcpJsonVREventConnection m_ServerConnection;

        [Tooltip("Name of the event the ForceServer generates on Primary Button down.")]
        [SerializeField] private string m_ServerPhantomBtnDownEventName;
        [Tooltip("Name of the event the ForceServer generates on Primary Button up.")]
        [SerializeField] private string m_ServerPhantomBtnUpEventName;
        [Tooltip("Name of the event the ForceServer generates for stylus Position updates.")]
        [SerializeField] private string m_ServerPhantomPositionEventName;
        [Tooltip("Name of the event the ForceServer generates for stylus Rotation updates.")]
        [SerializeField] private string m_ServerPhantomRotationEventName;

        [Header("VREvents Generated")]
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes down.")]
        [SerializeField] private string m_PhantomBtnDownEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes up.")]
        [SerializeField] private string m_PhantomBtnUpEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new position.")]
        [SerializeField] private string m_PhantomPositionEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new orientation.")]
        [SerializeField] private string m_PhantomRotationEventName;

        private bool m_PrimaryBtnDown;
        private Vector3 m_StylusPosition;
        private Quaternion m_StylusRotation;











        bool m_ViscosityOn = false;
        bool m_FrictionOn = false;

        void SendTestButtonDownCommands()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/BeginPoints"));
            m_ServerConnection.Send(new VREventVector3("ForceEffect/PointConstraint/AddVertex", SwapCoordSystem(m_StylusPosition)));
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/EndPoints"));
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Start"));
        }

        void SendTestButtonUpCommands()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Stop"));
        }

        void SendTestPositionMoveCommands()
        {

            // apply ambient viscosity whenever the stylus is above the Y = 0mm plane, turn off when below
            if ((m_StylusPosition[1] > 0) && (!m_ViscosityOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscous/Start"));
                m_ViscosityOn = true;
            }
            else if ((m_StylusPosition[1] < 0) && (m_ViscosityOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscous/Stop"));
                m_ViscosityOn = false;
            }

            // apply ambient friction whenever the stylus is below the Y = 0mm plane, turn off when above
            if ((m_StylusPosition[1] < 0) && (!m_FrictionOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Start"));
                m_FrictionOn = true;
            }
            else if ((m_StylusPosition[1] > 0) && (m_FrictionOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Stop"));
                m_FrictionOn = false;
            }

            // change the gain of both ambient effects so they increase left to right
            // should feel little or no effect when moving the stylus on the left and big effect on the right
            float alpha = Mathf.Clamp((m_StylusPosition[0] + 300.0f) / 600.0f, 0.0f, 1.0f);
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscous/SetGain", alpha));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetGain", alpha));
        }


        void SendTestStartCommands()
        {
            // Examples of setting a global Model-to-World transformation
            //m_ServerConnection.Send(new VREventVector3("Phantom/ModelToWorld/Translation", new Vector3(100, 0, 0)));
            // 90 deg around Y
            //m_ServerConnection.Send(new VREventQuaternion("Phantom/ModelToWorld/Rotation", new Vector3(0, 0.7071f, 0, 0.7071f)));
            // 180 deg around Y
            //m_ServerConnection.Send(new VREventQuaternion("Phantom/ModelToWorld/Rotation", new Quaternion(0, 1, 0, 0)));
            //m_ServerConnection.Send(new VREventVector3("Phantom/ModelToWorld/Scale", new Vector3(0.5f, 0.5f, 0.5f)));

            m_ViscosityOn = false;
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscous/SetGain", 0.8f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscous/SetMagnitudeCap", 1.0f));

            m_FrictionOn = false;
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetGain", 0.1f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetMagnitudeCap", 0.1f));

            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStiffness", 0.8f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDamping", 0.2f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", 0.2f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", 0.2f));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", 10.0f));

            // a set of parallel lines to demonstrate line constraints
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/BeginLines"));
            float left = -240;
            float right = -20;
            float xinc = 20;
            float top = 150;
            float bottom = -150;
            for (float x = left; x <= right; x += xinc)
            {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/LineConstraint/AddVertex", new Vector3(x, top, 0)));
                m_ServerConnection.Send(new VREventVector3("ForceEffect/LineConstraint/AddVertex", new Vector3(x, bottom, 0)));
            }
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/EndLines"));
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/Start"));


            // a simple surface to demonstrate surface constraints
            left = 50;
            right = 175;
            top = 150;
            bottom = 50;
            float back = -25;
            float front = 25;
            // signal start of mesh data
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/BeginGeometry"));
            // fill up vertex buffer
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", new Vector3(left, top, back)));      // v0
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", new Vector3(left, bottom, front)));  // v1
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", new Vector3(right, bottom, front))); // v2
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", new Vector3(right, top, back)));     // v3
                                                                                                                                       // fill up indices buffer
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddIndices", new Vector3(0, 1, 3)));  // triangle 0 = v0, v1, v3
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddIndices", new Vector3(3, 1, 2)));  // triangle 1 = v3, v1, v2
                                                                                                                            // signal end of mesh data
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/EndGeometry"));
            // start applying forces
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/Start"));

            // a simple surface to demonstrate surface contact
            left = 50;
            right = 175;
            top = -50;
            bottom = -150;
            back = -25;
            front = 25;
            // signal start of mesh data
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/BeginGeometry"));
            // fill up vertex buffer
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddVertex", new Vector3(left, top, back)));      // v0
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddVertex", new Vector3(left, bottom, front)));  // v1
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddVertex", new Vector3(right, bottom, front))); // v2
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddVertex", new Vector3(right, top, back)));     // v3
                                                                                                                                    // fill up indices buffer
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddIndices", new Vector3(0, 1, 3)));  // triangle 0 = v0, v1, v3
            m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddIndices", new Vector3(3, 1, 2)));  // triangle 1 = v3, v1, v2
                                                                                                                         // signal end of mesh data
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/EndGeometry"));
            // start applying forces
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/Start"));
        }


    }
}
