#include "App.h"
#include "../platform/Window.h"
#include "../renderer/Renderer.h"
#include "../ui/ImGuiLayer.h"
#include <regex>
#include <filesystem>

static const char* all_items[] = {"课程观看次数", "课程观看时长", "音视频观看次数", "音视频观看时长"};
App::App()
{
    window = std::make_unique<Window>("OUCTool", 600, 360);
    renderer = std::make_unique<Renderer>(window->GetHWND(), window->GetWidth(), window->GetHeight());
    imGuiLayer = std::make_unique<ImGuiLayer>(window->GetHWND(), renderer->GetDevice(), renderer->GetContext());
    if (std::filesystem::exists("user_info.json")) {
        std::ifstream f("user_info.json");
        nlohmann::json data = nlohmann::json::parse(f);
        strcpy_s(username, sizeof(username), data["name"].get<std::string>().data());
        strcpy_s(password, sizeof(password), data["pwd"].get<std::string>().data());
    }
}

int App::Run()
{
    while (!window->ShouldClose())
    {
        window->PollEvents();
        renderer->BeginFrame();
        imGuiLayer->Begin();
        // --- 绘制粉蓝渐变背景 ---
        ImDrawList* bgList = ImGui::GetBackgroundDrawList();
        // 定义颜色 (RGBA)
        ImU32 colPink = IM_COL32(255, 182, 193, 255); // 浅粉色 (Light Pink)
        ImU32 colBlue = IM_COL32(106, 90, 205, 255); // 板岩暗蓝灰色 (Slate Blue)
		int width = window->GetWidth();
		int height = window->GetHeight();
        // 绘制垂直渐变 (上方粉色，下方蓝色)
        bgList->AddRectFilledMultiColor(
            ImVec2(0, 0),
            ImVec2(static_cast<float>(width), static_cast<float>(height)), // 这里建议动态获取窗口宽高
            colPink, // 左上
            colBlue, // 右上
            colBlue, // 右下
            colPink  // 左下
        );
        if(loggedIn)
            MainWindow();
		else
            LoginWindow();
        imGuiLayer->End();
        renderer->EndFrame();
    }
    return 0;
}

