using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{
    /// <summary>
    /// Defines the coordinate system and physical bounds of the haptic workspace within the Unity scene.
    /// There should be exactly one active HapticWorkspace in the scene.
    /// This GameObject's transform (position, rotation, scale) maps the server's raw "Touch Space"
    /// into Unity's world space.
    /// </summary>
    [ExecuteInEditMode]
    public class HapticWorkspace : MonoBehaviour
    {
        public enum DeviceType { Custom, PhantomPremium15, PhantomOmni }

        public static HapticWorkspace Instance { get; private set; }

        [Tooltip("Select a preset for the haptic device being used. This will update the Min/Max Bounds.")]
        public DeviceType device = DeviceType.PhantomPremium15;

        [Tooltip("The minimum bounds of the haptic workspace in millimeters (raw device coordinates).")]
        public Vector3 minBounds;
        [Tooltip("The maximum bounds of the haptic workspace in millimeters (raw device coordinates).")]
        public Vector3 maxBounds;

        [Tooltip("The color of the workspace gizmo.")]
        public Color gizmoColor = new Color(0.5f, 0.5f, 1.0f, 0.75f);

        // --- Public Properties ---

        public Vector3 Size => maxBounds - minBounds;
        public Vector3 Center => minBounds + Size / 2.0f;

        // --- Unity Methods ---

        private void OnEnable()
        {
            if (Instance != null && Instance != this)
            {
                Debug.LogError("More than one active HapticWorkspace in the scene. This may lead to unexpected behavior as only one can be the active instance.");
            }
            // The last HapticWorkspace to be enabled becomes the active instance.
            Instance = this;
        }

        private void OnDisable()
        {
            if (Instance == this)
            {
                Instance = null;
            }
        }

        private void OnValidate()
        {
            // Update bounds based on preset
            if (device == DeviceType.PhantomPremium15)
            {
                minBounds = new Vector3(-250, -95, -130);
                maxBounds = new Vector3(250, 230, 130);
            }
            else if (device == DeviceType.PhantomOmni)
            {
                // NOTE: These are typical values, may need adjustment.
                minBounds = new Vector3(-80, -60, -50);
                maxBounds = new Vector3(80, 70, 50);
            }
        }

        // This function is called when the script is first added to a GameObject or when the user clicks the "Reset" command.
        private void Reset()
        {
            // Set a sensible default scale for meters-to-millimeters conversion.
            transform.localScale = new Vector3(0.001f, 0.001f, 0.001f);
        }

        private void OnDrawGizmos()
        {
            // The gizmo cube should be centered relative to the device's raw coordinate system.
            // We draw the gizmo in the local space of this GameObject. Since the scale is 0.001,
            // our millimeter-sized gizmo will appear correctly scaled in the Unity scene.
            Gizmos.matrix = transform.localToWorldMatrix;
            Gizmos.color = gizmoColor;

            // The gizmo should be drawn centered around the device's raw center point.
            Gizmos.DrawWireCube(Center, Size);
        }
    }
}
