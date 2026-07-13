using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{
    [RequireComponent(typeof(MeshFilter))]
    public class SurfaceConstraintForceEffect : MonoBehaviour
    {
        void StartEffect()
        {
            m_PhantomForceClient.surfaceConstraintStiffness = 0.8f;
            m_PhantomForceClient.surfaceConstraintDamping = 0.1f;
            m_PhantomForceClient.surfaceConstraintStaticFriction = 0.2f;
            m_PhantomForceClient.surfaceConstraintDynamicFriction = 0.2f;
            m_PhantomForceClient.surfaceConstraintSnapDistance = 0.01f; // 10mm

            MeshFilter meshFilter = GetComponent<MeshFilter>();
            if (meshFilter == null || meshFilter.sharedMesh == null)
            {
                Debug.LogError("SurfaceConstraintForceEffect requires a MeshFilter with a valid mesh.", this);
                return;
            }

            // The PhantomForceClient expects mesh vertices in WORLD space. The vertices in the
            // mesh are in LOCAL space. We must transform them to world space.
            Mesh mesh = meshFilter.sharedMesh;
            List<Vector3> worldVertices = new List<Vector3>();
            foreach (var v in mesh.vertices)
            {
                worldVertices.Add(transform.TransformPoint(v));
            }
            
            // Indices do not need to be transformed.
            m_PhantomForceClient.SetSurfaceConstraintMesh(worldVertices, new List<int>(mesh.triangles));
            m_PhantomForceClient.StartSurfaceConstraintEffect();
        }

        void StopEffect()
        {
            m_PhantomForceClient.StopSurfaceConstraintEffect();
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
    }
}
