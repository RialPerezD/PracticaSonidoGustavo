using FMODUnity;
using UnityEngine;
using UnityEngine.InputSystem.XR;

[RequireComponent(typeof(CharacterController))]
public class PlayerController : MonoBehaviour
{
    [Header("Configuration")] public float speed = 3f;
    public float sensitivity = 200f;
    public new Transform camera;

    [Header("Conf FMOD")]
    public EventReference sound;
    public float interval = 0.75f;

    private float _rotationX = 0f;
    private CharacterController _controller;
    private float timerfootsteps = 0f;

    void Start()
    {
        _controller = GetComponent<CharacterController>();

        Cursor.lockState = CursorLockMode.Locked;
    }

    private void Update()
    {
        float mouseX = Input.GetAxis("Mouse X") * sensitivity * Time.deltaTime;
        float mouseY = Input.GetAxis("Mouse Y") * sensitivity * Time.deltaTime;

        _rotationX -= mouseY;
        _rotationX = Mathf.Clamp(_rotationX, -90f, 90f);

        camera.localRotation = Quaternion.Euler(_rotationX, 0f, 0f);

        transform.Rotate(Vector3.up * mouseX);


        float x = Input.GetAxis("Horizontal");
        float z = Input.GetAxis("Vertical");

        Vector3 mover = transform.right * x + transform.forward * z;

        _controller.Move(mover * (speed * Time.deltaTime));

        _controller.Move(Physics.gravity * Time.deltaTime);

        FootSteps(mover);
    }

    void FootSteps(Vector3 vectorMovimiento)
    {

        if (_controller.isGrounded && vectorMovimiento.magnitude > 0)
        {
            timerfootsteps -= Time.deltaTime;

            if (timerfootsteps <= 0)
            {
                PlaySound();
                timerfootsteps = interval;
            }
        }
        else
        {
            timerfootsteps = 0;
        }
    }

    void PlaySound()
    {

        if (!sound.IsNull)
        {
            RuntimeManager.PlayOneShot(sound, transform.position);
        }
    }

}