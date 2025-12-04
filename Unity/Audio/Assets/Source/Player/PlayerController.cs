using UnityEngine;

public class PlayerController : MonoBehaviour
{
    [Header("Conf")]
    public float speed = 5f;
    public float mousesens = 200f;
    public Transform camera;


    private float Xrot = 0f;
    private CharacterController controller;

    void Start()
    {
        controller = GetComponent<CharacterController>();

        Cursor.lockState = CursorLockMode.Locked;
    }

    void Update()
    {
        float mouseX = Input.GetAxis("Mouse X") * mousesens * Time.deltaTime;
        float mouseY = Input.GetAxis("Mouse Y") * mousesens * Time.deltaTime;

        Xrot -= mouseY;
        Xrot = Mathf.Clamp(Xrot, -90f, 90f);

        camera.localRotation = Quaternion.Euler(Xrot, 0f, 0f);

        transform.Rotate(Vector3.up * mouseX);


        float x = Input.GetAxis("Horizontal"); 
        float z = Input.GetAxis("Vertical");   

        Vector3 mover = transform.right * x + transform.forward * z;

        controller.Move(mover * speed * Time.deltaTime);

        controller.Move(Physics.gravity * Time.deltaTime);
    }
}
