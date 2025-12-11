using FMOD.Studio;
using FMODUnity;
using UnityEngine;
using STOP_MODE = FMOD.Studio.STOP_MODE;

public class LoopableAmbiance : MonoBehaviour
{
    [Header("FMOD")] public EventReference soundEvent;
    
    private EventInstance _soundInstance;

    private bool _isPlaying = false;
    
    void Start()
    {
        _soundInstance = RuntimeManager.CreateInstance(soundEvent);
        StartAmbiance(gameObject.transform, gameObject.transform.position);
    }
    
    /**
    * <summary>
     * Starts playing the ambiance sound at the specified source or world position.
     * </summary>
     *
     * <param name = "source"> 
    *  The Transform of the GameObject to attach the sound to.
     * </param>
     * <param name = "worldPos">
     * The world position to play the sound at if no source is provided.
     * </param>
    */
    public void StartAmbiance(Transform source, Vector3 worldPos)
    {
        if (_isPlaying) return;

        if (source)
        {
            RuntimeManager.AttachInstanceToGameObject(_soundInstance, source.gameObject, source.GetComponent<Rigidbody>());
        }
        else
        {
            _soundInstance.set3DAttributes(RuntimeUtils.To3DAttributes(worldPos));
        }

        _soundInstance.start();
        _isPlaying = true;
    }

    /**
     * <summary>
     * Stops playing the ambiance sound, with an option to fade out.
     * </summary>
     * 
     * <param name =  "fadeOut">
     * If true, the sound will fade out; otherwise, it will stop immediately.
     * </param>
     */
    public void StopAmbience(bool fadeOut = true)
    {
        if (!_isPlaying) return;

        _soundInstance.stop(fadeOut ? STOP_MODE.ALLOWFADEOUT : STOP_MODE.IMMEDIATE);
        _soundInstance.release();
        _isPlaying = false;
    }
}
