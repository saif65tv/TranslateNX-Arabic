#pragma once
#include <string>

enum class OcrApi {
    OcrSpace,
    GoogleVision
};

enum class TranslateApi {
    MyMemory,
    DeepL,
    GoogleCloud
};

enum class AppMode {
    Classic,
    AI
};

enum class AiApi {
    Puter,
    Gemini
};

struct Config {
    AppMode       appMode      = AppMode::AI;
    AiApi         aiApi        = AiApi::Gemini;

    OcrApi        ocrApi       = OcrApi::OcrSpace;
    TranslateApi  translateApi = TranslateApi::MyMemory;

    std::string   srcLang = "ja";
    std::string   dstLang = "tr";
    std::string   uiLang = "tr";

    std::string   ocrApiKey;
    std::string   visionApiKey;
    std::string   deeplApiKey;
    std::string   googleTransApiKey;

    std::string   puterApiKey;
    std::string   geminiApiKey;

    std::string   geminiModel = "gemini-3.1-flash-lite";
    std::string   geminiThinking = "minimal";
};

namespace ConfigManager {
    Config load();
    void save(const Config& cfg);
    bool promptKeyboard(const char* title, std::string& out);
}
