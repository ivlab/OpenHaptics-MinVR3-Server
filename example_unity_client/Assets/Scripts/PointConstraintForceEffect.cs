using System.Collections;
using System.Collections.Generic;
using UnityEngine;


namespace IVLab.MinVR3
{

    public class PointConstraintForceEffect : MonoBehaviour
    {
        public List<Vector3> points {
            get => m_Points;
            set {
                m_Points = value;
                // If the effect is active, resend the parameters to the server.
                if (Application.isPlaying && isActiveAndEnabled)
                {
                    UpdateEffect();
                }
            }
        }

        public float stiffness {
            get => m_Stiffness;
            set {
                m_Stiffness = value;
                if (Application.isPlaying && isActiveAndEnabled) {
                    UpdateEffect();
                }
            }
        }

        public float damping {
            get => m_Damping;
            set {
                m_Damping = value;
                if (Application.isPlaying && isActiveAndEnabled) {
                    UpdateEffect();
                }
            }
        }

        public float staticFriction {
            get => m_StaticFriction;
            set {
                m_StaticFriction = value;
                if (Application.isPlaying && isActiveAndEnabled) {
                    UpdateEffect();
                }
            }
        }

        public float dynamicFriction {
            get => m_DynamicFriction;
            set {
                m_DynamicFriction = value;
                if (Application.isPlaying && isActiveAndEnabled) {
                    UpdateEffect();
                }
            }
        }

        public float snapDistance {
            get => m_SnapDistance;
            set {
                m_SnapDistance = value;
                if (Application.isPlaying && isActiveAndEnabled) {
                    UpdateEffect();
                }
            }
        }





        public void SetPoint(Vector3 p)
        {
            m_Points.Clear();
            m_Points.Add(p);
            // If the effect is active, resend the parameters to the server.
            if (Application.isPlaying && isActiveAndEnabled)
            {
                UpdateEffect();
            }
        }

        public void SetPoints(List<Vector3> points)
        {
            m_Points = points;
            // If the effect is active, resend the parameters to the server.
            if (Application.isPlaying && isActiveAndEnabled)
            {
                UpdateEffect();
            }
        }

        public void StartEffect()
        {
            UpdateEffect(); // Send all current parameters and points
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/Start"));
        }

        public void StopEffect()
        {
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/Stop"));
        }

        void Start()
        {
            Debug.Assert(m_PhantomForceClient != null);
            StartEffect();
        }


        // This function is called in the editor when a script is loaded or a value is changed in the Inspector.
        private void OnValidate()
        {
            // If the application is playing and this component is active, it means the effect is
            // currently running. In this case, we should immediately send any updated parameters
            // to the server to reflect the changes in the Inspector.
            if (Application.isPlaying && isActiveAndEnabled)
            {   
                UpdateEffect();
            }
        }

        private void UpdateEffect()
        {
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetStiffness", m_Stiffness));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetDamping", m_Damping));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", m_StaticFriction));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", m_DynamicFriction));
            // The server expects snap distance in millimeters.
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", m_SnapDistance * 1000.0f));
            
            if (m_Points != null) { // Ensure points list is not null before sending
                SendPointsToServer();
            }
        }

        public PhantomForceClient phantomForceClient {
            get => m_PhantomForceClient;
            set => m_PhantomForceClient = value;
        }

        private void SendPointsToServer()
        {
            // The server expects points in its own "Touch Space". The points in our m_Points
            // list are in the LOCAL space of this GameObject. We must transform them to
            // world space and then convert them to touch space before sending.
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/BeginPoints"));
            foreach (var p in m_Points) {
                Vector3 worldPoint = transform.TransformPoint(p);
                Vector3 touchSpacePoint = m_PhantomForceClient.UnityWorldToTouchSpace(worldPoint);
                m_PhantomForceClient.Send(new VREventVector3("ForceEffect/PointConstraint/AddVertex", touchSpacePoint));
            }
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/EndPoints"));
        }

        [Tooltip("Client object that manages the network connection to the ForceServer.")]
        [SerializeField] PhantomForceClient m_PhantomForceClient;

        // This function is called when the script is first added to a GameObject or when the user clicks the "Reset" command.
        void Reset()
        {
            m_Stiffness = 0.8f;
            m_Damping = 0.2f;
            m_StaticFriction = 0.2f;
            m_DynamicFriction = 0.2f;
            m_SnapDistance = 0.01f;
            m_Points = new List<Vector3>();
        }

        [Header("Effect Properties")]
        [Tooltip("Stiffness of the constraint (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_Stiffness;

        [Tooltip("Damping of the constraint (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_Damping;

        [Tooltip("Static friction of the constraint (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_StaticFriction;

        [Tooltip("Dynamic friction of the constraint (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_DynamicFriction;

        [Tooltip("The distance in meters from a point at which the stylus will snap to it.")]
        [SerializeField] private float m_SnapDistance;

        [Header("Geometry")]
        [Tooltip("The list of points (in local space) to which the stylus will be constrained.")]
        [SerializeField] private List<Vector3> m_Points;
    }

}
