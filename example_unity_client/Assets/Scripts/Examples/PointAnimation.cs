using System.Collections;
using System.Collections.Generic;
using UnityEngine;
namespace IVLab.MinVR3
{
    /// <summary>
    /// A simple script that animates a GameObject in a circular path on the XZ plane.
    /// </summary>
    public class PointAnimation : MonoBehaviour
    {
        [Tooltip("The radius of the circular animation path in meters.")]
        [SerializeField] private float m_Radius;

        [Tooltip("The speed of the animation in radians per second.")]
        [SerializeField] private float m_Speed;

        private Vector3 m_StartPosition;

        // This function is called when the script is first added to a GameObject or when the user clicks the "Reset" command.
        private void Reset()
        {
            m_Radius = 0.03f; // 3 cm
            m_Speed = 2.0f;
        }

        void Start()
        {
            // Store the initial position of the GameObject when the scene starts.
            m_StartPosition = transform.position;
        }

        void Update()
        {
            // Calculate the offset from the start position in a circle on the XZ plane.
            transform.position = m_StartPosition + new Vector3(Mathf.Cos(Time.time * m_Speed) * m_Radius, 0, Mathf.Sin(Time.time * m_Speed) * m_Radius);
        }
    }
}
