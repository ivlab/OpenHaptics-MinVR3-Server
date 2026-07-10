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

            // The PhantomForceClient expects points in WORLD space. The points in our
            // m_Points list are in the LOCAL space of this GameObject. We must
            // transform them to world space before sending them to the client.
            List<Vector3> worldPoints = new List<Vector3>();
            foreach (var p in m_Points) {
                worldPoints.Add(transform.TransformPoint(p));
            }
            m_PhantomForceClient.pointConstraintPoints = worldPoints;

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
