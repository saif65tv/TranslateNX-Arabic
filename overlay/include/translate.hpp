#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct TranslationRegion {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    std::string original;
    std::string translated;
};

struct TranslateResult {
    bool        success = false;
    std::string translatedText;
    std::vector<std::string> translatedLines;
    std::vector<TranslationRegion> regions;
    std::string errorMsg;
};

namespace Translate {
    TranslateResult runMyMemory(const std::vector<std::string>& lines,
                            const std::string& langPair = "ja|tr");

    TranslateResult runDeepL(const std::vector<std::string>& lines,
                               const std::string& apiKey,
                               const std::string& sourceLang,
                               const std::string& targetLang);

    TranslateResult runGoogleCloud(const std::vector<std::string>& lines,
                               const std::string& apiKey,
                               const std::string& sourceLang,
                               const std::string& targetLang);

    TranslateResult runGeminiAI(const std::vector<uint8_t>& jpegData,
                                const std::string& apiKey,
                                const std::string& targetLang,
                                const std::string& model,
                                const std::string& thinking);
}
