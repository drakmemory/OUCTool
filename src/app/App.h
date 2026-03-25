#pragma once
#include <memory>
#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "ui/ImGuiLayer.h"
#include "utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cpr/cpr.h>
#include <codecvt>
#include <locale>
#pragma execution_character_set("utf-8")
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
    std::string login_btn_text = "登录";
    std::thread run_thread;
    std::vector<std::vector<char*>> courseValue;
    std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
	long long userId = 0; // 用户ID
	std::string name; // 用户姓名
	bool loginInProgress = false; // 登录中状态
    cpr::Cookies _cookies;
    nlohmann::json all_courses;
    bool loggedIn = false; // 登录状态
    void LoginWindow();
    void MainWindow();
};
