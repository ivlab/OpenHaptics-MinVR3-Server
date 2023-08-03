using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using UnityEngine;

namespace IVLab.MinVR3
{

    public class PhantomForceClient : MonoBehaviour, IVREventProducer
    {
        void Reset()
        {
            m_PhantomBtnDownEventNameFromServer = "Phantom/Primary DOWN";
            m_PhantomBtnUpEventNameFromServer = "Phantom/Primary UP";
            m_PhantomPositionEventNameFromServer = "Phantom/Position";
            m_PhantomRotationEventNameFromServer = "Phantom/Rotation";

            m_PhantomBtnDownEvent = VREventPrototypeAny.Create("Phantom/PrimaryBtn/Down");
            m_PhantomBtnUpEvent = VREventPrototypeAny.Create("Phantom/PrimaryBtn/Up");
            m_PhantomPositionEvent = VREventPrototypeAny.Create<Vector3>("Phantom/Position");
            m_PhantomRotationEvent = VREventPrototypeAny.Create<Quaternion>("Phantom/Rotation");
            SetEditorModeForEventPrototypes();
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

        Vector3 SwapCoordSystem(Vector3 v)
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
            if (vrEvent.name == m_PhantomBtnDownEventNameFromServer) {
                m_PrimaryBtnDown = true;
                VREngine.instance.eventManager.QueueEvent(new VREvent(m_PhantomBtnDownEvent.GetEventName()));

                SendTestButtonDownCommands();
            } 
            else if (vrEvent.name == m_PhantomBtnUpEventNameFromServer) {
                m_PrimaryBtnDown = false;
                VREngine.instance.eventManager.QueueEvent(new VREvent(m_PhantomBtnUpEvent.GetEventName()));

                SendTestButtonUpCommands();
            }
            else if (vrEvent.name == m_PhantomPositionEventNameFromServer)
            {
                m_StylusPosition = SwapCoordSystem((vrEvent as VREventVector3).GetData());

                VREngine.instance.eventManager.QueueEvent(new VREventVector3(m_PhantomPositionEvent.GetEventName(), m_StylusPosition));

                SendTestPositionMoveCommands();
            }
            else if (vrEvent.name == m_PhantomRotationEventNameFromServer)
            {
                m_StylusRotation = SwapCoordSystem((vrEvent as VREventQuaternion).GetData());

                VREngine.instance.eventManager.QueueEvent(new VREventQuaternion(m_PhantomRotationEvent.GetEventName(), m_StylusRotation));
            }
        }

        public List<IVREventPrototype> GetEventPrototypes()
        {
            List<IVREventPrototype> eventPrototypes = new List<IVREventPrototype>();
            eventPrototypes.Add(m_PhantomBtnDownEvent);
            eventPrototypes.Add(m_PhantomBtnUpEvent);
            eventPrototypes.Add(m_PhantomPositionEvent);
            eventPrototypes.Add(m_PhantomRotationEvent);
            return eventPrototypes;
        }

        void OnValidate()
        {
            // TODO: After addressing the TODO below, these lines can be removed.  These just prevent
            // any accidental bugs by changing the datatype to one that does not make sense.
            m_PhantomBtnUpEvent.SetEventDataType("");
            m_PhantomBtnDownEvent.SetEventDataType("");
            m_PhantomPositionEvent.SetEventDataType(typeof(Vector3));
            m_PhantomRotationEvent.SetEventDataType(typeof(Quaternion));

            SetEditorModeForEventPrototypes();
        }

        void SetEditorModeForEventPrototypes()
        {
            // VREventPrototypes support two different editor modes.  Usually, we want to provide
            // a dropdown list in the editor so programmers can select an event prototype from a
            // list of known prototypes provided by the event manager.  This is the other case.  In
            // a virtual input device like this, we want the editor interface to make it possible
            // for us to define a new name for an event.

            // TODO: In this case, the data types will not change, so it would be better to use
            // specific VREventPrototypes rather than VREventPrototypeAny.  However, the specific
            // versions do not yet have an editor property drawer that supports this "define in
            // editor mode".
            m_PhantomBtnDownEvent.SetDefineNewPrototypeInEditor(true);
            m_PhantomBtnUpEvent.SetDefineNewPrototypeInEditor(true);
            m_PhantomPositionEvent.SetDefineNewPrototypeInEditor(true);
            m_PhantomRotationEvent.SetDefineNewPrototypeInEditor(true);
        }

        [Header("Connection to ForceServer")]
        [SerializeField] private TcpJsonVREventConnection m_ServerConnection;

        [SerializeField] private string m_PhantomBtnDownEventNameFromServer;
        [SerializeField] private string m_PhantomBtnUpEventNameFromServer;
        [SerializeField] private string m_PhantomPositionEventNameFromServer;
        [SerializeField] private string m_PhantomRotationEventNameFromServer;

        [Header("VREvents Generated")]
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes down.")]
        [SerializeField] private VREventPrototypeAny m_PhantomBtnDownEvent;
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes up.")]
        [SerializeField] private VREventPrototypeAny m_PhantomBtnUpEvent;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new position.")]
        [SerializeField] private VREventPrototypeAny m_PhantomPositionEvent;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new orientation.")]
        [SerializeField] private VREventPrototypeAny m_PhantomRotationEvent;

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
