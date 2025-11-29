using UnityEngine;
using FMODUnity;
using FMOD.Studio;

public class MusicCrossFader : MonoBehaviour
{
    private const string BlendParameter = "Blend";

    [Header("FMOD")] public EventReference musicEvent;

    [Header("Fade blend")] [Range(0f, 1f)] public float targetBlend = 0f; // 0 = Song A, 1 = Song B

    [Tooltip("How fast the fade's blend goes")]
    public float transitionSpeed = 2.0f;

    public void SwapToA()
    {
        targetBlend = 0f;
    }

    public void SwapToB()
    {
        targetBlend = 1f;
    }

    private EventInstance musicInstance;
    private float currentBlend = 0.0f; // 0 = Song A, 1 = Song B

    private void Start()
    {
        // Create and initialize music's instance
        musicInstance = RuntimeManager.CreateInstance(musicEvent);
        musicInstance.start();

        // Initialize blend parameter to 0 (Song A)
        musicInstance.setParameterByName(BlendParameter, 0f);
    }

    private void Update()
    {
        // Lerp blend
        if (Mathf.Abs(currentBlend - targetBlend) > 0.001f)
        {
            currentBlend = Mathf.MoveTowards(currentBlend, targetBlend, transitionSpeed * Time.deltaTime);

            // Set FMOD parameter
            musicInstance.setParameterByName(BlendParameter, currentBlend);
        }

        if (Input.GetKeyDown(KeyCode.A)) SwapToA();
        if (Input.GetKeyDown(KeyCode.B)) SwapToB();
    }

    private void OnDestroy()
    {
        musicInstance.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        musicInstance.release();
    }
}