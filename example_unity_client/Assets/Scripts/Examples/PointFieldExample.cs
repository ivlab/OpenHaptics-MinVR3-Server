using System.Collections;
using System.Collections.Generic;
using UnityEngine;
namespace IVLab.MinVR3
{
    /// <summary>
    /// A simple test script for the PointConstraintForceEffect.
    /// When the 'G' key is pressed, it generates a new set of random points
    /// within the haptic workspace and applies them to the constraint effect.
    /// This script can be placed on the same GameObject as the PointConstraintForceEffect.
    /// </summary>
    [RequireComponent(typeof(PointConstraintForceEffect))]
    public class PointFieldExample : MonoBehaviour
    {
        [Tooltip("The number of random points to generate.")]
        public int numberOfPoints = 50;

        [Tooltip("The key to press to generate a new set of points.")]
        public KeyCode generationKey = KeyCode.G;

        private PointConstraintForceEffect m_PointConstraint;

        void Reset()
        {
            numberOfPoints = 50;
            generationKey = KeyCode.G;
            m_PointConstraint = GetComponent<PointConstraintForceEffect>();
        }

        void Start()
        {
            m_PointConstraint = GetComponent<PointConstraintForceEffect>();
            Debug.Log($"Press the '{generationKey}' key to generate {numberOfPoints} random haptic points.");
        }

        void Update()
        {
            if (Input.GetKeyDown(generationKey))
            {
                GenerateAndSetPoints();
            }
        }

        void GenerateAndSetPoints()
        {
            if (HapticWorkspace.Instance == null)
            {
                Debug.LogError("Cannot generate points because no HapticWorkspace was found in the scene.");
                return;
            }
            List<Vector3> newPoints = new List<Vector3>();
            for (int i = 0; i < numberOfPoints; i++)
            {
                // 1. Generate a random point within the haptic workspace bounds. These bounds are defined
                //    in the server's millimeter-based "Touch Space".
                Vector3 randomTouchSpacePoint = new Vector3(
                    Random.Range(HapticWorkspace.Instance.minBounds.x, HapticWorkspace.Instance.maxBounds.x),
                    Random.Range(HapticWorkspace.Instance.minBounds.y, HapticWorkspace.Instance.maxBounds.y),
                    Random.Range(HapticWorkspace.Instance.minBounds.z, HapticWorkspace.Instance.maxBounds.z)
                );

                // 2. The PointConstraintForceEffect expects its points to be defined in its own local space.
                //    So, we must convert the point from the server's Touch Space to Unity World Space, and
                //    then from World Space to the local space of the GameObject this script is on.
                Vector3 worldPoint = m_PointConstraint.phantomForceClient.TouchSpaceToUnityWorld(randomTouchSpacePoint);
                Vector3 localPoint = transform.InverseTransformPoint(worldPoint);
                newPoints.Add(localPoint);
            }
            m_PointConstraint.SetPoints(newPoints);
            m_PointConstraint.StartEffect();
            Debug.Log($"Generated and set {numberOfPoints} new random points.");
        }
    }
}