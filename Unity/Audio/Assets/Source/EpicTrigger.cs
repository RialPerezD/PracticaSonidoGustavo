using FMOD.Studio;
using FMODUnity;
using UnityEngine;

[RequireComponent(typeof(BoxCollider))]
public class EpicTrigger : MonoBehaviour
{
    [Header("FMOD")] public EventReference musicEvent;

    [Header("FMOD Settings")]
    public string globalParamName = "AMB_DUCK";
    private EventInstance _musicInstance;
    
    private const string StateParameter = "LitvarState";
    private const string StateSad = "Sad";
    private const string StateEpic = "Epic";

    private void Start()
    {
        _musicInstance = RuntimeManager.CreateInstance(musicEvent);
        _musicInstance.start();
    }
    
    private void OnDestroy()
    {
        _musicInstance.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        _musicInstance.release();
    }

    private void OnTriggerEnter(Collider other)
    {
        // Epic Music
        _musicInstance.setParameterByNameWithLabel(StateParameter, StateEpic);
        RuntimeManager.StudioSystem.setParameterByName(globalParamName, 1.0f);
    }

    private void OnTriggerExit(Collider other)
    {
        // Sad Music
        _musicInstance.setParameterByNameWithLabel(StateParameter, StateSad);
        RuntimeManager.StudioSystem.setParameterByName(globalParamName, 0.0f);
    }

    public void SetVolume(float volume)
    {
        _musicInstance.setVolume(volume);
    }
}