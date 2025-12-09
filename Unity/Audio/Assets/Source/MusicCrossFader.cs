using UnityEngine;
using FMODUnity;
using FMOD.Studio;

[RequireComponent(typeof(BoxCollider))]
public class MusicCrossFader : MonoBehaviour
{
    private const string BlendParameter = "Mezcla";

    [Header("FMOD Settings")]
    public EventReference musicEvent;

    [Header("Fade Settings")]
    [Range(0f, 1f)]
    public float targetBlend = 0f;

    [Tooltip("Transition Speed")]
    public float transitionSpeed = 1.0f;

    [Header("References")]
    public EpicTrigger epicTriggerScript;

    private EventInstance musicInstance;
    private float currentBlend = 0.0f;

    private void Start()
    {
        musicInstance = RuntimeManager.CreateInstance(musicEvent);
        musicInstance.start();

        musicInstance.setParameterByName(BlendParameter, 0f);

        musicInstance.setVolume(0f);

        currentBlend = 0f;
        targetBlend = 0f;
    }

    private void Update()
    {
        if (Mathf.Abs(currentBlend - targetBlend) > 0.001f)
        {
            currentBlend = Mathf.MoveTowards(currentBlend, targetBlend, transitionSpeed * Time.deltaTime);

            musicInstance.setParameterByName(BlendParameter, currentBlend);

            musicInstance.setVolume(currentBlend);

            if (epicTriggerScript)
            {
                epicTriggerScript.SetVolume(1f - currentBlend);
            }
        }

        if (Input.GetKeyDown(KeyCode.I)) SwapToA();
        if (Input.GetKeyDown(KeyCode.O)) SwapToB();


    }

    public void SwapToA() { targetBlend = 0f; }
    public void SwapToB() { targetBlend = 1f; }

    private void OnTriggerEnter(Collider other)
    {
        SwapToB();
    }

    private void OnTriggerExit(Collider other)
    {
        SwapToA();
    }

    private void OnDestroy()
    {
        musicInstance.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        musicInstance.release();
    }
}
