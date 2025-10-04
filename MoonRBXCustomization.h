#pragma once

#include <string>
#include <vector>
#include <random>
#include <thread>
#include <functional>
#include <mutex>
#include <Windows.h>
#include <dwmapi.h>
#include <chrono>

#pragma comment(lib, "dwmapi.lib")

class MoonRBXCustomization
{
private:
    // -----------------------------
    // Version & Settings
    // -----------------------------
    int moonRbxVersion = 2;
    int moonVersion = 8;
    bool animations = false;
    COLORREF bg;                       // Background color
    COLORREF text;                     // Text color
    std::string moonRbxTextType;       // Text style: BINARY, HEX, EMOJI, etc.
    std::string colorType;             // Color preset
    std::string colorAnimationType;    // Optional animation type

    std::mutex modify; // Thread-safety mutex

    // -----------------------------w
    // Random Generators
    // -----------------------------
    static std::mt19937& rng() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    std::string generate_weird_font(int length = 80) {
        constexpr auto chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";
        std::uniform_int_distribution<int> dist(0, sizeof(chars) - 2);
        std::string out; out.reserve(length);
        for (int i = 0; i < length; ++i) out += chars[dist(rng())];
        return out;
    }

    std::string generate_random_symbols(int length = 80) {
        const std::vector<std::string> symbols = { "🔥","💀","⚡","✨","🎯","💎","🌙","☠️" };
        std::uniform_int_distribution<int> dist(0, symbols.size() - 1);
        std::string out; out.reserve(length);
        for (int i = 0; i < length; ++i) out += symbols[dist(rng())];
        return out;
    }

    std::string generate_random_binary() {
        constexpr int bits = 128;
        std::uniform_int_distribution<int> bit(0, 1);
        std::string out; out.reserve(bits);
        for (int i = 0; i < bits; ++i) out += (bit(rng()) ? '1' : '0');
        return out;
    }

    std::string generate_random_hex(int length = 80) {
        constexpr char hex[] = "0123456789ABCDEF";
        std::uniform_int_distribution<int> dist(0, 15);
        std::string out; out.reserve(length);
        for (int i = 0; i < length; ++i) out += hex[dist(rng())];
        return out;
    }

    // -----------------------------
    // Window Operations
    // -----------------------------
    void changeTitleOfHwnds(const std::vector<HWND>& hwnds, const auto& generator) {
        for (auto hwnd : hwnds) {
            if (!IsWindow(hwnd)) continue;

            std::string textStr = "MoonRBX V2 | " + generator();
            std::wstring wText(textStr.begin(), textStr.end());
            SetWindowText(hwnd, wText.c_str());

            DwmSetWindowAttribute(hwnd, 35, &bg, sizeof(bg));
            DwmSetWindowAttribute(hwnd, 36, &text, sizeof(text));
        }
    }

    void updateWindowColors(const std::vector<HWND>& hwnds) {
        for (auto hwnd : hwnds) {
            DwmSetWindowAttribute(hwnd, 35, &bg, sizeof(bg));
            DwmSetWindowAttribute(hwnd, 36, &text, sizeof(text));
        }
    }

    // -----------------------------
    // Color Management
    // -----------------------------
    void applyColorType() {
        const auto type = colorType;

        if (type == "BRIGHT_BLUE") { bg = RGB(0, 120, 255); text = RGB(255, 255, 255); }
        else if (type == "BRIGHT_RED") { bg = RGB(255, 60, 60); text = RGB(255, 255, 255); }
        else if (type == "BRIGHT_YELLOW") { bg = RGB(255, 240, 0); text = RGB(0, 0, 0); }
        else if (type == "BRIGHT_GREEN") { bg = RGB(0, 255, 128); text = RGB(0, 0, 0); }
        else if (type == "HACKY_DARK") { bg = RGB(0, 0, 0); text = RGB(0, 255, 0); }
        else if (type == "BRIGHT_PURPLE") { bg = RGB(180, 0, 255); text = RGB(255, 255, 255); }
        else if (type == "BRIGHT_ORANGE") { bg = RGB(255, 140, 0); text = RGB(0, 0, 0); }
        else if (type == "ANIMATION") { animations = true; }
        else { bg = RGB(0, 0, 0); text = RGB(255, 255, 255); }

        animations = false; // reset unless explicitly set
    }

public:
    // -----------------------------
    // Public API
    // -----------------------------
    std::function<void()> findCpuInfo; // Callback for CPU usage
    std::function<void()> logging;     // Optional logging callback

    // Thread-safe setters
    void setMoonRbxTextType(const std::string& type) {
        std::lock_guard<std::mutex> lock(modify);
        moonRbxTextType = type;
    }

    void setMoonRbxColorType(const std::string& type) {
        std::lock_guard<std::mutex> lock(modify);
        colorType = type;
    }

    // -----------------------------
    // Main operation loop
    // -----------------------------
    void mainOperation(const std::vector<HWND>& hwnds) {
        std::lock_guard<std::mutex> lock(modify);

        applyColorType();

        if (moonRbxTextType == "HACK_BINARY")
            changeTitleOfHwnds(hwnds, [this]() { return generate_random_binary(); });
        else if (moonRbxTextType == "HACK_HEX")
            changeTitleOfHwnds(hwnds, [this]() { return generate_random_hex(); });
        else if (moonRbxTextType == "WEIRD_FONT")
            changeTitleOfHwnds(hwnds, [this]() { return generate_weird_font(); });
        else if (moonRbxTextType == "EMOJI")
            changeTitleOfHwnds(hwnds, [this]() { return generate_random_symbols(); });
        else {
            findCpuInfo();
            updateWindowColors(hwnds);
            std::this_thread::sleep_for(std::chrono::milliseconds(950));
        }

        if (!animations)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};

