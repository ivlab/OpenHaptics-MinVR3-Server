using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;



namespace IVLab.MinVR3
{
    public class PhantomForceClient : MonoBehaviour, IVREventProducer
    {
        /// <summary>
        /// Current state of the primary button on the Phantom device.
        /// </summary>
        public bool primaryBtnDown {
            get => m_PrimaryBtnDown;
        }

        /// <summary>
        /// Current position of the Phantom stylus in Unity's world coordinates.
        /// </summary>
        public Vector3 stylusPosition {
            get => m_StylusPosition;
        }

        /// <summary>
        /// Current rotation of the Phantom stylus in Unity's world coordinates.
        /// </summary>
        public Quaternion stylusRotation {
            get => m_StylusRotation;
        }


        // --- Coordinate System Utilities ---

        /// <summary>
        /// Converts a position from the server's raw Touch Space (Right-Handed, mm, off-center)
        /// to Unity's world space. This assumes the HapticWorkspace GameObject has a scale
        /// of (0.001, 0.001, 0.001) to handle unit conversion.
        /// </summary>
        public Vector3 TouchSpaceToUnityWorld(Vector3 touchSpacePos)
        {
            if (HapticWorkspace.Instance == null) return Vector3.zero;

            // 1. Flip the Z-axis to convert from right-handed to left-handed coordinates.
            // The position is still in the server's raw, off-center millimeter space.
            Vector3 unityLocalPos = new Vector3(touchSpacePos.x, touchSpacePos.y, -touchSpacePos.z);

            // 2. Transform from the local space of the haptic workspace to Unity's world space.
            // Unity's TransformPoint automatically applies the parent's scale (0.001),
            // converting our millimeter-based local position to a meter-based world position.
            return HapticWorkspace.Instance.transform.TransformPoint(unityLocalPos);
        }

        /// <summary>
        /// Converts a rotation from the server's Touch Space (Right-Handed) to Unity's world space.
        /// </summary>
        public Quaternion TouchSpaceToUnityWorld(Quaternion touchSpaceRot)
        {
            if (HapticWorkspace.Instance == null) return Quaternion.identity;

            // 1. Convert from right-handed to left-handed quaternion.
            Quaternion unityLocalRot = new Quaternion(-touchSpaceRot.x, touchSpaceRot.y, -touchSpaceRot.z, touchSpaceRot.w);

            // 2. Transform from the local space of the haptic workspace to Unity's world space.
            return HapticWorkspace.Instance.transform.rotation * unityLocalRot;
        }

        /// <summary>
        /// Converts a position from Unity's world space to the server's raw Touch Space
        /// (Right-Handed, mm, off-center). This assumes the HapticWorkspace GameObject has a scale
        /// of (0.001, 0.001, 0.001) to handle unit conversion.
        /// </summary>
        public Vector3 UnityWorldToTouchSpace(Vector3 unityWorldPos)
        {
            if (HapticWorkspace.Instance == null) return Vector3.zero;

            // 1. Transform from Unity world space to the local space of the haptic workspace.
            // Because the workspace has a scale of 0.001, the resulting local position will be in millimeters.
            Vector3 unityLocalPos = HapticWorkspace.Instance.transform.InverseTransformPoint(unityWorldPos);

            // 2. Convert from left-handed to right-handed coordinates by flipping Z.
            return new Vector3(unityLocalPos.x, unityLocalPos.y, -unityLocalPos.z);
        }


        // ** FORCE EFFECT: AMBIENT FRICTION **

        public bool ambientFrictionActive {
            get => m_AmbientFrictionActive;
            set {
                m_AmbientFrictionActive = value;
                if (m_AmbientFrictionActive) {
                    StartAmbientFrictionEffect();
                } else {
                    StopAmbientFrictionEffect();
                }
            }
        }

        public float ambientFrictionGain {
            get => m_AmbientFrictionGain;
            set {
                m_AmbientFrictionGain = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetGain", m_AmbientFrictionGain));
            }
        }

