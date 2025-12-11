using UnityEngine;
using System.Collections;
using System.Collections.Generic;
public class RandomLayerToggling : MonoBehaviour
{
    [Header("Refrences")] public Layerer layersController;

    [Header("Settings")] 
    public string[] layerNames;
    public Vector2 timeOffRange = new Vector2(10.0f, 15.0f);
    public Vector2 timeOnRange = new Vector2(8.0f, 10.0f);
    
    
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        StartCoroutine(RandomToggleRoutine());
    }

    /**
     * <summary>
     * Coroutine that randomly toggles a specified layer on and off at random intervals.
     * </summary>
     */
    IEnumerator RandomToggleRoutine()
    {
        while (true)
        {
            float waitOff = Random.Range(timeOffRange.x, timeOffRange.y);
            yield return new WaitForSeconds(waitOff);
            if (layersController)
            {
                layersController.ToggleLayer(layerNames[0]);
            }
            
            float waitOn = Random.Range(timeOnRange.x, timeOnRange.y);
            yield return new WaitForSeconds(waitOn);
            if (layersController)
            {
                layersController.ToggleLayer(layerNames[0]);
            }
        }
    }
    
}
