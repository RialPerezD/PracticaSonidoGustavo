using UnityEngine;
using FMODUnity;
using FMOD.Studio;

public class MusicCrossFader : MonoBehaviour
{
    [Header("Configuración FMOD")]
    public EventReference eventoMusica; // Arrastra aquí tu evento "MusicManager"

    [Header("Control de Mezcla")]
    [Range(0f, 1f)]
    public float intensidadMezcla = 0f; // 0 = Canción A, 1 = Canción B

    [Tooltip("Qué tan rápido ocurre la transición")]
    public float velocidadTransicion = 2.0f;

    private EventInstance instanciaMusica;
    private float valorActual = 0f;

    void Start()
    {
        // 1. Crear e iniciar la instancia de música
        instanciaMusica = RuntimeManager.CreateInstance(eventoMusica);
        instanciaMusica.start();

        // Inicializamos el parámetro en 0
        instanciaMusica.setParameterByName("Mezcla", 0f);
    }

    void Update()
    {
        // 2. Suavizado matemático (Lerp)
        // Esto hace que si cambias intensidadMezcla bruscamente, el audio cambie suavemente
        if (Mathf.Abs(valorActual - intensidadMezcla) > 0.001f)
        {
            valorActual = Mathf.MoveTowards(valorActual, intensidadMezcla, velocidadTransicion * Time.deltaTime);

            // 3. Enviar el valor a FMOD
            instanciaMusica.setParameterByName("Mezcla", valorActual);
        }

        // --- PRUEBA RÁPIDA (TECLAS) ---
        if (Input.GetKeyDown(KeyCode.A)) CambiarACancionA();
        if (Input.GetKeyDown(KeyCode.B)) CambiarACancionB();
    }

    // Métodos públicos para llamar desde otros scripts o eventos de UI
    public void CambiarACancionA()
    {
        intensidadMezcla = 0f;
    }

    public void CambiarACancionB()
    {
        intensidadMezcla = 1f;
    }

    // Limpieza de memoria
    void OnDestroy()
    {
        instanciaMusica.stop(FMOD.Studio.STOP_MODE.ALLOWFADEOUT);
        instanciaMusica.release();
    }
}