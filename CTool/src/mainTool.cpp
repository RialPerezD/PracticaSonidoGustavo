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

struct AudioNode {
    int id = -1;
    int inputPin = -1;
    int outputPin = -1;
    int extraOutputPin = -1;
    int audioIndex = -1;
    bool to_delete = false;
    bool condition = true;
    std::string name = "";
};

struct Link {
    int id;
    int startAttr;
    int endAttr;
};

std::vector<AudioNode> audioNodes;
std::vector<Link> links;
int nextId = 100;
int nextLinkId = 1;
AudioManager audioManager;
int currentPlayingNodeId = -1;
bool state = true;

AudioNode* FindNodeById(int nodeId) {
    for (auto& n : audioNodes) if (n.id == nodeId) return &n;
    return nullptr;
}

bool LinkExists(int startAttr, int endAttr) {
    for (const auto& l : links) if (l.startAttr == startAttr && l.endAttr == endAttr) return true;
    return false;
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(1600, 800, "ImNodes + Audio Graph", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImNodes::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (!audioManager.Init()) std::cerr << "Failed to init AudioManager\n";

    AudioNode startNode;
    startNode.id = nextId++;
    startNode.inputPin = nextId++;
    startNode.outputPin = nextId++;
    startNode.extraOutputPin = nextId++;
    startNode.name = "Intro (Inicio)";
    startNode.audioIndex = audioManager.LoadWav("../assets/0_intro.wav");
    audioNodes.push_back(startNode);
    ImNodes::SetNodeScreenSpacePos(startNode.id, ImVec2(100.0f, 100.0f));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (currentPlayingNodeId != -1) {
            AudioNode* currentNode = FindNodeById(currentPlayingNodeId);
            if (currentNode) {
                if (!audioManager.IsPlaying(currentNode->audioIndex)) {
                    int nextNodeId = -1;
                    int selectedPin;
                    if (currentNode->name == "If Node") {
                        selectedPin = currentNode->condition ? currentNode->outputPin : currentNode->extraOutputPin;
                    }
                    else {
                        selectedPin = state ? currentNode->outputPin : currentNode->extraOutputPin;
                    }

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

                    if (nextNodeId != -1) {
                        AudioNode* nextNode = FindNodeById(nextNodeId);
                        if (nextNode) {
                            if (nextNode->audioIndex != -1) audioManager.Play(nextNode->audioIndex, false);
                            currentPlayingNodeId = nextNodeId;
                        }
                        else currentPlayingNodeId = -1;
                    }
                    else currentPlayingNodeId = -1;
                }
            }
            else currentPlayingNodeId = -1;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Nodos")) {
                if (ImGui::MenuItem("Intro")) create_audio_node("Intro", "../assets/0_intro.wav");
                if (ImGui::MenuItem("First Piano")) create_audio_node("First Piano", "../assets/1_firstPiano.wav");
                if (ImGui::MenuItem("Ding ding")) create_audio_node("Ding ding", "../assets/2_dingDing.wav");
                if (ImGui::MenuItem("This is crazy frog")) create_audio_node("This is crazy frog", "../assets/3_thisIsCrazyFrog.wav");
                if (ImGui::MenuItem("Body song")) create_audio_node("Body song", "../assets/4_mainSong.wav");
                if (ImGui::MenuItem("Motillo")) create_audio_node("Motillo", "../assets/5_motillo.wav");

                if (ImGui::MenuItem("If Node")) {
                    AudioNode n;
                    n.id = nextId++;
                    n.inputPin = nextId++;
                    n.outputPin = nextId++;
                    n.extraOutputPin = nextId++;
                    n.name = "If Node";
                    n.audioIndex = -1;
                    n.condition = true;
                    audioNodes.push_back(n);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Change state")) {
                if (ImGui::MenuItem("Toggle state")) state = !state;
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImNodes::BeginNodeEditor();

        for (auto& n : audioNodes) {
            if (n.to_delete) continue;

            ImNodes::BeginNode(n.id);
            ImNodes::BeginNodeTitleBar();
            ImGui::Text("%s", n.name.c_str());
            ImNodes::EndNodeTitleBar();

            if (n.audioIndex != -1) ImGui::Text("Audio Index: %d", n.audioIndex);
            if (n.id == currentPlayingNodeId) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "-> PLAYING");

            std::string playId = "Play##" + std::to_string(n.id);
            if (ImGui::Button(playId.c_str())) {
                if (currentPlayingNodeId != -1) {
                    AudioNode* prevNode = FindNodeById(currentPlayingNodeId);
                    if (prevNode && prevNode->audioIndex != -1) audioManager.Stop(prevNode->audioIndex);
                }
                if (n.audioIndex != -1) audioManager.Play(n.audioIndex, false);
                currentPlayingNodeId = n.id;
            }

            ImGui::SameLine();
            std::string deleteId = "Eliminar##" + std::to_string(n.id);
            if (ImGui::Button(deleteId.c_str())) {
                if (n.id == currentPlayingNodeId && n.audioIndex != -1) {
                    audioManager.Stop(n.audioIndex);
                    currentPlayingNodeId = -1;
                }
                n.to_delete = true;
            }

            if (n.name == "If Node") ImGui::Checkbox("Condition", &n.condition);

            ImNodes::BeginInputAttribute(n.inputPin);
            ImGui::Text("Input");
            ImNodes::EndInputAttribute();

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

        for (const auto& l : links) ImNodes::Link(l.id, l.startAttr, l.endAttr);
        ImNodes::EndNodeEditor();

        int startAttr = 0, endAttr = 0;
        if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
            if (startAttr != endAttr && !LinkExists(startAttr, endAttr)) links.push_back({ nextLinkId++, startAttr, endAttr });
        }

        int destroyedLinkId;
        if (ImNodes::IsLinkDestroyed(&destroyedLinkId)) {
            links.erase(std::remove_if(links.begin(), links.end(), [destroyedLinkId](const Link& link) { return link.id == destroyedLinkId; }), links.end());
        }

        links.erase(std::remove_if(links.begin(), links.end(), [](const Link& l) {
            for (const auto& n : audioNodes) {
                if (n.to_delete && (l.startAttr == n.inputPin || l.startAttr == n.outputPin || l.startAttr == n.extraOutputPin || l.endAttr == n.inputPin || l.endAttr == n.outputPin || l.endAttr == n.extraOutputPin)) return true;
            }
            return false;
            }), links.end());

        audioNodes.erase(std::remove_if(audioNodes.begin(), audioNodes.end(), [](const AudioNode& n) { return n.to_delete; }), audioNodes.end());

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    audioManager.Close();
    ImNodes::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
