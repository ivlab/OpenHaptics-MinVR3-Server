using UnityEngine;

namespace IVLab.MinVR3
{
    /// <summary>
    /// Implements the "Remote Surface Haptic Rendering" effect.
    /// This script performs a raycast from the haptic stylus proxy. If it hits an object
    /// on the specified layer, it sends the contact point and surface normal to the server.
    /// The server then uses this data to simulate an infinite tangent plane at the point of contact.
    /// </summary>
    public class TangentPlaneForceEffect : MonoBehaviour
    {
        [Tooltip("Client object that manages the network connection to the ForceServer.")]
        [SerializeField] private PhantomForceClient m_PhantomForceClient;

        [Tooltip("The layer(s) that the haptic device can interact with.")]
        [SerializeField] private LayerMask m_HapticallyRenderedLayer;

        [Header("Force Properties")]
        [Tooltip("Stiffness of the virtual surface (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_Stiffness = 0.8f;

        [Tooltip("Damping of the virtual surface (0-1).")]
        [Range(0, 1)]
        [SerializeField] private float m_Damping = 0.1f;

        [Header("Raycast Properties")]
        [Tooltip("The maximum distance for the raycast to find a surface.")]
        [SerializeField] private float m_MaxRaycastDistance = 1.0f; // 1 meter

        [Tooltip("How far 'behind' the stylus to start the raycast to ensure it hits surfaces even if slightly penetrated.")]
        [SerializeField] private float m_RaycastLookBehind = 0.1f; // 10 cm

        private bool m_IsActive = false;

        void Start()
        {
            if (m_PhantomForceClient == null)
            {
                Debug.LogError("PhantomForceClient must be assigned.", this);
                this.enabled = false;
            }
        }

        private void OnEnable()
        {
            StartEffect();
        }

        private void OnDisable()
        {
            StopEffect();
        }

        void StartEffect()
        {
            m_PhantomForceClient.Send(new VREvent("ForceEffect/TangentPlaneConstraint/Start"));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/TangentPlaneConstraint/SetStiffness", m_Stiffness));
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/TangentPlaneConstraint/SetDamping", m_Damping));
            m_IsActive = true;
        }

        void StopEffect()
        {
            m_PhantomForceClient.Send(new VREvent("ForceEffect/TangentPlaneConstraint/Stop"));
            m_IsActive = false;
        }

        void LateUpdate()
        {
            if (!m_IsActive) return;

            // The ray should originate slightly behind the stylus and point in its forward direction.
            Vector3 rayOrigin = transform.position - transform.forward * m_RaycastLookBehind;

            if (Physics.Raycast(rayOrigin, transform.forward, out RaycastHit hit, m_MaxRaycastDistance, m_HapticallyRenderedLayer))
            {
                // We have a hit! Send the contact point and normal to the server.
                // The PhantomForceClient will handle the coordinate space conversion.
                Vector3 contactPoint = hit.point;
                Vector3 surfaceNormal = hit.normal;

                VREvent e = new VREvent("ForceEffect/TangentPlaneConstraint/SetContactData");

                // The server expects separate float values for each component.
                Vector3 touchSpacePoint = m_PhantomForceClient.UnityWorldToTouchSpace(contactPoint);
                e.AddData("contactPointX", touchSpacePoint.x);
                e.AddData("contactPointY", touchSpacePoint.y);
                e.AddData("contactPointZ", touchSpacePoint.z);

                // Normals also need to be transformed. We can reuse the position conversion logic
                // as it correctly handles the handedness flip.
                Vector3 touchSpaceNormal = m_PhantomForceClient.UnityWorldToTouchSpace(surfaceNormal) - m_PhantomForceClient.UnityWorldToTouchSpace(Vector3.zero);
                e.AddData("surfaceNormalX", touchSpaceNormal.x);
                e.AddData("surfaceNormalY", touchSpaceNormal.y);
                e.AddData("surfaceNormalZ", touchSpaceNormal.z);

                m_PhantomForceClient.Send(e);
            }
        }
    }
}