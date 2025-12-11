/**
 * @file main_graph_editor.cpp
 * @brief An interactive audio graph editor built using ImGui and ImNodes.
 *
 * This application allows users to define a sequence of audio playback nodes
 * and links them to create a dynamic music or sound flow graph. The flow logic
 * automatically progresses to the next linked node upon audio completion.
 */

#include "imgui.h"
#include "imnodes.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "../deps/glfw/include/GLFW/glfw3.h"
#include "sound.h"

 /**
  * @brief Structure representing an audio processing or playback node in the graph.
  */
struct AudioNode {
    int id = -1; /**< Unique node identifier used by ImNodes. */
    int inputPin = -1; /**< Unique identifier for the input attribute pin. */
    int outputPin = -1; /**< Unique identifier for the primary output attribute pin (Output 1). */
    int extraOutputPin = -1; /**< Unique identifier for the secondary output attribute pin (Output 2). */
    int audioIndex = -1; /**< Index of the loaded audio track in AudioManager (-1 if no audio, e.g., "If Node"). */
    bool to_delete = false; /**< Flag indicating if the node should be removed on the next cleanup cycle. */
    bool condition = true; /**< Boolean state used for conditional branching nodes (e.g., "If Node"). */
    std::string name = ""; /**< Display name of the node. */
};

/**
 * @brief Structure representing a connection (link) between two attribute pins.
 */
struct Link {
    int id; /**< Unique link identifier. */
    int startAttr; /**< Identifier of the source attribute pin (output). */
    int endAttr; /**< Identifier of the destination attribute pin (input). */
};

/** @brief Global vector holding all currently active audio nodes. */
std::vector<AudioNode> audioNodes;
/** @brief Global vector holding all active links between node pins. */
std::vector<Link> links;
/** @brief Counter for generating unique node and pin IDs. */
int nextId = 100;
/** @brief Counter for generating unique link IDs. */
int nextLinkId = 1;
/** @brief Global instance of the audio manager responsible for OpenAL operations. */
AudioManager audioManager;
/** @brief ID of the node currently playing audio (-1 if none). */
int currentPlayingNodeId = -1;
/** @brief Global state flag used for simple branching logic in standard nodes. */
bool state = true;

/**
 * @brief Searches for an AudioNode by its unique ID.
 *
 * @param nodeId The ID of the node to find.
 * @return A pointer to the found AudioNode, or nullptr if not found.
 */
AudioNode* FindNodeById(int nodeId) {
    for (auto& n : audioNodes) if (n.id == nodeId) return &n;
    return nullptr;
}

/**
 * @brief Checks if a link already exists between two specific attribute pins.
 *
 * @param startAttr The identifier of the starting pin.
 * @param endAttr The identifier of the ending pin.
 * @return True if a link exists, false otherwise.
 */
bool LinkExists(int startAttr, int endAttr) {
    for (const auto& l : links) if (l.startAttr == startAttr && l.endAttr == endAttr) return true;
    return false;
}

/**
 * @brief The main function of the audio graph editor application.
 *
 * Initializes all dependencies, sets up the initial graph, and runs the main loop
 * to handle audio flow, GUI rendering, and user input.
 *
 * @return 0 on successful execution, -1 on initialization failure.
 */
