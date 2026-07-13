using IVLab.MinVR3;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LichenTouchEffect : MonoBehaviour
{
    public GameObject targetMesh;
    public GameObject rawCursor;
    public Renderer cursorRenderer;

    private PhantomForceClient hapticClient;
    private MeshCollider cachedCollider;



    void Start()
    {
        hapticClient = FindObjectOfType<PhantomForceClient>();
        if (targetMesh != null) cachedCollider = targetMesh.GetComponent<MeshCollider>();
    }

    void Update()
    {
        if (targetMesh == null || cachedCollider == null || hapticClient == null) return;

        Vector3 rayOrigin = rawCursor.transform.position;
        Vector3 rayDirection = rawCursor.transform.forward;

        Ray ray = new Ray(rayOrigin, rayDirection);
        RaycastHit hit;

        if (Physics.Raycast(ray, out hit, 5.0f, Physics.DefaultRaycastLayers, QueryTriggerInteraction.Ignore))
        {
            if (hit.collider == cachedCollider)
            {
                // 1. Color Picker Pass
                Color pickedColor = GetColorFromHit(hit);
                if (cursorRenderer != null) cursorRenderer.material.color = pickedColor;


                transform.position = hit.point;
                Debug.DrawRay(hit.point, hit.normal * 0.4f, Color.yellow);
            }
        }

    }

    private Color GetColorFromHit(RaycastHit hit)
    {
        Renderer rend = hit.transform.GetComponent<Renderer>();
        if (rend == null || rend.material == null || rend.material.mainTexture == null) return Color.white;
        Texture2D tex = rend.material.mainTexture as Texture2D;
        if (tex == null) return Color.white;
        Vector2 pixelUV = hit.textureCoord;
        pixelUV.x *= tex.width; pixelUV.y *= tex.height;
        return tex.GetPixel((int)pixelUV.x, (int)pixelUV.y);
    }
}