using System.Collections;
using System.Collections.Generic;
using UnityEngine;


namespace IVLab.MinVR3
{

    public class PointConstraintForceEffect : MonoBehaviour
    {
        public List<Vector3> points {
            get => m_Points;
            set => m_Points = value;
        }

        public void SetPoint(Vector3 p)
        {
            m_Points.Clear();
            m_Points.Add(p);
        }

        public void SetPoints(List<Vector3> points)
        {
            m_Points = points;
        }

        void StartEffect()
        {
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetStiffness", 0.8f));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetDamping", 0.2f));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", 0.2f));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", 0.2f));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", 10.0f));

            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/BeginPoints"));
            foreach (Vector3 p in m_Points) {
                m_PhantomForceClient.Send(new VREventVector3("ForceEffect/PointConstraint/AddVertex",
                    m_PhantomForceClient.SwapCoordSystem(p)));
            }
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/EndPoints"));

            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/Start"));
        }

        void StopEffect()
        {
            m_PhantomForceClient.Send(new VREvent("ForceEffect/PointConstraint/Stop"));
        }

        void Start()
        {
            Debug.Assert(m_PhantomForceClient != null);
        }

        private void OnEnable()
        {
            StartEffect();
        }

        private void OnDisable()
        {
            StopEffect();
        }

        [Tooltip("Client object that manages the network connection to the ForceServer.")]
        [SerializeField] PhantomForceClient m_PhantomForceClient;

        [SerializeField] List<Vector3> m_Points;
    }

}