int main() {
    // Initialize GLFW
    if (!glfwInit()) return -1;

    // Create the GLFW window and OpenGL context
    GLFWwindow* window = glfwCreateWindow(1600, 800, "ImNodes + Audio Graph", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    // Initialize ImGui and ImNodes contexts
    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Initialize the AudioManager (OpenAL)
    if (!audioManager.Init()) std::cerr << "Failed to init AudioManager\n";

    // --- Initial Graph Setup ---

    // Create the mandatory 'Start' node
    AudioNode startNode;
    startNode.id = nextId++;
    startNode.inputPin = nextId++;
    startNode.outputPin = nextId++;
    startNode.extraOutputPin = nextId++;
    startNode.name = "Intro (Start)";
    startNode.audioIndex = audioManager.LoadWav("../assets/0_intro.wav");
    audioNodes.push_back(startNode);
    ImNodes::SetNodeScreenSpacePos(startNode.id, ImVec2(100, 100));

    // Helper lambda to add audio nodes and set their initial position
    auto add = [&](const char* name, const char* path, ImVec2 pos) {
        AudioNode n;
        n.id = nextId++;
        n.inputPin = nextId++;
        n.outputPin = nextId++;
        n.extraOutputPin = nextId++;
        n.name = name;
        n.audioIndex = audioManager.LoadWav(path);
        audioNodes.push_back(n);
        ImNodes::SetNodeScreenSpacePos(n.id, pos);
        return n.id;
        };

    // Create the remaining hardcoded initial nodes
    int firstPiano = add("First Piano", "../assets/1_firstPiano.wav", ImVec2(350, 100));
    int ding1 = add("Ding ding", "../assets/2_dingDing.wav", ImVec2(600, 100));
    int crazy = add("This is crazy frog", "../assets/3_thisIsCrazyFrog.wav", ImVec2(850, 100));
    int body = add("Body song", "../assets/4_mainSong.wav", ImVec2(1100, 100));
    int ding2 = add("Ding ding", "../assets/2_dingDing.wav", ImVec2(600, 350));
    int motillo1 = add("Motillo", "../assets/5_motillo.wav", ImVec2(900, 350));
    int motillo2 = add("Motillo", "../assets/5_motillo.wav", ImVec2(900, 450));

    // Helper lambda to find the pin ID associated with a given node ID
    auto pin = [&](int nodeId, bool input, bool extra) {
        for (auto& n : audioNodes)
            if (n.id == nodeId)
                return input ? n.inputPin : (extra ? n.extraOutputPin : n.outputPin);
        return -1;
        };

    // Helper lambda to create a link between two pins
    auto link = [&](int a, int b) {
        links.push_back({ nextLinkId++, a, b });
        };

    // Create the hardcoded initial links defining the audio flow
    link(pin(startNode.id, false, false), pin(firstPiano, true, false));
    link(pin(startNode.id, false, true), pin(firstPiano, true, false));
    link(pin(firstPiano, false, false), pin(ding1, true, false));
    link(pin(firstPiano, false, true), pin(ding1, true, false));
    link(pin(ding1, false, false), pin(crazy, true, false));
    link(pin(ding1, false, true), pin(crazy, true, false));
    link(pin(crazy, false, false), pin(body, true, false));
    link(pin(crazy, false, true), pin(ding2, true, false));
    link(pin(body, false, false), pin(ding2, true, false));
    link(pin(body, false, true), pin(ding2, true, false));
    link(pin(ding2, false, false), pin(crazy, true, false));
    link(pin(ding2, false, true), pin(motillo1, true, false));
    link(pin(motillo1, false, false), pin(ding2, true, false));
    link(pin(motillo1, false, true), pin(motillo2, true, false));
    link(pin(motillo2, false, false), pin(ding2, true, false));
    link(pin(motillo2, false, true), pin(motillo1, true, false));

    // --- Main Rendering and Logic Loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // --- Audio Flow Logic Update ---
        if (currentPlayingNodeId != -1) {
            AudioNode* currentNode = FindNodeById(currentPlayingNodeId);
            if (currentNode) {
                // Check if the current node's audio has finished playing
                if (!audioManager.IsPlaying(currentNode->audioIndex)) {
                    int nextNodeId = -1;
                    int selectedPin;

                    // Determine which output pin to follow based on node type
                    if (currentNode->name == "If Node") {
                        selectedPin = currentNode->condition ? currentNode->outputPin : currentNode->extraOutputPin;
                    }
                    else {
                        // Use global 'state' for branching if not a dedicated 'If Node'
                        selectedPin = state ? currentNode->outputPin : currentNode->extraOutputPin;
                    }

                    // Traverse links to find the input pin connected to the selected output pin
                    for (const auto& l : links) {
                        if (l.startAttr == selectedPin) {
                            for (const auto& next : audioNodes) {
                                if (next.inputPin == l.endAttr) {
                                    nextNodeId = next.id;
                                    break;
                                }
                            }
                        }
                        if (nextNodeId != -1) break;
                    }

                    // Transition to the next node
                    if (nextNodeId != -1) {
                        AudioNode* nextNode = FindNodeById(nextNodeId);
                        if (nextNode) {
                            // Only play audio if the node has an associated audio index
                            if (nextNode->audioIndex != -1) audioManager.Play(nextNode->audioIndex, false);
                            currentPlayingNodeId = nextNodeId;
                        }
                        else currentPlayingNodeId = -1;
                    }
                    else currentPlayingNodeId = -1; // End of audio flow
                }
            }
            else currentPlayingNodeId = -1;
        }

        // --- ImGui/ImNodes Rendering Setup ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Helper lambda to create and initialize a new audio node
        auto create_audio_node = [&](const std::string& name, const std::string& path) {
            AudioNode n;
            n.id = nextId++;
            n.inputPin = nextId++;
            n.outputPin = nextId++;
            n.extraOutputPin = nextId++;
            n.name = name;
            n.audioIndex = audioManager.LoadWav(path.c_str());
            audioNodes.push_back(n);
            };

        // --- Menu Bar (Node Creation and State Toggling) ---
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Nodos")) {
                // Audio nodes creation menu
                if (ImGui::MenuItem("Intro")) create_audio_node("Intro", "../assets/0_intro.wav");
                if (ImGui::MenuItem("First Piano")) create_audio_node("First Piano", "../assets/1_firstPiano.wav");
                if (ImGui::MenuItem("Ding ding")) create_audio_node("Ding ding", "../assets/2_dingDing.wav");
                if (ImGui::MenuItem("This is crazy frog")) create_audio_node("This is crazy frog", "../assets/3_thisIsCrazyFrog.wav");
                if (ImGui::MenuItem("Body song")) create_audio_node("Body song", "../assets/4_mainSong.wav");
                if (ImGui::MenuItem("Motillo")) create_audio_node("Motillo", "../assets/5_motillo.wav");

                // Conditional node creation menu
                if (ImGui::MenuItem("If Node")) {
                    AudioNode n;
                    n.id = nextId++;
                    n.inputPin = nextId++;
                    n.outputPin = nextId++;
                    n.extraOutputPin = nextId++;
                    n.name = "If Node";
                    n.audioIndex = -1; // No associated audio
                    n.condition = true;
                    audioNodes.push_back(n);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Change state")) {
                // Global state toggling menu
                if (ImGui::MenuItem("Toggle state")) state = !state;
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // --- Node Editor Drawing ---
        ImNodes::BeginNodeEditor();

        // Iterate and draw all active nodes
        for (auto& n : audioNodes) {
            if (n.to_delete) continue;

            ImNodes::BeginNode(n.id);
            ImNodes::BeginNodeTitleBar();
            ImGui::Text("%s", n.name.c_str());
            ImNodes::EndNodeTitleBar();

            // Node content display
            if (n.audioIndex != -1) ImGui::Text("Audio Index: %d", n.audioIndex);
            if (n.id == currentPlayingNodeId) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "-> PLAYING");

            // Manual Play button logic
            std::string playId = "Play##" + std::to_string(n.id);
            if (ImGui::Button(playId.c_str())) {
                // Stop any previously playing node
                if (currentPlayingNodeId != -1) {
                    AudioNode* prevNode = FindNodeById(currentPlayingNodeId);
                    if (prevNode && prevNode->audioIndex != -1) audioManager.Stop(prevNode->audioIndex);
                }
                // Start playing this node
                if (n.audioIndex != -1) audioManager.Play(n.audioIndex, false);
                currentPlayingNodeId = n.id;
            }

            // Delete button logic
            ImGui::SameLine();
            std::string deleteId = "Delete##" + std::to_string(n.id);
            if (ImGui::Button(deleteId.c_str())) {
                // Stop audio if this node is currently playing
                if (n.id == currentPlayingNodeId && n.audioIndex != -1) {
                    audioManager.Stop(n.audioIndex);
                    currentPlayingNodeId = -1;
                }
                n.to_delete = true; // Mark for deletion
            }

            // Conditional checkbox for "If Node"
            if (n.name == "If Node") ImGui::Checkbox("Condition", &n.condition);

            // Input Attribute (Pin)
            ImNodes::BeginInputAttribute(n.inputPin);
            ImGui::Text("Input");
            ImNodes::EndInputAttribute();

            // Output Attributes (Pins)
            ImGui::BeginGroup();
            ImNodes::BeginOutputAttribute(n.outputPin);
            ImGui::Text("Output 1");
            ImNodes::EndOutputAttribute();

            ImNodes::BeginOutputAttribute(n.extraOutputPin);
            ImGui::Text("Output 2");
            ImNodes::EndOutputAttribute();
            ImGui::EndGroup();

            ImNodes::EndNode();
        }

        // Draw Links
        for (const auto& l : links) ImNodes::Link(l.id, l.startAttr, l.endAttr);
        ImNodes::EndNodeEditor();

        // --- Link Creation and Destruction Handling ---
        int startAttr = 0, endAttr = 0;
        // Handle creation of a new link by user interaction
        if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
            if (startAttr != endAttr && !LinkExists(startAttr, endAttr)) links.push_back({ nextLinkId++, startAttr, endAttr });
        }

        int destroyedLinkId;
        // Handle destruction of a link by user interaction
        if (ImNodes::IsLinkDestroyed(&destroyedLinkId)) {
            links.erase(std::remove_if(links.begin(), links.end(), [destroyedLinkId](const Link& link) { return link.id == destroyedLinkId; }), links.end());
        }

        // --- Cleanup (Remove Links and Nodes) ---

        // Remove links connected to nodes marked for deletion
        links.erase(std::remove_if(links.begin(), links.end(), [](const Link& l) {
            for (const auto& n : audioNodes) {
                if (n.to_delete && (l.startAttr == n.inputPin || l.startAttr == n.outputPin || l.startAttr == n.extraOutputPin || l.endAttr == n.inputPin || l.endAttr == n.outputPin || l.endAttr == n.extraOutputPin)) return true;
            }
            return false;
            }), links.end());

        // Final deletion of nodes marked for deletion
        audioNodes.erase(std::remove_if(audioNodes.begin(), audioNodes.end(), [](const AudioNode& n) { return n.to_delete; }), audioNodes.end());

        // --- Rendering ---
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // --- Shutdown and Cleanup ---
    audioManager.Close();
    ImNodes::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}