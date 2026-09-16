#include "config.hpp"
#include <switch.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <cctype>

static const char* CONFIG_PATH = "sdmc:/config/translate/config.ini";

// ─── Yardımcı: basit INI okuma ─────────────────────────────────────────────
static std::string iniGet(const char* path, const char* key) {
    FILE* f = fopen(path, "r");
    if (!f) return {};
    char line[512];
    std::string k(key);
    while (fgets(line, sizeof(line), f)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        std::string lkey(line, eq - line);
        // trim
        while (!lkey.empty() && (lkey.back() == ' ' || lkey.back() == '\t')) lkey.pop_back();
        if (lkey != k) continue;
        std::string val(eq + 1);
        while (!val.empty() && (val.back() == '\n' || val.back() == '\r' || val.back() == ' ')) val.pop_back();
        fclose(f);
        return val;
    }
    fclose(f);
    return {};
}

namespace ConfigManager {

Config load() {
    FILE* f = fopen(CONFIG_PATH, "r");
    if (!f) {
        // Auto-create
        mkdir("sdmc:/config", 0777);
        mkdir("sdmc:/config/translate", 0777);
        f = fopen(CONFIG_PATH, "w");
        if (f) {
            fprintf(f, "ocr_api=vision\n");
            fprintf(f, "translate_api=deepl\n");
            fprintf(f, "src_lang=en\n");
            fprintf(f, "dst_lang=tr\n");
            fprintf(f, "ui_lang=en\n");
            fprintf(f, "app_mode=classic\n");
            fprintf(f, "ai_api=gemini\n");
            fprintf(f, "ocr_api_key=\n");
            fprintf(f, "vision_api_key=\n");
            fprintf(f, "deepl_api_key=\n");
            fprintf(f, "google_trans_api_key=\n");
            fprintf(f, "puter_api_key=\n");
            fprintf(f, "gemini_api_key=\n");
            fprintf(f, "gemini_model=gemini-3.1-flash-lite\n");
            fprintf(f, "gemini_thinking=minimal\n");
            fclose(f);
        }
    } else {
        fclose(f);
    }

    Config cfg;

    std::string modeStr = iniGet(CONFIG_PATH, "app_mode");
    cfg.appMode = (modeStr == "ai") ? AppMode::AI : AppMode::Classic;

    std::string aiStr = iniGet(CONFIG_PATH, "ai_api");
    cfg.aiApi = (aiStr == "puter") ? AiApi::Puter : AiApi::Gemini;

    std::string ocrStr = iniGet(CONFIG_PATH, "ocr_api");
    if (ocrStr == "vision") cfg.ocrApi = OcrApi::GoogleVision;
    else cfg.ocrApi = OcrApi::OcrSpace;

    std::string transStr = iniGet(CONFIG_PATH, "translate_api");
    if (transStr == "deepl") cfg.translateApi = TranslateApi::DeepL;
    else if (transStr == "googlecloud") cfg.translateApi = TranslateApi::GoogleCloud;
    else cfg.translateApi = TranslateApi::MyMemory;

    cfg.srcLang          = iniGet(CONFIG_PATH, "source_lang");
    if (cfg.srcLang.empty()) cfg.srcLang = iniGet(CONFIG_PATH, "src_lang"); // Fallback for old configs

    cfg.dstLang          = iniGet(CONFIG_PATH, "target_lang");
    if (cfg.dstLang.empty()) cfg.dstLang = iniGet(CONFIG_PATH, "dst_lang");

    cfg.uiLang           = iniGet(CONFIG_PATH, "ui_lang");
    
    cfg.ocrApiKey        = iniGet(CONFIG_PATH, "ocr_api_key");
    cfg.visionApiKey     = iniGet(CONFIG_PATH, "vision_api_key");
    
    cfg.deeplApiKey      = iniGet(CONFIG_PATH, "deepl_api_key");
    cfg.googleTransApiKey = iniGet(CONFIG_PATH, "google_trans_api_key");
    cfg.puterApiKey       = iniGet(CONFIG_PATH, "puter_api_key");
    cfg.geminiApiKey      = iniGet(CONFIG_PATH, "gemini_api_key");
    cfg.geminiModel       = iniGet(CONFIG_PATH, "gemini_model");
    cfg.geminiThinking    = iniGet(CONFIG_PATH, "gemini_thinking");

    if (cfg.srcLang.empty()) cfg.srcLang = "en";
    if (cfg.dstLang.empty()) cfg.dstLang = "tr";
    if (cfg.uiLang.empty())  cfg.uiLang = "en";
    if (cfg.geminiModel.empty()) cfg.geminiModel = "gemini-3.1-flash-lite";
    if (cfg.geminiThinking.empty()) cfg.geminiThinking = "minimal";
    
    for (auto & c: cfg.srcLang) c = toupper((unsigned char)c);
    for (auto & c: cfg.dstLang) c = toupper((unsigned char)c);
    for (auto & c: cfg.uiLang) c = toupper((unsigned char)c);
    
    return cfg;
}

void save(const Config& cfg) {
    mkdir("sdmc:/config", 0777);
    mkdir("sdmc:/config/translate", 0777);

    FILE* f = fopen(CONFIG_PATH, "r");
    std::vector<std::string> lines;
    bool foundOcrApi = false, foundTransApi = false, foundSrc = false, foundDst = false, foundUi = false;
    bool hasAppMode = false, hasAiApi = false;
    bool hasPuterKey = false, hasGeminiKey = false;
    bool hasGeminiModel = false, hasGeminiThinking = false;
    
    const char* appModeStr = (cfg.appMode == AppMode::AI) ? "ai" : "classic";
    const char* aiApiStr   = (cfg.aiApi == AiApi::Puter) ? "puter" : "gemini";

    const char* ocrStr = "ocrspace";
    if (cfg.ocrApi == OcrApi::GoogleVision) ocrStr = "vision";

    const char* transStr = "mymemory";
    if (cfg.translateApi == TranslateApi::DeepL) transStr = "deepl";
    else if (cfg.translateApi == TranslateApi::GoogleCloud) transStr = "googlecloud";

    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            std::string s(line);
            if (!s.empty() && s.back() != '\n') s += '\n';
            char* eq = strchr(line, '=');
            if (eq) {
                std::string lkey(line, eq - line);
                while (!lkey.empty() && (lkey.back() == ' ' || lkey.back() == '\t')) lkey.pop_back();
                
                std::string sl = cfg.srcLang; for(auto& c:sl) c=tolower((unsigned char)c);
                std::string dl = cfg.dstLang; for(auto& c:dl) c=tolower((unsigned char)c);
                std::string ul = cfg.uiLang;  for(auto& c:ul) c=tolower((unsigned char)c);
                
                if (lkey == "app_mode") { lines.push_back("app_mode=" + std::string(appModeStr) + "\n"); continue; }
                if (lkey == "ai_api") { lines.push_back("ai_api=" + std::string(aiApiStr) + "\n"); continue; }

                if (lkey == "ocr_api") { lines.push_back("ocr_api=" + std::string(ocrStr) + "\n"); foundOcrApi = true; continue; }
                if (lkey == "translate_api") { lines.push_back("translate_api=" + std::string(transStr) + "\n"); foundTransApi = true; continue; }
                if (lkey == "src_lang" || lkey == "source_lang") { lines.push_back("src_lang=" + sl + "\n"); foundSrc = true; continue; }
                if (lkey == "dst_lang" || lkey == "target_lang") { lines.push_back("dst_lang=" + dl + "\n"); foundDst = true; continue; }
                if (lkey == "ui_lang") { lines.push_back("ui_lang=" + ul + "\n"); foundUi = true; continue; }
            if (lkey == "gemini_model") { lines.push_back("gemini_model=" + cfg.geminiModel + "\n"); hasGeminiModel = true; continue; }
            if (lkey == "gemini_thinking") { lines.push_back("gemini_thinking=" + cfg.geminiThinking + "\n"); hasGeminiThinking = true; continue; }
            }
            lines.push_back(s);
        }
        fclose(f);
    }
    
    std::string sl = cfg.srcLang; for(auto& c:sl) c=tolower((unsigned char)c);
    std::string dl = cfg.dstLang; for(auto& c:dl) c=tolower((unsigned char)c);
    std::string ul = cfg.uiLang;  for(auto& c:ul) c=tolower((unsigned char)c);

    if (!foundOcrApi) lines.push_back("ocr_api=" + std::string(ocrStr) + "\n");
    if (!foundTransApi) lines.push_back("translate_api=" + std::string(transStr) + "\n");
    if (!foundSrc) lines.push_back("src_lang=" + sl + "\n");
    if (!foundDst) lines.push_back("dst_lang=" + dl + "\n");
    if (!foundUi) lines.push_back("ui_lang=" + ul + "\n");

    for (const auto& line : lines) {
        if (line.rfind("app_mode=", 0) == 0) hasAppMode = true;
        if (line.rfind("ai_api=", 0) == 0) hasAiApi = true;
        if (line.rfind("puter_api_key=", 0) == 0) hasPuterKey = true;
        if (line.rfind("gemini_api_key=", 0) == 0) hasGeminiKey = true;
        if (line.rfind("gemini_model=", 0) == 0) hasGeminiModel = true;
        if (line.rfind("gemini_thinking=", 0) == 0) hasGeminiThinking = true;
    }

    if (!hasAppMode) lines.push_back("app_mode=" + std::string(appModeStr) + "\n");
    if (!hasAiApi) lines.push_back("ai_api=" + std::string(aiApiStr) + "\n");
    if (!hasPuterKey) lines.push_back("puter_api_key=" + cfg.puterApiKey + "\n");
    if (!hasGeminiKey) lines.push_back("gemini_api_key=" + cfg.geminiApiKey + "\n");
    if (!hasGeminiModel) lines.push_back("gemini_model=" + cfg.geminiModel + "\n");
    if (!hasGeminiThinking) lines.push_back("gemini_thinking=" + cfg.geminiThinking + "\n");

    f = fopen(CONFIG_PATH, "w");
    if (f) {
        for (const auto& l : lines) {
            fprintf(f, "%s", l.c_str());
        }
        fclose(f);
    }
}

} // namespace ConfigManager