void App::MainWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("MainWindow", nullptr, window_flags);

    if (all_courses.is_null())
    {
        // 首次显示获取课程列表
        session->SetUrl(cpr::Url{
            "https://lms.ouchn.cn/api/my-courses?conditions={%22status%22:[%22ongoing%22]}&fields=id,name,course_code,course_type,display_name,org_id,imported_from&page=1&page_size=100"
            });

        auto r = session->Get();
        if (r.status_code == 200)
        {
            all_courses = nlohmann::json::parse(r.text);
            for(auto& course : all_courses["courses"])
            {
                course["selection"] = 0;
                course["hint"] = "请输入课程观看次数";
                if (course["completeness"].is_null())
                    course["completeness"] = 0;
                courseValue.push_back(std::vector<char*>(4));
                for(int i = 0; i < 4; i++)
                {
                    courseValue.back()[i] = new char[32];
                    strcpy_s(courseValue.back()[i], 32, "");
                }
			}
        }
    }
    else
    {
		// 显示课程列表
        auto& courses = all_courses["courses"];
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, ImGui::GetWindowSize().y - 50), ImGuiWindowFlags_NoScrollbar, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
        {
            if (ImGui::BeginTable(
                "CourseGrid",
                3,
                ImGuiTableFlags_SizingStretchSame))
            {
                for (int n = 0; n < courses.size(); n++)
                {
                    ImGui::TableNextColumn();
                    ImGui::PushID(n);
					ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(128, 128, 128, 80));
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 5));
                    ImGui::BeginChild(
                        "Card",
                        ImVec2(-FLT_MIN, 120),
                        ImGuiChildFlags_AlwaysUseWindowPadding,
                        ImGuiWindowFlags_NoScrollbar
                    );
                    ImGui::PopStyleColor(1);
                    ImGui::PopStyleVar(2);
					ImGui::PushFont(imGuiLayer->fontBold);
					ImGui::SetWindowFontScale(0.9f);
                    ImGui::TextWrapped(
                        courses[n]["display_name"]
                        .get<std::string>().c_str()
                    );
					ImGui::PopFont();
                    ImGui::SetWindowFontScale(1.f);
                    float progress =
                        courses[n]["completeness"].get<float>() / 100.0f;
                    ImGui::PushFont(imGuiLayer->fontSmall);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.f);
                    ImGui::ProgressBar(progress, ImVec2(ImGui::GetWindowSize().x - ImGui::CalcTextSize(std::format("{}%%", progress * 100).data()).x - 16, 8), "");
                    ImGui::PopStyleVar();
                    float originalY = ImGui::GetCursorPosY();
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(originalY - 17);
                    ImGui::TextColored({ 0,0,0,255 }, std::format("{}%%", progress * 100).data());
                    ImGui::Spacing();
                    ImGui::PopFont();
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(255, 255, 255, 255));
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(20, 20, 20, 255));            // 输入文字深黑色
                    ImGui::PushStyleColor(ImGuiCol_TextDisabled, IM_COL32(105, 105, 105, 255)); // 提示文字暗灰色
                    ImGui::PushStyleColor(ImGuiCol_InputTextCursor, IM_COL32(20, 20, 20, 255)); // 强制光标为黑色
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - 16);
					int custom_selection = courses[n]["selection"].get<int>();
                    ImGui::InputTextWithHint("##input", courses[n]["hint"].get<std::string>().c_str(), 
                        courseValue[n][custom_selection], sizeof(courseValue[n][custom_selection]));
					ImGui::Spacing();
                    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - 16);
                    if (ImGui::BeginCombo("##自定义", all_items[custom_selection], ImGuiComboFlags_NoArrowButton)) {
                        for (int i = 0; i < IM_ARRAYSIZE(all_items); i++)
                        {
                            bool selected = (custom_selection == i);
                            if (ImGui::Selectable(all_items[i], selected))
                            {
                                courses[n]["selection"] = i;
                                courses[n]["hint"] = "请输入" + std::string(all_items[i]); // 写回输入框
                            }
                            if (selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopStyleColor(5);
                    ImGui::PopStyleVar();

                    ImGui::EndChild();
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            ImGui::EndChild();
			auto textSize = ImGui::CalcTextSize("开始执行");
            ImGui::Spacing();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - textSize.x) / 2);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(138, 43, 226, 255));        // 正常状态
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(148, 63, 236, 255)); // 悬停状态
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(118, 23, 206, 255));  // 点击状态
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));          // 文字白色
            if (ImGui::Button("开始执行"))
            {
                // 创建刷课线程
                system("cls");
                run_thread = std::thread([this]() {
                    auto& courses = all_courses["courses"];
                    for (int i = 0; i < courses.size(); i++) {
                        // 获取学习过的课
                        auto course_id = courses[i]["id"].get<long long>();
                        session->SetUrl(std::format(
                            "https://lms.ouchn.cn/api/course/{}/activity-reads-for-user",
                            course_id
						));
                        nlohmann::json activity_reads, all_completeness, modules, all_activity;
						cpr::Response r = session->Get();
                        if (r.status_code == 200)
                        {
							activity_reads = nlohmann::json::parse(r.text);
							activity_reads = activity_reads["activity_reads"];
                            for (auto& read : activity_reads) {
                                all_completeness[std::to_string(read["activity_id"].get<long long>())] = read["completeness"].get<std::string>();
                            }
                        }
						// 获取当前课程模块
                        session->SetUrl({ std::format("https://lms.ouchn.cn/api/courses/{}/modules",course_id) });
						r = session->Get();
                        if (r.status_code == 200)
                        {
							modules = nlohmann::json::parse(r.text)["modules"];
                        }
                        std::string module_ids = "[";
                        for (auto& m : modules) {
                            module_ids += std::to_string(m["id"].get<long long>()) + ",";
                        }
						module_ids.pop_back();
						module_ids += ']';
                        // 获取当前课程所有任务
                        session->SetUrl(std::format("https://lms.ouchn.cn/api/course/{}/all-activities?module_ids={}&activity_types=learning_activities,exams,classrooms", course_id, module_ids));
						r = session->Get();
                        if (r.status_code == 200)
                        {
							all_activity = nlohmann::json::parse(r.text)["learning_activities"];
                        }
						// 开始刷课
						std::cout << "开始刷取课程\"" << courses[i]["display_name"].get<std::string>().c_str() << "\"..." << std::endl;
                        for (auto& activity : all_activity) {
                            // 获取任务信息
							std::string name = activity["title"].get<std::string>();
							long long activity_id = activity["id"].get<long long>();
							std::string type = activity["type"].get<std::string>();
                            // 如果是文件则获取文件列表
							nlohmann::json files = activity["uploads"];
							// 查找是否学习过以及进度
                            if (all_completeness[std::to_string(activity_id)] == "full") {
                                std::cout << "课程\"" << name << "\"已学完，跳过。" << std::endl;
                                continue; // 已学完跳过
                            }
                            // 开始刷课逻辑
                            session->SetUrl("https://lms.ouchn.cn/api/course/activities-read/" + std::to_string(activity_id));
							if (type == "online_video") {
                                session->SetHeader(cpr::Header{ {"Content-Type", "application/json"} });
                                nlohmann::json body;
								body["start"] = 0;
								body["end"] = 9999;
                                session->SetBody(cpr::Body{ body.dump() });
                                auto r1 = session->Post();
                                std::cout << r1.text;
								std::cout << "正在刷取课程\"" << name.c_str() << "\"..." << std::endl;
                            }
                            else if (type == "material") {
                                for (auto& file : files) {
                                    long long file_id = file["id"].get<long long>();
                                    nlohmann::json body;
                                    body["upload_id"] = file_id;
                                    session->SetHeader(cpr::Header{ {"Content-Type", "application/json"} });
                                    session->SetBody(cpr::Body{ body.dump()});
                                    auto r1 = session->Post();
									std::cout << "正在刷取课程\"" << name.c_str() << "\"的文件..." << std::endl;
                                }
                            }else
                                session->Post();
							// 防止屏蔽请求，间隔8秒刷一次
                            std::this_thread::sleep_for(std::chrono::seconds(8));
                        }
                        // 开始刷课程时长
                        session->SetUrl("https://lms.ouchn.cn/statistics/api/user-visits");
                        if (!std::string(courseValue[i][1]).empty())
                        {
                            nlohmann::json visit_payload = {
                                {"user_id", userId},
                                {"course_id", course_id},
                                {"visit_duration", std::stoll(courseValue[i][1])}
                            };
							session->SetHeader(cpr::Header{ {"Content-Type", "text/plain;charset=UTF-8"} });
                            session->SetBody(cpr::Body{ visit_payload.dump() });
                            session->Post();
                        }
						// 开始刷观看次数
                        if (!std::string(courseValue[i][0]).empty())
                            for (int o = 0; o < std::stoi(courseValue[i][0]) - 1; o++)
                            {
                                nlohmann::json visit_payload = {
                                {"user_id", userId},
                                {"course_id", course_id},
                                {"visit_duration", 0}
                                };
                                session->SetBody(cpr::Body{ visit_payload.dump() });
                                session->Post();
                                // 防止屏蔽请求，间隔8秒刷一次
                                std::this_thread::sleep_for(std::chrono::seconds(8));
                            }
                        // 开始刷音视频时长
                        session->SetUrl("https://lms.ouchn.cn/statistics/api/user-visits");
                        if (!std::string(courseValue[i][3]).empty())
                        {
                            nlohmann::json visit_payload = {
                                {"user_id", userId},
                                {"course_id", course_id},
                                {"activity_type", "online_video"},
                                {"visit_duration", std::stoll(courseValue[i][1])}
                            };
                            session->SetBody(cpr::Body{ visit_payload.dump() });
                            session->Post();
                        }
						// 开始刷音视频观看次数
                        if (!std::string(courseValue[i][2]).empty())
                            for (int o = 0; o < std::stoi(courseValue[i][0]) - 1; o++)
                            {
                                nlohmann::json visit_payload = {
                                {"user_id", userId},
                                {"course_id", course_id},
                                {"activity_type", "online_video"},
                                {"visit_duration", 0}
                                };
                                session->SetBody(cpr::Body{ visit_payload.dump() });
                                session->Post();
                                // 防止屏蔽请求，间隔8秒刷一次
                                std::this_thread::sleep_for(std::chrono::seconds(8));
                            }
						std::cout << courses[i]["name"].get<std::string>() << "课程刷取完成！" << std::endl;
                    }
                    });
                run_thread.detach();
            }
			ImGui::PopStyleColor(4);
			ImGui::PopStyleVar(2);
        }
    }

    ImGui::End();
}

