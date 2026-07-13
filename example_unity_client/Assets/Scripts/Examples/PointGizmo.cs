using System.Collections;
using System.Collections.Generic;
using UnityEngine;
namespace IVLab.MinVR3
{
    [RequireComponent(typeof(PointConstraintForceEffect))]
    public class PointGizmo : MonoBehaviour
    {
        public Color gizmoColor = Color.yellow;
        public float gizmoRadius = 0.01f; // 1 cm

        private void OnDrawGizmos()
        {
            PointConstraintForceEffect pointConstraint = GetComponent<PointConstraintForceEffect>();
            Gizmos.color = gizmoColor;
            foreach (var p in pointConstraint.points)
            {
                Gizmos.DrawSphere(transform.TransformPoint(p), gizmoRadius);
            }
        }
    }
}