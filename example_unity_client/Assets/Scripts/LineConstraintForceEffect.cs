using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{
    public class LineConstraintForceEffect : MonoBehaviour
    {
        public List<Vector3> vertices {
            get => m_Vertices;
            set => m_Vertices = value;
        }

        void StartEffect()
        {
            m_PhantomForceClient.lineConstraintStiffness = 0.8f;
            m_PhantomForceClient.lineConstraintDamping = 0.2f;
            m_PhantomForceClient.lineConstraintStaticFriction = 0.2f;
            m_PhantomForceClient.lineConstraintDynamicFriction = 0.2f;
            m_PhantomForceClient.lineConstraintSnapDistance = 0.01f; // 10mm

            // The PhantomForceClient expects points in WORLD space. The points in our
            // m_Vertices list are in the LOCAL space of this GameObject. We must
            // transform them to world space before sending them to the client.
            List<Vector3> worldVertices = new List<Vector3>();
            foreach (var v in m_Vertices)
            {
                worldVertices.Add(transform.TransformPoint(v));
            }
            m_PhantomForceClient.lineConstraintVertices = worldVertices;

            m_PhantomForceClient.StartLineConstraintEffect();
        }

        void StopEffect()
        {
            m_PhantomForceClient.StopLineConstraintEffect();
        }

        void Start()
        {
            Debug.Assert(m_PhantomForceClient != null, "PhantomForceClient must be assigned.");
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

        [Tooltip("Vertices defining the line segments in the local space of this GameObject.")]
        [SerializeField] List<Vector3> m_Vertices;
    }
}
