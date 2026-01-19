#pragma once
#include <memory>
#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "ui/ImGuiLayer.h"
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

class App
{
public:
    App();
    int Run();

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<ImGuiLayer> imGuiLayer;
    char username[64]{};
    char password[64]{};
    std::string login_btn_text = "µÇÂ¼";
    std::thread run_thread;
    std::vector<std::vector<char*>> courseValue;
    std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
	long long userId = 0; // ÓÃ»§ID
	std::string name; // ÓÃ»§ÐÕÃû
	bool loginInProgress = false; // µÇÂ¼ÖÐ×´Ì¬
    cpr::Cookies _cookies;
    nlohmann::json all_courses;
    bool loggedIn = false; // µÇÂ¼×´Ì¬

    void LoginWindow();
    void MainWindow();
};