        public float ambientFrictionMagnitudeCap {
            get => m_AmbientFrictionMagnitudeCap;
            set {
                m_AmbientFrictionMagnitudeCap = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetMagnitudeCap", m_AmbientFrictionMagnitudeCap));
            }
        }

        public void StartAmbientFrictionEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Start"));
        }

        public void StopAmbientFrictionEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Stop"));
        }

        void AmbientFrictionInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetGain", m_AmbientFrictionGain));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetMagnitudeCap", m_AmbientFrictionMagnitudeCap));
            if (m_AmbientFrictionActive) {
                StartAmbientFrictionEffect();
            }
        }



        // ** FORCE EFFECT: AMBIENT VISCOSITY **


        public bool ambientViscosityActive {
            get => m_AmbientViscosityActive;
            set {
                m_AmbientViscosityActive = value;
                if (m_AmbientViscosityActive) {
                    StartAmbientViscosityEffect();
                } else {
                    StopAmbientViscosityEffect();
                }
            }
        }

        public float ambientViscosityGain {
            get => m_AmbientViscosityGain;
            set {
                m_AmbientViscosityGain = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscosity/SetGain", m_AmbientViscosityGain));
            }
        }

        public float ambientViscosityMagnitudeCap {
            get => m_AmbientViscosityMagnitudeCap;
            set {
                m_AmbientViscosityMagnitudeCap = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscosity/SetMagnitudeCap", m_AmbientViscosityMagnitudeCap));
            }
        }

        public void StartAmbientViscosityEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscosity/Start"));
        }

        public void StopAmbientViscosityEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscosity/Stop"));
        }

        void AmbientViscosityInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscosity/SetGain", m_AmbientViscosityGain));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscosity/SetMagnitudeCap", m_AmbientViscosityMagnitudeCap));
            if (m_AmbientViscosityActive) {
                StartAmbientViscosityEffect();
            }
        }


        // ** FORCE EFFECT: POINT CONSTRAINT **


        public bool pointConstraintActive {
            get => m_PointConstraintActive;
            set {
                m_PointConstraintActive = value;
                if (m_PointConstraintActive) {
                    StartPointConstraintEffect();
                } else {
                    StopPointConstraintEffect();
                }
            }
        }

        public float pointConstraintStiffness {
            get => m_PointConstraintStiffness;
            set {
                m_PointConstraintStiffness = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStiffness", m_PointConstraintStiffness));
            }
        }

        public float pointConstraintDamping {
            get => m_PointConstraintDamping;
            set {
                m_PointConstraintDamping = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDamping", m_PointConstraintDamping));
            }
        }

        public float pointConstraintStaticFriction {
            get => m_PointConstraintStaticFriction;
            set {
                m_PointConstraintStaticFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", m_PointConstraintStaticFriction));
            }
        }

        public float pointConstraintDynamicFriction {
            get => m_PointConstraintDynamicFriction;
            set {
                m_PointConstraintDynamicFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", m_PointConstraintDynamicFriction));
            }
        }

        public float pointConstraintSnapDistance {
            get => m_PointConstraintSnapDistance;
            set {
                m_PointConstraintSnapDistance = value;
                // Snap distance is in mm, so we must scale it from Unity meters.
                m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", m_PointConstraintSnapDistance * 1000.0f));
            }
        }

        /// <summary>
        /// Read/write access to the set of points used in the point constraint effect
        /// </summary>
        public List<Vector3> pointConstraintPoints {
            get => m_PointConstraintPoints;
            set {
                m_PointConstraintPoints = value;
                PointConstraintSendPoints();
            }
        }

        /// <summary>
        /// Shortcut to set the point constraint to use just this single point.  To use multiple points, set the
        /// list of points via the pointConstraintPoints property.
        /// </summary>
        public void SetPointConstraintPoint(Vector3 p)
        {
            m_PointConstraintPoints.Clear();
            m_PointConstraintPoints.Add(p);
            PointConstraintSendPoints();
        }

        public void StartPointConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Start"));
        }

        void PointConstraintInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStiffness", m_PointConstraintStiffness));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDamping", m_PointConstraintDamping));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetStaticFriction", m_PointConstraintStaticFriction));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetDynamicFriction", m_PointConstraintDynamicFriction));
            // Snap distance is in mm, so we must scale it from Unity meters.
            m_ServerConnection.Send(new VREventFloat("ForceEffect/PointConstraint/SetSnapDistance", m_PointConstraintSnapDistance * 1000.0f));
            if ((m_PointConstraintPoints != null) && (m_PointConstraintPoints.Count > 0)) {
                PointConstraintSendPoints();
            }
            if (m_PointConstraintActive) {
                StartPointConstraintEffect();
            }
        }

        void PointConstraintSendPoints()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/BeginPoints"));
            foreach (Vector3 p in m_PointConstraintPoints) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/PointConstraint/AddVertex", UnityWorldToTouchSpace(p)));
            }
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/EndPoints"));
        }

        public void StopPointConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Stop"));
        }



        // ** FORCE EFFECT: LINE CONSTRAINT **


        public bool lineConstraintActive {
            get => m_LineConstraintActive;
            set {
                m_LineConstraintActive = value;
                if (m_LineConstraintActive) {
                    StartLineConstraintEffect();
                } else {
                    StopLineConstraintEffect();
                }
            }
        }

        public float lineConstraintStiffness {
            get => m_LineConstraintStiffness;
            set {
                m_LineConstraintStiffness = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetStiffness", m_LineConstraintStiffness));
            }
        }

        public float lineConstraintDamping {
            get => m_LineConstraintDamping;
            set {
                m_LineConstraintDamping = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetDamping", m_LineConstraintDamping));
            }
        }

        public float lineConstraintStaticFriction {
            get => m_LineConstraintStaticFriction;
            set {
                m_LineConstraintStaticFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetStaticFriction", m_LineConstraintStaticFriction));
            }
        }

        public float lineConstraintDynamicFriction {
            get => m_LineConstraintDynamicFriction;
            set {
                m_LineConstraintDynamicFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetDynamicFriction", m_LineConstraintDynamicFriction));
            }
        }

        public float lineConstraintSnapDistance {
            get => m_LineConstraintSnapDistance;
            set {
                m_LineConstraintSnapDistance = value;
                // Snap distance is in mm, so we must scale it from Unity meters.
                m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetSnapDistance", m_LineConstraintSnapDistance * 1000.0f));
            }
        }

        /// <summary>
        /// Read/write access to the vertices used in the line constraint effect
        /// </summary>
        public List<Vector3> lineConstraintVertices {
            get => m_LineConstraintVertices;
            set {
                m_LineConstraintVertices = value;
                LineConstraintSendVertices();
            }
        }

        /// <summary>
        /// Shortcut to set the line constraint to use just a single line segment. To use multiple line segments,
        /// set the list of vertices in the lineConstraintVertices property. 
        /// </summary>
        public void SetLineConstraintLine(Vector3 beginPt, Vector3 endPt)
        {
            m_LineConstraintVertices.Clear();
            m_LineConstraintVertices.Add(beginPt);
            m_LineConstraintVertices.Add(endPt);
            LineConstraintSendVertices();
        }

        public void StartLineConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/Start"));
        }

        void LineConstraintInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetStiffness", m_LineConstraintStiffness));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetDamping", m_LineConstraintDamping));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetStaticFriction", m_LineConstraintStaticFriction));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetDynamicFriction", m_LineConstraintDynamicFriction));
            // Snap distance is in mm, so we must scale it from Unity meters.
            m_ServerConnection.Send(new VREventFloat("ForceEffect/LineConstraint/SetSnapDistance", m_LineConstraintSnapDistance * 1000.0f));
            if ((m_LineConstraintVertices != null) && (m_LineConstraintVertices.Count > 0)) {
                LineConstraintSendVertices();
            }
            if (m_LineConstraintActive) {
                StartLineConstraintEffect();
            }
        }

        void LineConstraintSendVertices()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/BeginLines"));
            foreach (Vector3 p in m_LineConstraintVertices) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/LineConstraint/AddVertex", UnityWorldToTouchSpace(p)));
            }
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/EndLines"));
        }

        public void StopLineConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/LineConstraint/Stop"));
        }



        // ** FORCE EFFECT: SURFACE CONSTRAINT **


        public bool surfaceConstraintActive {
            get => m_SurfaceConstraintActive;
            set {
                m_SurfaceConstraintActive = value;
                if (m_SurfaceConstraintActive) {
                    StartSurfaceConstraintEffect();
                } else {
                    StopSurfaceConstraintEffect();
                }
            }
        }

        public float surfaceConstraintStiffness {
            get => m_SurfaceConstraintStiffness;
            set {
                m_SurfaceConstraintStiffness = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetStiffness", m_SurfaceConstraintStiffness));
            }
        }

        public float surfaceConstraintDamping {
            get => m_SurfaceConstraintDamping;
            set {
                m_SurfaceConstraintDamping = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetDamping", m_SurfaceConstraintDamping));
            }
        }

        public float surfaceConstraintStaticFriction {
            get => m_SurfaceConstraintStaticFriction;
            set {
                m_SurfaceConstraintStaticFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetStaticFriction", m_SurfaceConstraintStaticFriction));
            }
        }

        public float surfaceConstraintDynamicFriction {
            get => m_SurfaceConstraintDynamicFriction;
            set {
                m_SurfaceConstraintDynamicFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetDynamicFriction", m_SurfaceConstraintDynamicFriction));
            }
        }

        public float surfaceConstraintSnapDistance {
            get => m_SurfaceConstraintSnapDistance;
            set {
                m_SurfaceConstraintSnapDistance = value;
                // Snap distance is in mm, so we must scale it from Unity meters.
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetSnapDistance", m_SurfaceConstraintSnapDistance * 1000.0f));
            }
        }

        /// <summary>
        /// Read access to the vertices used in the surface constraint effect; set the vertices using the
        /// SetSurfaceConstraintMesh() function.
        /// </summary>
        public List<Vector3> surfaceConstraintVertices {
            get => m_SurfaceConstraintVertices;
        }

        /// <summary>
        /// Read access to the indices used in the surface constraint effect; set the indices using the
        /// SetSurfaceConstraintMesh() function.
        /// </summary>
        public List<int> surfaceConstraintIndices {
            get => m_SurfaceConstraintIndices;
        }

        public void SetSurfaceConstraintMesh(Mesh mesh)
        {
            m_SurfaceConstraintIndices = mesh.GetIndices(0).ToList();
            mesh.GetVertices(m_SurfaceConstraintVertices);
            SurfaceConstraintSendMesh();
        }

        public void SetSurfaceConstraintMesh(List<Vector3> vertices, List<int> indices)
        {
            m_SurfaceConstraintIndices = indices;
            m_SurfaceConstraintVertices = vertices;
            SurfaceConstraintSendMesh();
        }


        public void StartSurfaceConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/Start"));
        }

        void SurfaceConstraintInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetStiffness", m_SurfaceConstraintStiffness));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetDamping", m_SurfaceConstraintDamping));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetStaticFriction", m_SurfaceConstraintStaticFriction));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetDynamicFriction", m_SurfaceConstraintDynamicFriction));
            // Snap distance is in mm, so we must scale it from Unity meters.
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceConstraint/SetSnapDistance", m_SurfaceConstraintSnapDistance * 1000.0f));
            if ((m_SurfaceConstraintIndices != null) && (m_SurfaceConstraintIndices.Count > 0)) {
                SurfaceConstraintSendMesh();
            }
            if (m_SurfaceConstraintActive) {
                StartSurfaceConstraintEffect();
            }
        }

        void SurfaceConstraintSendMesh()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/BeginGeometry"));
            foreach (Vector3 p in m_SurfaceConstraintVertices) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddVertex", UnityWorldToTouchSpace(p)));
            }
            for (int i = 0; i < m_SurfaceConstraintIndices.Count; i += 3) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceConstraint/AddIndices",
                    new Vector3(m_SurfaceConstraintIndices[i],
                                m_SurfaceConstraintIndices[i + 1],
                                m_SurfaceConstraintIndices[i + 2])));
            }

            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/EndGeometry"));
        }

        public void StopSurfaceConstraintEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceConstraint/Stop"));
        }



        // ** FORCE EFFECT: SURFACE CONTACT **


        public bool surfaceContactActive {
            get => m_SurfaceContactActive;
            set {
                m_SurfaceContactActive = value;
                if (m_SurfaceContactActive) {
                    StartSurfaceContactEffect();
                } else {
                    StopSurfaceContactEffect();
                }
            }
        }

        public float surfaceContactStiffness {
            get => m_SurfaceContactStiffness;
            set {
                m_SurfaceContactStiffness = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetStiffness", m_SurfaceContactStiffness));
            }
        }

        public float surfaceContactDamping {
            get => m_SurfaceContactDamping;
            set {
                m_SurfaceContactDamping = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetDamping", m_SurfaceContactDamping));
            }
        }

        public float surfaceContactStaticFriction {
            get => m_SurfaceContactStaticFriction;
            set {
                m_SurfaceContactStaticFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetStaticFriction", m_SurfaceContactStaticFriction));
            }
        }

        public float surfaceContactDynamicFriction {
            get => m_SurfaceContactDynamicFriction;
            set {
                m_SurfaceContactDynamicFriction = value;
                m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetDynamicFriction", m_SurfaceContactDynamicFriction));
            }
        }

        /// <summary>
        /// Read access to the vertices used in the surface Contact effect; set the vertices using the
        /// SetSurfaceContactMesh() function.
        /// </summary>
        public List<Vector3> surfaceContactVertices {
            get => m_SurfaceContactVertices;
        }

        /// <summary>
        /// Read access to the indices used in the surface Contact effect; set the indices using the
        /// SetSurfaceContactMesh() function.
        /// </summary>
        public List<int> surfaceContactIndices {
            get => m_SurfaceContactIndices;
        }

        public void SetSurfaceContactMesh(Mesh mesh)
        {
            m_SurfaceContactIndices = mesh.GetIndices(0).ToList();
            mesh.GetVertices(m_SurfaceContactVertices);
            SurfaceContactSendMesh();
        }

        public void SetSurfaceContactMesh(List<Vector3> vertices, List<int> indices)
        {
            m_SurfaceContactIndices = indices;
            m_SurfaceContactVertices = vertices;
            SurfaceContactSendMesh();
        }

        public void StartSurfaceContactEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/Start"));
        }

        void SurfaceContactInit()
        {
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetStiffness", m_SurfaceContactStiffness));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetDamping", m_SurfaceContactDamping));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetStaticFriction", m_SurfaceContactStaticFriction));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/SurfaceContact/SetDynamicFriction", m_SurfaceContactDynamicFriction));
            if ((m_SurfaceContactIndices != null) && (m_SurfaceContactIndices.Count > 0)) {
                SurfaceContactSendMesh();
            }
            if (m_SurfaceContactActive) {
                StartSurfaceContactEffect();
            }
        }

        void SurfaceContactSendMesh()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/BeginGeometry"));
            foreach (Vector3 p in m_SurfaceContactVertices) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddVertex", UnityWorldToTouchSpace(p)));
            }
            for (int i = 0; i < m_SurfaceContactIndices.Count; i += 3) {
                m_ServerConnection.Send(new VREventVector3("ForceEffect/SurfaceContact/AddIndices",
                    new Vector3(m_SurfaceContactIndices[i],
                                m_SurfaceContactIndices[i + 1],
                                m_SurfaceContactIndices[i + 2])));
            }

            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/EndGeometry"));
        }

        public void StopSurfaceContactEffect()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/SurfaceContact/Stop"));
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

        void Start()
        {
            // If a specific workspace is not assigned, try to find the active singleton instance.
            if (m_HapticWorkspace == null) {
                m_HapticWorkspace = HapticWorkspace.Instance;
            }

            if (HapticWorkspace.Instance == null)
            {
                Debug.LogError("No HapticWorkspace found in the scene. A HapticWorkspace component is required to define the coordinate system. Disabling PhantomForceClient.");
                this.enabled = false;
                return;
            }

            m_PrimaryBtnDown = false;
            m_ServerConnection.OnVREventReceived += OnVREventReceivedFromServer;

            // Stops any active force effects and resets the model-to-world matrix
            m_ServerConnection.Send(new VREvent("Phantom/Reset"));

            // Send saved parameters for all force effects and start them if they are already marked active.
            // After this initialization, incremental updates are sent only when the parameters are changed.
            AmbientFrictionInit();
            AmbientViscosityInit();
            PointConstraintInit();
            LineConstraintInit();
            SurfaceConstraintInit();
            SurfaceContactInit();
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
                m_StylusPosition = TouchSpaceToUnityWorld((vrEvent as VREventVector3).GetData());

                VREngine.instance.eventManager.QueueEvent(new VREventVector3(m_PhantomPositionEventName, m_StylusPosition));

                SendTestPositionMoveCommands();
            }
            else if (vrEvent.name == m_ServerPhantomRotationEventName)
            {
                m_StylusRotation = TouchSpaceToUnityWorld((vrEvent as VREventQuaternion).GetData());

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

        [Header("Coordinate System")]
        [Tooltip("A reference to the HapticWorkspace object that defines the coordinate system for this client. If null, it will try to find the active HapticWorkspace instance.")]
        [SerializeField] private HapticWorkspace m_HapticWorkspace;
        [Tooltip("Name of the event the ForceServer generates on Primary Button down.")]
        [SerializeField] private string m_ServerPhantomBtnDownEventName;
        [Tooltip("Name of the event the ForceServer generates on Primary Button up.")]
        [SerializeField] private string m_ServerPhantomBtnUpEventName;
        [Tooltip("Name of the event the ForceServer generates for stylus Position updates.")]
        [SerializeField] private string m_ServerPhantomPositionEventName;
        [Tooltip("Name of the event the ForceServer generates for stylus Rotation updates.")]
        [SerializeField] private string m_ServerPhantomRotationEventName;
        [Tooltip("Prefix the ForceServer uses for ForceEffect commands.")]
        [SerializeField] private string m_ServerForceEffectEventPrefix;

        [Header("VREvents Generated")]
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes down.")]
        [SerializeField] private string m_PhantomBtnDownEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus Primary button goes up.")]
        [SerializeField] private string m_PhantomBtnUpEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new position.")]
        [SerializeField] private string m_PhantomPositionEventName;
        [Tooltip("VREvent to generate when the Phantom Stylus is moved to a new orientation.")]
        [SerializeField] private string m_PhantomRotationEventName;

        [Header("Ambient Friction")]
        [SerializeField] private bool m_AmbientFrictionActive;
        [SerializeField] private float m_AmbientFrictionGain;
        [SerializeField] private float m_AmbientFrictionMagnitudeCap;

        [Header("Ambient Viscosity")]
        [SerializeField] private bool m_AmbientViscosityActive;
        [SerializeField] private float m_AmbientViscosityGain;
        [SerializeField] private float m_AmbientViscosityMagnitudeCap;

        [Header("Point(s) Constraint")]
        [SerializeField] private bool m_PointConstraintActive;
        [SerializeField] private List<Vector3> m_PointConstraintPoints;
        [SerializeField] private float m_PointConstraintStiffness;
        [SerializeField] private float m_PointConstraintDamping;
        [SerializeField] private float m_PointConstraintStaticFriction;
        [SerializeField] private float m_PointConstraintDynamicFriction;
        [SerializeField] private float m_PointConstraintSnapDistance;

        [Header("Line(s) Constraint")]
        [SerializeField] private bool m_LineConstraintActive;
        [SerializeField] private List<Vector3> m_LineConstraintVertices;
        [SerializeField] private float m_LineConstraintStiffness;
        [SerializeField] private float m_LineConstraintDamping;
        [SerializeField] private float m_LineConstraintStaticFriction;
        [SerializeField] private float m_LineConstraintDynamicFriction;
        [SerializeField] private float m_LineConstraintSnapDistance;

        [Header("Surface Constraint")]
        [SerializeField] private bool m_SurfaceConstraintActive;
        [SerializeField] private List<Vector3> m_SurfaceConstraintVertices;
        [SerializeField] private List<int> m_SurfaceConstraintIndices;
        [SerializeField] private float m_SurfaceConstraintStiffness;
        [SerializeField] private float m_SurfaceConstraintDamping;
        [SerializeField] private float m_SurfaceConstraintStaticFriction;
        [SerializeField] private float m_SurfaceConstraintDynamicFriction;
        [SerializeField] private float m_SurfaceConstraintSnapDistance;

        [Header("Surface Contact")]
        [SerializeField] private bool m_SurfaceContactActive;
        [SerializeField] private List<Vector3> m_SurfaceContactVertices;
        [SerializeField] private List<int> m_SurfaceContactIndices;
        [SerializeField] private float m_SurfaceContactStiffness;
        [SerializeField] private float m_SurfaceContactDamping;
        [SerializeField] private float m_SurfaceContactStaticFriction;
        [SerializeField] private float m_SurfaceContactDynamicFriction;


        private bool m_PrimaryBtnDown;
        private Vector3 m_StylusPosition;
        private Quaternion m_StylusRotation;








        bool m_ViscosityOn = false;
        bool m_FrictionOn = false;

        void SendTestButtonDownCommands()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/BeginPoints"));
            m_ServerConnection.Send(new VREventVector3("ForceEffect/PointConstraint/AddVertex", UnityWorldToTouchSpace(m_StylusPosition)));
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/EndPoints"));
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Start"));
        }

        void SendTestButtonUpCommands()
        {
            m_ServerConnection.Send(new VREvent("ForceEffect/PointConstraint/Stop"));
        }

        void SendTestPositionMoveCommands()
        {

            // apply ambient viscosity whenever the stylus is above the workspace's local Y=0 plane
            Vector3 localPos = HapticWorkspace.Instance.transform.InverseTransformPoint(m_StylusPosition);
            if ((localPos.y > 0) && (!m_ViscosityOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscous/Start"));
                m_ViscosityOn = true;
            }
            else if ((localPos.y <= 0) && (m_ViscosityOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientViscous/Stop"));
                m_ViscosityOn = false;
            }

            // apply ambient friction whenever the stylus is below the workspace's local Y=0 plane
            if ((localPos.y <= 0) && (!m_FrictionOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Start"));
                m_FrictionOn = true;
            }
            else if ((localPos.y > 0) && (m_FrictionOn))
            {
                m_ServerConnection.Send(new VREvent("ForceEffect/AmbientFriction/Stop"));
                m_FrictionOn = false;
            }

            // change the gain of both ambient effects so they increase left to right
            // The server's workspace is about 500mm wide.
            float alpha = Mathf.Clamp((localPos.x * METERS_TO_MM + (HapticWorkspace.Instance.Size.x / 2.0f)) / HapticWorkspace.Instance.Size.x, 0.0f, 1.0f);
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientViscous/SetGain", alpha));
            m_ServerConnection.Send(new VREventFloat("ForceEffect/AmbientFriction/SetGain", alpha));
        }


        void SendTestStartCommands()
        {

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
            // The test commands have been moved to a separate example script to keep this client script clean.
        }


    }
}
