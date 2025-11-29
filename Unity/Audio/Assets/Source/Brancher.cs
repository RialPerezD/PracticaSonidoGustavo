using FMOD.Studio;
using FMODUnity;
using UnityEngine;

public class Brancher : MonoBehaviour
{
    [Header("FMOD")] public EventReference musicEvent;

    public void BranchToExploration()
    {
        _musicInstance.setParameterByNameWithLabel(StateParameter, StateExploration);
    }

    public void BranchToCombat()
    {
        _musicInstance.setParameterByNameWithLabel(StateParameter, StateCombat);
    }

    private const string StateParameter = "State";
    private const string StateExploration = "Exploration";
    private const string StateCombat = "Combat";
    private EventInstance _musicInstance;

    private void Start()
    {
        _musicInstance = RuntimeManager.CreateInstance(musicEvent);
        _musicInstance.start();
        BranchToExploration();
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.LeftArrow))
            BranchToExploration();
        else if (Input.GetKeyDown(KeyCode.RightArrow))
            BranchToCombat();
    }
}