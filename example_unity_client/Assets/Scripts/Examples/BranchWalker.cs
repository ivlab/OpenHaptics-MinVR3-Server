using UnityEngine;

public class BranchWalker : MonoBehaviour
{
    [Header("Target Settings")]
    [Tooltip("The branch mesh/collider transform we want to snap to.")]
    public Transform branchTransform;

    [Header("Movement Settings")]
    [Tooltip("How slow the player walks forward while holding Space.")]
    public float forwardSpeed = 0.01f;

    [Tooltip("How high above the branch surface the player should float.")]
    public float heightAboveBranch = 0.1f;

    [Tooltip("Layer mask for the branch to ensure we only hit the branch.")]
    public LayerMask branchLayer;

    [Tooltip("How far down the script will look for the branch.")]
    public float maxRaycastDistance = 1f;

    [Tooltip("How high above the player to start the downward raycast (prevents clipping inside).")]
    public float raycastHeightOffset = 0.2f;

    void GoToGround()
    {
        // 2. Start the raycast from safely ABOVE the tentative position to prevent clipping inside the mesh
        Vector3 rayOrigin = transform.position + (Vector3.up * raycastHeightOffset);
        Vector3 rayDirection = Vector3.down;

        // 3. Snap to the surface below that new position
        if (Physics.Raycast(rayOrigin, rayDirection, out RaycastHit hit, maxRaycastDistance, branchLayer))
        {
            // Snap position to the surface + height offset
            transform.position = hit.point + (hit.normal * heightAboveBranch);
            Vector3 newForward = Vector3.Cross(hit.normal, transform.right).normalized;
            transform.rotation = Quaternion.LookRotation(newForward, hit.normal);
        }
        else
        {
            Debug.Log("Off the branch.");
        }
    }

    void Start()
    {
        //GoToGround();
    }

    void Update()
    {
        if (branchTransform == null) return;

        // 1. Calculate tentative forward movement if Space is held
        Vector3 targetPosition = transform.position;
        Debug.Log(transform.forward * forwardSpeed);
        if (Input.GetKeyDown(KeyCode.Space))
        {
            targetPosition += transform.forward * forwardSpeed;
            transform.position = targetPosition;


            //GoToGround();
        }

   
    }

    // Draws a visual aid in the Scene view
    void OnDrawGizmos()
    {
        // Red ray showing the downward calculation
        Gizmos.color = Color.red;
        Vector3 rayOrigin = transform.position + (Vector3.up * raycastHeightOffset);
        Gizmos.DrawRay(rayOrigin, Vector3.down * maxRaycastDistance);
    }
}