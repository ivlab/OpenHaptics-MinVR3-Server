using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{
    /// <summary>
    /// An example that uses the PointConstraintForceEffect to make the haptic
    /// stylus "follow" a moving GameObject in the scene. This script should be
    /// placed on the same GameObject as the PointConstraintForceEffect.
    /// </summary>
    [RequireComponent(typeof(PointConstraintForceEffect))]
    public class PointFollowerExample : MonoBehaviour
    {
        [Tooltip("The GameObject that the haptic constraint point will follow.")]
        public GameObject targetToFollow;

        private PointConstraintForceEffect m_PointConstraint;

        void Start()
        {
            m_PointConstraint = GetComponent<PointConstraintForceEffect>();
            if (targetToFollow == null)
            {
                Debug.LogWarning("PointFollowerExample: 'Target to Follow' is not set. The effect will not do anything.", this);
            }
            else
            {
                //m_PointConstraint.StartEffect();
            }
        }

        void Update()
        {
            if (targetToFollow != null)
            {
                // The PointConstraintForceEffect expects its points to be defined in its own local space.
                // So, we must convert the target's world position into the local space of the
                // GameObject that holds the constraint effect.
                Vector3 localPos = transform.InverseTransformPoint(targetToFollow.transform.position);
                m_PointConstraint.SetPoint(localPos);
                // TODO: investigate why this needs to be called here. 
                m_PointConstraint.StartEffect();
            }
        }
    }
}