using FMOD.Studio;
using FMODUnity;
using UnityEngine;
using System.Collections;


public class OneShotEmitter : MonoBehaviour
{
    [Header("FMOD")] public EventReference soundEvent;
    [Header("FMOD Settings")]
    
    private EventInstance _soundInstance;

    public Vector2 timeRange = new Vector2(3.0f, 7.0f);
    
    public float minPitch = 0.95f;
    public float maxPitch = 1.05f;
    
    
    void Start()
    {
        StartCoroutine(PlayRoutine());

    }

    /**
     * <summary>
     * Coroutine that plays the one-shot sound at random intervals.
     * </summary>
     */
    private IEnumerator PlayRoutine()
    {
        while (true)
        {
            float delay = Random.Range(timeRange.x, timeRange.y);
            yield return new WaitForSeconds(delay);

            PlayOneShot3D();
        }
    }
    
    /**
     * <summary>
     * Plays the one-shot sound in 3D space with random pitch variation.
     * </summary>
     */
    private void PlayOneShot3D()
    {
        _soundInstance =  RuntimeManager.CreateInstance(soundEvent);
        
        float randomPitch = Random.Range(minPitch, maxPitch);
        _soundInstance.setPitch(randomPitch);
        
        _soundInstance.set3DAttributes(RuntimeUtils.To3DAttributes(gameObject));
        _soundInstance.start();
        _soundInstance.release();
    }
 }

