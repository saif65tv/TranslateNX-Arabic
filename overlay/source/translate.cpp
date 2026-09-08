#include "translate.hpp"
#include "http_client.hpp"
#include <switch.h>
#include <cstring>
#include <cJSON.h>

// ─── URL encode (MyMemory için) ────────────────────────────────────────────
static std::string urlEncode(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 3);
    static const char* hex = "0123456789ABCDEF";
    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += static_cast<char>(c);
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 0xF];
        }
    }
    return out;
}



namespace Translate {

TranslateResult runMyMemory(const std::vector<std::string>& lines, const std::string& langPair) {
    TranslateResult result;
    if (lines.empty()) {
        result.errorMsg = "Çeviri: OCR'dan metin gelmedi";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    std::string mergedText;
    for (size_t i = 0; i < lines.size(); ++i) {
        mergedText += lines[i];
        if (i < lines.size() - 1) mergedText += " \n \n ";
    }

    std::string safeText = mergedText.size() > 500 ? mergedText.substr(0, 500) : mergedText;
    std::string url = "https://api.mymemory.translated.net/get?q=" +
                      urlEncode(safeText) +
                      "&langpair=" + langPair +
                      "&de=switchocr@example.com";

    HttpResponse resp;
    for (int retry = 0; retry < 3; retry++) {
        resp = HttpClient::get(url);
        if (resp.ok()) {
            break;
        }
        svcSleepThread(1000000000ull); // 1 saniye bekle ve tekrar dene
    }

    if (!resp.ok()) {
        if (resp.statusCode == 0) {
            result.errorMsg = "MyMemory: Bağlantı kurulamadı — " + resp.errorStr;
        } else if (resp.statusCode == 429) {
            result.errorMsg = "MyMemory: Günlük ücretsiz kota doldu (HTTP 429) — günde 1000 karakter";
        } else if (resp.statusCode == 503) {
            result.errorMsg = "MyMemory: Servis geçici olarak kullanılamıyor (HTTP 503) — daha sonra tekrar deneyin";
        } else {
            result.errorMsg = "MyMemory: HTTP " + std::to_string(resp.statusCode);
        }
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    cJSON* root = cJSON_Parse(resp.body.c_str());
    if (root) {
        cJSON* respData = cJSON_GetObjectItem(root, "responseData");
        if (respData) {
            cJSON* translated = cJSON_GetObjectItem(respData, "translatedText");
            if (translated && cJSON_IsString(translated)) {
                result.translatedText = translated->valuestring;
            }
        }
        cJSON_Delete(root);
    }

    if (result.translatedText.empty()) {
        result.errorMsg = "MyMemory: Boş yanıt döndü — API sınırı aşıldı olabilir";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    if (result.translatedText.find("QUERY LENGTH") != std::string::npos) {
        result.errorMsg = "MyMemory: Metin çok uzun (500 karakter sınırı aşıldı)";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    // Split by \n \n 
    size_t pos = 0;
    std::string token;
    std::string delimiter = " \n \n ";
    std::string s = result.translatedText;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        result.translatedLines.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    result.translatedLines.push_back(s);

    // Eğer sayı uyuşmuyorsa, düz metin gibi kabul edeceğiz
    result.success = true;
        for (auto& line : result.translatedLines) {
        }
        return result;
}


static std::string xmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 32);

    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;";  break;
            case '<': out += "&lt;";   break;
            case '>': out += "&gt;";   break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += c; break;
        }
    }

    return out;
}

static std::string xmlUnescape(const std::string& s) {
    std::string out = s;

    auto replaceAll = [&out](const std::string& from, const std::string& to) {
        size_t pos = 0;
        while ((pos = out.find(from, pos)) != std::string::npos) {
            out.replace(pos, from.length(), to);
            pos += to.length();
        }
    };

    replaceAll("&lt;", "<");
    replaceAll("&gt;", ">");
    replaceAll("&quot;", "\"");
    replaceAll("&apos;", "'");
    replaceAll("&amp;", "&");

    return out;
}

static bool deepLEndsSentence(const std::string& text) {
    if (text.empty())
        return false;

    size_t end = text.size();

    while (end > 0 &&
           (text[end - 1] == ' ' ||
            text[end - 1] == '\t' ||
            text[end - 1] == '\r' ||
            text[end - 1] == '\n')) {
        --end;
    }

    if (end == 0)
        return false;

    const char c = text[end - 1];

    return c == '.' ||
           c == '!' ||
           c == '?' ||
           c == ':' ||
           c == ';';
}

static bool parseDeepLTaggedText(
    const std::string& translated,
    std::vector<std::string>& out
) {
    size_t pos = 0;

    while (true) {
        const size_t open = translated.find("<line", pos);
        if (open == std::string::npos)
            break;

        const size_t openEnd = translated.find('>', open);
        if (openEnd == std::string::npos)
            return false;

        const size_t close = translated.find("</line>", openEnd + 1);
        if (close == std::string::npos)
            return false;

        std::string value =
            translated.substr(openEnd + 1, close - (openEnd + 1));

        out.push_back(xmlUnescape(value));

        pos = close + 7;
    }

    return !out.empty();
}

TranslateResult runDeepL(
                             const std::vector<std::string>& lines,
                             const std::string& apiKey,
                             const std::string& sourceLang,
                             const std::string& targetLang,
                             const std::string& context) {
    TranslateResult result;

    if (lines.empty() || apiKey.empty()) {
        result.errorMsg = "DeepL: API key girilmemiş";
        return result;
    }

    /*
     * Group OCR lines into sentence-sized translation blocks.
     *
     * The UI still receives exactly one translation per original OCR line.
     * DeepL, however, sees related lines together inside the same text item.
     *
     * Example:
     *   Line 0: New roads that were under construction had to
     *   Line 1: be abandoned, and Ichiru Manor was buried
     *   Line 2: under a slew of rock, resulting in numerous
     *   Line 3: fatalities.
     *
     * These are sent together, with XML tags preserving the four outputs.
     */
    std::vector<std::string> blocks;
    std::string currentBlock;

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];

