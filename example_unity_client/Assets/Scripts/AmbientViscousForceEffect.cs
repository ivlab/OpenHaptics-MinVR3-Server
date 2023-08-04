using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace IVLab.MinVR3
{

    public class AmbientViscousForceEffect : MonoBehaviour
    {
        void StartEffect()
        {
            SetGain(m_Gain);
            SetMagnitudeCap(m_MagnitudeCap);
            m_PhantomForceClient.Send(new VREvent("ForceEffect/AmbientViscous/Start"));
        }

        void StopEffect()
        {
            m_PhantomForceClient.Send(new VREvent("ForceEffect/AmbientViscous/Start"));
        }

        void SetGain(float gain)
        {
            m_Gain = gain;
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/AmbientViscous/SetGain", m_Gain));
        }

        void SetMagnitudeCap(float cap)
        {
            m_MagnitudeCap = cap;
            m_PhantomForceClient.Send(new VREventFloat("ForceEffect/AmbientViscous/SetMagnitudeCap", m_MagnitudeCap));
        }

        void Start()
        {
            Debug.Assert(m_PhantomForceClient != null);
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

        [SerializeField] float m_Gain;
        [SerializeField] float m_MagnitudeCap;
    }

}
