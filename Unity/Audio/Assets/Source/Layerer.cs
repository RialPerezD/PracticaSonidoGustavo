using FMOD.Studio;
using FMODUnity;
using UnityEngine;
using System.Collections.Generic;

public class Layerer : MonoBehaviour
{
    [Header("FMOD")]
    public EventReference musicEvent;

    public EventInstance _musicInstance;

    [System.Serializable]
    public struct Layer
    {
        public string name;
        public string parameterLabel;
        [HideInInspector] public float currentValue;
        [HideInInspector] public float targetValue;
    }

    public Layer[] layers;
    public float fadeSpeed = 3.0f;

    #region LayerToggling

    public void ToggleLayer(string layerName)
    {
        for (int i = 0; i < layers.Length; i++)
        {
            if (layers[i].parameterLabel != layerName) continue;

            Layer layer = layers[i];
            layer.targetValue = (layer.targetValue == 0.0f) ? 1.0f : 0.0f;
            layers[i] = layer;
            return;
        }
    }

    #endregion

    /**
     * <summary>
     * Sets the overall volume of the music instance.
     * This is somewhat legacy since this class was more for testing.
     * </summary>
     * 
     * <param name = "volume">
     * The desired volume level (0.0f to 1.0f).
     * </param>
     */
    public void SetVolume(float volume)
    {
        _musicInstance.setVolume(volume);
    }

    private void Start()
    {
        _musicInstance = RuntimeManager.CreateInstance(musicEvent);
        _musicInstance.start();

        for (int i = 0; i < layers.Length; i++)
        {
            layers[i].currentValue = 0.0f;
            layers[i].targetValue = 1.0f;
        }

        layers[1].targetValue = 0.0f;
    }

    private void Update()
    {
        for (int i = 0; i < layers.Length; i++)
        {
            Layer layer = layers[i];

            if (layer.currentValue < layer.targetValue)
            {
                layer.currentValue =
                    Mathf.MoveTowards(layer.currentValue, layer.targetValue, Time.deltaTime * fadeSpeed);

                _musicInstance.setParameterByName(layer.parameterLabel, layer.currentValue);
                layers[i] = layer;
            }
        }
    }
}