        if (!currentBlock.empty())
            currentBlock += " ";

        currentBlock +=
            "<line id=\"" +
            std::to_string(i) +
            "\">" +
            xmlEscape(line) +
            "</line>";

        if (deepLEndsSentence(line) || currentBlock.size() > 1800) {
            blocks.push_back(currentBlock);
            currentBlock.clear();
        }
    }

    if (!currentBlock.empty())
        blocks.push_back(currentBlock);

    std::string body =
        "target_lang=" + targetLang;

    if (!sourceLang.empty()) {
        body += "&source_lang=" + sourceLang;
    }

    /*
     * Each block is one DeepL text item.
     * XML tags are used only to preserve the original OCR item mapping.
     */
    for (const auto& block : blocks) {
        body += "&text=" + urlEncode(block);
    }

    if (!context.empty()) {
        body += "&context=" + urlEncode(context);
    }

    body += "&split_sentences=1";
    body += "&tag_handling=xml";
    body += "&tag_handling_version=v2";
    body += "&non_splitting_tags=line";

    HttpResponse resp;

    for (int retry = 0; retry < 3; retry++) {
        resp = HttpClient::post(
            "https://api-free.deepl.com/v2/translate",
            body,
            {
                "Authorization: DeepL-Auth-Key " + apiKey,
                "Content-Type: application/x-www-form-urlencoded"
            }
        );

        if (resp.ok())
            break;

        svcSleepThread(1000000000ull);
    }

    if (!resp.ok()) {
        if (resp.statusCode == 0) {
            result.errorMsg =
                "DeepL: Bağlantı kurulamadı — " + resp.errorStr;
        } else if (resp.statusCode == 400) {
            result.errorMsg =
                "DeepL: Geçersiz istek (HTTP 400)";
        } else if (resp.statusCode == 403) {
            result.errorMsg =
                "DeepL: Geçersiz API key (HTTP 403)";
        } else if (resp.statusCode == 413) {
            result.errorMsg =
                "DeepL: Metin çok büyük (HTTP 413)";
        } else if (resp.statusCode == 429) {
            result.errorMsg =
                "DeepL: İstek oran sınırı aşıldı (HTTP 429)";
        } else if (resp.statusCode == 456) {
            result.errorMsg =
                "DeepL: Aylık karakter kotası doldu (HTTP 456)";
        } else {
            result.errorMsg =
                "DeepL: HTTP " + std::to_string(resp.statusCode);
        }

        return result;
    }

    cJSON* root = cJSON_Parse(resp.body.c_str());

    if (!root) {
        result.errorMsg = "DeepL: JSON yanıtı çözülemedi";
        return result;
    }

    cJSON* translations =
        cJSON_GetObjectItem(root, "translations");

    if (!cJSON_IsArray(translations)) {
        cJSON_Delete(root);
        result.errorMsg = "DeepL: translations alanı bulunamadı";
        return result;
    }

    const int count = cJSON_GetArraySize(translations);

    for (int i = 0; i < count; ++i) {
        cJSON* tr =
            cJSON_GetArrayItem(translations, i);

        cJSON* txt =
            cJSON_GetObjectItem(tr, "text");

        if (!txt || !cJSON_IsString(txt)) {
            cJSON_Delete(root);
            result.translatedLines.clear();
            result.errorMsg =
                "DeepL: ترجمة غير صالحة من الخادم";
            return result;
        }

        std::vector<std::string> blockTranslations;

        if (!parseDeepLTaggedText(
                txt->valuestring,
                blockTranslations)) {

            cJSON_Delete(root);
            result.translatedLines.clear();
            result.errorMsg =
                "DeepL: تعذر استرجاع ترجمة أسطر OCR";
            return result;
        }

        for (const auto& translated : blockTranslations) {
            result.translatedLines.push_back(translated);
            result.translatedText += translated + "\n";
        }
    }

    cJSON_Delete(root);

    if (result.translatedLines.size() != lines.size()) {
        result.translatedLines.clear();
        result.translatedText.clear();
        result.errorMsg =
            "DeepL: عدد الترجمات لا يطابق عدد أسطر OCR";
        return result;
    }

    result.success = true;
    return result;
}

