using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{
    [RequireComponent(typeof(MeshFilter))]
    public class SurfaceContactForceEffect : MonoBehaviour
    {
        void StartEffect()
        {
            m_PhantomForceClient.surfaceContactStiffness = 0.8f;
            m_PhantomForceClient.surfaceContactDamping = 0.1f;
            m_PhantomForceClient.surfaceContactStaticFriction = 0.2f;
            m_PhantomForceClient.surfaceContactDynamicFriction = 0.2f;

            MeshFilter meshFilter = GetComponent<MeshFilter>();
            if (meshFilter == null || meshFilter.sharedMesh == null)
            {
                Debug.LogError("SurfaceContactForceEffect requires a MeshFilter with a valid mesh.", this);
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
            m_PhantomForceClient.SetSurfaceContactMesh(worldVertices, new List<int>(mesh.triangles));
            m_PhantomForceClient.StartSurfaceContactEffect();
        }

        void StopEffect()
        {
            m_PhantomForceClient.StopSurfaceContactEffect();
        }

        void Start()
        {
            Debug.Assert(m_PhantomForceClient != null, "PhantomForceClient must be assigned.");
            StartEffect();
        }

        [Tooltip("Client object that manages the network connection to the ForceServer.")]
        [SerializeField] PhantomForceClient m_PhantomForceClient;
    }
}