void App::LoginWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    // 1. 设置窗口位置和大小（固定 240x320）
    ImVec2 win_size(240, 320);
    ImGui::SetNextWindowSize(win_size, ImGuiCond_Always);
    ImGui::SetNextWindowPos(
        ImVec2((io.DisplaySize.x - win_size.x) * 0.5f,
            (io.DisplaySize.y - win_size.y) * 0.5f),
        ImGuiCond_Always
    );

    // 2. 准备样式（必须在 Begin 之前）
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25, 25));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(255, 255, 255, 255)); // 纯白背景

    // 3. 开启窗口 (flags 确保没有标题栏和多余装饰)
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar;

    // 只有 Begin 返回 true 时才进行绘制
    ImGui::Begin("LoginWindow", nullptr, window_flags);
    bool shouldCloseWebWindow = false;
    {
        // ----- 标题 -----
        ImGui::SetCursorPosY(35);
        ImGui::PushFont(imGuiLayer->fontLarge);
        const char* title = "登 录";
        float title_x = (win_size.x - ImGui::CalcTextSize(title).x) * 0.5f;
        ImGui::SetCursorPosX(title_x);
        // 使用超深灰色（近黑），确保看得清
        ImGui::TextColored(ImVec4(0.f, 0.f, 0.f, 1.0f), title);
        ImGui::PopFont();

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

        // ----- 输入框逻辑 -----
        auto DrawMinimalInput = [&](const char* id, const char* hint, char* buf, int size, bool is_pass) {
            // Label 颜色（中灰色）
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), hint);

            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));              // 背景透明
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));               // 无框
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(20, 20, 20, 255));            // 输入文字深黑色
            ImGui::PushStyleColor(ImGuiCol_InputTextCursor, IM_COL32(20, 20, 20, 255)); // 强制光标为黑色

            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImGui::PushItemWidth(-1.0f);
            ImGui::InputText(id, buf, size, is_pass ? ImGuiInputTextFlags_Password : 0);
            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();
            ImGui::PopItemWidth();
            ImU32 line_color = hovered || active ? IM_COL32(10, 10, 10, 255) : IM_COL32(210, 210, 210, 255);
            // 绘制底部的横线
            ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, ImGui::GetItemRectMax().y + 2);
            draw_list->AddLine(ImVec2(p0.x, p1.y), p1, line_color, 1.5f);

            ImGui::PopStyleColor(4);
            ImGui::Spacing();
            };

        ImGui::SetCursorPosY(90);
        DrawMinimalInput("##user", "学号", username, 64, false);
        ImGui::Spacing();
        DrawMinimalInput("##pass", "密码", password, 64, true);

        // ----- 圆角按钮 (通过 PushStyle 实现) -----
        ImGui::SetCursorPosY(220);
        ImGui::SetCursorPosX(25);

        // 设置按钮圆角
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        // 设置按钮颜色 (模仿渐变色中的主色调)
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(180, 190, 240, 255));        // 正常状态
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 200, 250, 255)); // 悬停状态
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(160, 170, 230, 255));  // 点击状态
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));          // 文字白色

        if (ImGui::Button(login_btn_text.c_str(), ImVec2(win_size.x - 50, 40)) && !loginInProgress) {
            // 登录逻辑
            login_btn_text = "登录中...";

            // 登录失败，可能是因为验证码等原因，调用webview2手动登录一次获取cookie
            std::thread([this]() {
                loginInProgress = true;
                session->SetUrl("https://iam.pt.ouchn.cn/am/oauth2/doLogin");
                session->SetPayload(cpr::Payload{ {"name", username}, {"pwd", password} });
                session->SetHeader(cpr::Header{ {"Content-Type", "application/x-www-form-urlencoded"} });
                session->SetUserAgent(cpr::UserAgent{ "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36" });
                cpr::Response r = session->Post();
                session->SetUrl("https://lms.ouchn.cn/user/courses");
                r = session->Get();
                std::regex id_regex(R"(id: (\d+))");
                std::regex name_regex(R"##(name: "(.*)")##");
                std::smatch id_match;
                std::smatch name_match;
                if (std::regex_search(r.text, id_match, id_regex) && std::regex_search(r.text, name_match, name_regex)) {
                    userId = std::stoll(id_match[1]);
                    name = name_match[1].str();
                    loggedIn = true;
                    loginInProgress = false;
                    login_btn_text = "登录";
                    nlohmann::json user_info = {
                        {"name", username},
                        {"pwd", password}
                    };
                    std::ofstream ofs("user_info.json");
                    ofs << user_info.dump(4);
                    ofs.close();
                    return;
                }
				// 登录失败，调用webview2手动登录一次获取cookie
				if (!r.text.contains("用户信息为空")) // 过滤用户名或密码错误的情况
                {
                    ShowWindow(imGuiLayer->webHWND, SW_SHOW);
                    SetWindowPos(imGuiLayer->webHWND, NULL, 100, 100, 800, 600, SWP_NOZORDER);
                }
                }).detach();
        }
        auto title_str = imGuiLayer->WebView.GetCurrentTitle();
        if (title_str == "统一身份认证平台")
        {
            // 使用 nlohmann::json 生成安全的 JS 字面量，防止用户名/密码中含特殊字符导致脚本语法错误或注入
            std::string jsUser = nlohmann::json(std::string(username)).dump();
            std::string jsPass = nlohmann::json(std::string(password)).dump();
            std::string js = std::format("var el = document.querySelector('input[name=\"name\"]'); if (el) el.value = {};", jsUser);
            js += std::format("var ep = document.querySelector('input[name=\"pwd\"]'); if (ep) ep.value = {};", jsPass);

            // 将 UTF-8 JS 字符串转换为 UTF-16 (wide string) 以传递给 WebView2
            int required = MultiByteToWideChar(CP_UTF8, 0, js.c_str(), -1, nullptr, 0);
            if (required > 0) {
                std::wstring wjs(required - 1, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, js.c_str(), -1, &wjs[0], required);
                imGuiLayer->WebView.Get()->ExecuteScript(wjs.c_str(), nullptr);
            }
        }
        else if (title_str.contains("我的课程"))
        {
            std::string cookies = imGuiLayer->WebView.GetCookieString(L"https://lms.ouchn.cn/");
            if (!cookies.empty()) {
                session->SetHeader(cpr::Header{ {"Cookie", cookies} });
                session->SetUrl("https://lms.ouchn.cn/user/courses");
                auto r = session->Get();
                std::regex id_regex(R"(id: (\d+))");
                std::regex name_regex(R"##(name: "(.*)")##");
                std::smatch id_match;
                std::smatch name_match;
                if (std::regex_search(r.text, id_match, id_regex) && std::regex_search(r.text, name_match, name_regex)) {
                    userId = std::stoll(id_match[1]);
                    name = name_match[1].str();
                    loggedIn = true;
                    loginInProgress = false;
                    login_btn_text = "登录";
                    nlohmann::json user_info = {
                        {"name", username},
                        {"pwd", password}
                    };
                    std::ofstream ofs("user_info.json");
                    ofs << user_info.dump(4);
                    ofs.close();
                    shouldCloseWebWindow = true;
                }
            }
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(1); // 弹出 FrameRounding
    }
    // 4. 结束窗口
    ImGui::End();

    // 5. 统一清理样式（必须在 End 之后）
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    if (shouldCloseWebWindow && imGuiLayer->webHWND) {
        DestroyWindow(imGuiLayer->webHWND);
        imGuiLayer->webHWND = nullptr;
    }
}