// ─── Google Cloud Translation API ─────────────────────────────────────────
TranslateResult runGoogleCloud(const std::vector<std::string>& lines, const std::string& apiKey, const std::string& sourceLang, const std::string& targetLang) {
    TranslateResult result;
    if (apiKey.empty()) {
        result.errorMsg = "Google Cloud Translate: API key girilmemiş";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }
    if (lines.empty()) {
        result.errorMsg = "Google Cloud Translate: Çevrilecek metin yok";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    std::string url = "https://translation.googleapis.com/language/translate/v2?key=" + apiKey;

    // Body: {"q": ["l1", "l2"], "target": "tr", "source": "ja", "format": "text"}
    cJSON* root = cJSON_CreateObject();
    cJSON* qArr = cJSON_CreateArray();
    for (const auto& line : lines) {
        cJSON_AddItemToArray(qArr, cJSON_CreateString(line.c_str()));
    }
    cJSON_AddItemToObject(root, "q", qArr);
    cJSON_AddStringToObject(root, "target", targetLang.c_str());
    if (!sourceLang.empty() && sourceLang != "auto") {
        cJSON_AddStringToObject(root, "source", sourceLang.c_str());
    }
    cJSON_AddStringToObject(root, "format", "text");

    char* jsonStr = cJSON_PrintUnformatted(root);
    std::string body(jsonStr);
    free(jsonStr);
    cJSON_Delete(root);

    std::vector<std::string> headers = {
        "Content-Type: application/json"
    };

    auto resp = HttpClient::post(url, body, headers);
    if (!resp.ok()) {
        if (resp.statusCode == 0) {
            result.errorMsg = "Google Cloud Translate: Bağlantı kurulamadı — " + resp.errorStr;
        } else if (resp.statusCode == 400) {
            result.errorMsg = "Google Cloud Translate: Geçersiz istek (HTTP 400) — dil kodu yanlış olabilir";
        } else if (resp.statusCode == 401) {
            result.errorMsg = "Google Cloud Translate: Kimlik doğrulama hatası (HTTP 401) — API key geçersiz";
        } else if (resp.statusCode == 403) {
            result.errorMsg = "Google Cloud Translate: Erişim reddedildi (HTTP 403) — Cloud Translation API etkinleştirilmemiş?";
        } else if (resp.statusCode == 429) {
            result.errorMsg = "Google Cloud Translate: İstek sınırı aşıldı (HTTP 429) — kota doldu";
        } else if (resp.statusCode == 500) {
            result.errorMsg = "Google Cloud Translate: Google sunucu hatası (HTTP 500) — daha sonra tekrar deneyin";
        } else if (resp.statusCode == 503) {
            result.errorMsg = "Google Cloud Translate: Servis geçici olarak kullanılamıyor (HTTP 503)";
        } else {
            result.errorMsg = "Google Cloud Translate: HTTP " + std::to_string(resp.statusCode) + " — " + resp.errorStr;
        }
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    // Response: {"data": {"translations": [{"translatedText": "ceviri"}]}}
    cJSON* respObj = cJSON_Parse(resp.body.c_str());
    if (!respObj) {
        result.errorMsg = "Google Cloud Translate: Sunucu yanıtı işlenemedi (JSON parse hatası)";
        for (auto& line : result.translatedLines) {
        }
        return result;
    }

    cJSON* data = cJSON_GetObjectItem(respObj, "data");
    if (data) {
        cJSON* translations = cJSON_GetObjectItem(data, "translations");
        if (cJSON_IsArray(translations)) {
            int count = cJSON_GetArraySize(translations);
            for (int i = 0; i < count; i++) {
                cJSON* item = cJSON_GetArrayItem(translations, i);
                cJSON* textNode = cJSON_GetObjectItem(item, "translatedText");
                if (textNode && cJSON_IsString(textNode) && textNode->valuestring) {
                    result.translatedLines.push_back(textNode->valuestring);
                } else {
                    result.translatedLines.push_back("");
                }
            }
        }
    }
    cJSON_Delete(respObj);

    for (const auto& l : result.translatedLines) {
        result.translatedText += l + "\n";
    }

    if (result.translatedLines.empty()) {
        result.errorMsg = "Google Cloud Translate: Görüntüde metin bulunamadı";
    } else {
        result.success = true;
    }
        for (auto& line : result.translatedLines) {
        }
        return result;
}

} // namespace Translate
