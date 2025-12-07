using FMODUnity;
using Unity.VisualScripting;
using UnityEngine;

[RequireComponent(typeof(CharacterController))]
public class PlayerController : MonoBehaviour
{
    [Header("Configuraci�n Movimiento")]
    public float speed = 5f;
    public float sensitivity = 200f;
    public float jumpHeight = 1.5f;
    public float gravity = -9.81f;

    [Header("Referencias")]
    public new Transform camera;

    [Header("Configuraci�n FMOD")]
    public EventReference sound;
    public float interval = 0.5f;

    private float _rotationX = 0f;
    private CharacterController _controller;
    private float timerfootsteps = 0f;

    private Vector3 _velocity;
    private float _groundedTimer; 

    private Vector3 _initialPosition;
    
    void Start()
    {
        _controller = GetComponent<CharacterController>();
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
        _initialPosition = transform.position;
    }

    private void Update()
    {
        HandleLook();
        HandleMovement();
    }

    void HandleLook()
    {
        float mouseX = Input.GetAxis("Mouse X") * sensitivity * Time.deltaTime;
        float mouseY = Input.GetAxis("Mouse Y") * sensitivity * Time.deltaTime;

        _rotationX -= mouseY;
        _rotationX = Mathf.Clamp(_rotationX, -90f, 90f);

        camera.localRotation = Quaternion.Euler(_rotationX, 0f, 0f);
        transform.Rotate(Vector3.up * mouseX);
    }

    void HandleMovement()
    {
        if (_controller.isGrounded)
        {
            _groundedTimer = 0.2f; 

            if (_velocity.y < 0)
            {
                _velocity.y = -2f;
            }
        }
        else
        {
            _groundedTimer -= Time.deltaTime;
        }

        float x = Input.GetAxis("Horizontal");
        float z = Input.GetAxis("Vertical");

        Vector3 move = transform.right * x + transform.forward * z;
        _controller.Move(move * (speed * Time.deltaTime));

        if (Input.GetButtonDown("Jump") && _groundedTimer > 0)
        {
            _velocity.y = Mathf.Sqrt(jumpHeight * -2f * gravity);

            _groundedTimer = 0;
        }

        _velocity.y += gravity * Time.deltaTime;
        _controller.Move(_velocity * Time.deltaTime);

        FootSteps(move);
    }

    void FootSteps(Vector3 vectorMovimiento)
    {
        if (_controller.isGrounded && vectorMovimiento.magnitude > 0.1f)
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
    
    private void OnTriggerEnter(Collider other)
    {
        if (other.CompareTag("KillZone"))
        {
            _controller.enabled = false; 
            transform.position = _initialPosition;
            _controller.enabled = true;
            Debug.LogWarning("Player has been reset to the initial position.");
        }
    }
}