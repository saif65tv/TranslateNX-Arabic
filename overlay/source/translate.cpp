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
        return result;
    }

    if (result.translatedText.find("QUERY LENGTH") != std::string::npos) {
        result.errorMsg = "MyMemory: Metin çok uzun (500 karakter sınırı aşıldı)";
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
    return result;
}

TranslateResult runDeepL(const std::vector<std::string>& lines,
                            const std::string& apiKey,
                            const std::string& sourceLang,
                            const std::string& targetLang) {
    TranslateResult result;
    if (lines.empty() || apiKey.empty()) {
        result.errorMsg = "DeepL: API key girilmemiş";
        return result;
    }

    std::string body = "target_lang=" + targetLang;
    if (!sourceLang.empty()) {
        body += "&source_lang=" + sourceLang;
    }
    for (const auto& line : lines) {
        body += "&text=" + urlEncode(line);
    }

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
        if (resp.ok()) {
            break;
        }
        svcSleepThread(1000000000ull); // 1 saniye bekle
    }

    if (!resp.ok()) {
        if (resp.statusCode == 0) {
            result.errorMsg = "DeepL: Bağlantı kurulamadı — " + resp.errorStr;
        } else if (resp.statusCode == 400) {
            result.errorMsg = "DeepL: Geçersiz istek (HTTP 400) — dil kodu yanlış olabilir";
        } else if (resp.statusCode == 403) {
            result.errorMsg = "DeepL: Geçersiz API key (HTTP 403) — ücretsiz key mi, yoksa PRO mu?";
        } else if (resp.statusCode == 413) {
            result.errorMsg = "DeepL: Metin çok büyük (HTTP 413) — daha az metin gönderin";
        } else if (resp.statusCode == 429) {
            result.errorMsg = "DeepL: İstek oran sınırı aşıldı (HTTP 429) — biraz bekleyip tekrar deneyin";
        } else if (resp.statusCode == 456) {
            result.errorMsg = "DeepL: Aylık karakter kotası doldu (HTTP 456) — plan yenileme tarihinizi kontrol edin";
        } else if (resp.statusCode == 500) {
            result.errorMsg = "DeepL: Sunucu hatası (HTTP 500) — daha sonra tekrar deneyin";
        } else if (resp.statusCode == 503) {
            result.errorMsg = "DeepL: Servis geçici olarak kullanılamıyor (HTTP 503) — daha sonra tekrar deneyin";
        } else if (resp.statusCode == 529) {
            result.errorMsg = "DeepL: Sunucu aşırı yüklendi (HTTP 529) — dakika sonra tekrar deneyin";
        } else {
            result.errorMsg = "DeepL: HTTP " + std::to_string(resp.statusCode);
        }
        return result;
    }

    // DeepL JSON: {"translations":[{"detected_source_language":"EN","text":"Tr1"}, {"text":"Tr2"}]}
    // Biz cJSON ile tüm text alanlarını toplayacağız.
    cJSON* root = cJSON_Parse(resp.body.c_str());
    if (root) {
        cJSON* translations = cJSON_GetObjectItem(root, "translations");
        if (cJSON_IsArray(translations)) {
            int count = cJSON_GetArraySize(translations);
            for (int i = 0; i < count; ++i) {
                cJSON* tr = cJSON_GetArrayItem(translations, i);
                cJSON* txt = cJSON_GetObjectItem(tr, "text");
                if (txt && cJSON_IsString(txt)) {
                    result.translatedLines.push_back(txt->valuestring);
                    result.translatedText += std::string(txt->valuestring) + "\n";
                }
            }
        }
        cJSON_Delete(root);
    }

    if (result.translatedLines.empty()) {
        result.errorMsg = "DeepL: Boş yanıt döndü — API key doğru mu?";
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
        return result;
    }
    if (lines.empty()) {
        result.errorMsg = "Google Cloud Translate: Çevrilecek metin yok";
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
        return result;
    }

    // Response: {"data": {"translations": [{"translatedText": "ceviri"}]}}
    cJSON* respObj = cJSON_Parse(resp.body.c_str());
    if (!respObj) {
        result.errorMsg = "Google Cloud Translate: Sunucu yanıtı işlenemedi (JSON parse hatası)";
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

    return result;
}

static std::string encodeBase64(const std::vector<uint8_t>& data) {
    static const char* chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t value = static_cast<uint32_t>(data[i]) << 16;

        if (i + 1 < data.size())
            value |= static_cast<uint32_t>(data[i + 1]) << 8;

        if (i + 2 < data.size())
            value |= static_cast<uint32_t>(data[i + 2]);

        out += chars[(value >> 18) & 0x3F];
        out += chars[(value >> 12) & 0x3F];
        out += (i + 1 < data.size()) ? chars[(value >> 6) & 0x3F] : '=';
        out += (i + 2 < data.size()) ? chars[value & 0x3F] : '=';
    }

    return out;
}

TranslateResult runGeminiAI(
    const std::vector<uint8_t>& jpegData,
    const std::string& apiKey,
    const std::string& targetLang,
    const std::string& model,
    const std::string& thinking
) {
    TranslateResult result;

    if (jpegData.empty()) {
        result.errorMsg = "Gemini AI: JPEG data is empty";
        return result;
    }

    if (apiKey.empty()) {
        result.errorMsg = "Gemini AI: API key is missing";
        return result;
    }

    std::string image64 = encodeBase64(jpegData);

    const std::string prompt =
        "Analyze this game screenshot. Find ALL visible human-readable text "
        "regions, including main dialogue, subtitles, menus, labels, buttons, "
        "small side text, status text, and other peripheral text. "
        "Do not omit text just because it is small. "
        "For every text region, provide its bounding box as "
        "[ymin, xmin, ymax, xmax] normalized from 0 to 1000, "
        "the exact original text, and a natural translation to " +
        targetLang +
        ". Keep each region separate. "
        "Do not merge unrelated regions. "
        "Do not create duplicate regions for the same text. "
        "The translated text must be suitable for drawing directly over "
        "the original game text. "
        "Preserve names, numbers, punctuation and important meaning. "
        "Return only the requested JSON schema.";

    cJSON* root = cJSON_CreateObject();
    cJSON* contents = cJSON_CreateArray();
    cJSON* content = cJSON_CreateObject();
    cJSON* parts = cJSON_CreateArray();

    cJSON* textPart = cJSON_CreateObject();
    cJSON_AddStringToObject(textPart, "text", prompt.c_str());
    cJSON_AddItemToArray(parts, textPart);

    cJSON* imagePart = cJSON_CreateObject();
    cJSON* inlineData = cJSON_CreateObject();

    cJSON_AddStringToObject(
        inlineData, "mime_type", "image/jpeg"
    );

    cJSON_AddStringToObject(
        inlineData, "data", image64.c_str()
    );

    cJSON_AddItemToObject(imagePart, "inline_data", inlineData);
    cJSON_AddItemToArray(parts, imagePart);

    cJSON_AddItemToObject(content, "parts", parts);
    cJSON_AddItemToArray(contents, content);
    cJSON_AddItemToObject(root, "contents", contents);

    // Gemini structured output schema:
    //
    // {
    //   "regions": [
    //      {
    //        "box_2d": [ymin, xmin, ymax, xmax],
    //        "original": "...",
    //        "translated": "..."
    //      }
    //   ]
    // }

    cJSON* generationConfig =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        generationConfig,
        "responseMimeType",
        "application/json"
    );

    cJSON* responseSchema =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        responseSchema,
        "type",
        "OBJECT"
    );

    cJSON* properties =
        cJSON_CreateObject();

    cJSON* regionsSchema =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        regionsSchema,
        "type",
        "ARRAY"
    );

    cJSON* regionItems =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        regionItems,
        "type",
        "OBJECT"
    );

    cJSON* regionProperties =
        cJSON_CreateObject();

    cJSON* boxSchema =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        boxSchema,
        "type",
        "ARRAY"
    );

    cJSON* boxItems =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        boxItems,
        "type",
        "INTEGER"
    );

    cJSON_AddItemToObject(
        boxSchema,
        "items",
        boxItems
    );

    cJSON_AddNumberToObject(
        boxSchema,
        "minItems",
        4
    );

    cJSON_AddNumberToObject(
        boxSchema,
        "maxItems",
        4
    );

    cJSON_AddItemToObject(
        regionProperties,
        "box_2d",
        boxSchema
    );

    cJSON* originalSchema =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        originalSchema,
        "type",
        "STRING"
    );

    cJSON_AddItemToObject(
        regionProperties,
        "original",
        originalSchema
    );

    cJSON* translatedSchema =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        translatedSchema,
        "type",
        "STRING"
    );

    cJSON_AddItemToObject(
        regionProperties,
        "translated",
        translatedSchema
    );

    cJSON_AddItemToObject(
        regionItems,
        "properties",
        regionProperties
    );

    cJSON* requiredRegion =
        cJSON_CreateArray();

    cJSON_AddItemToArray(
        requiredRegion,
        cJSON_CreateString("box_2d")
    );

    cJSON_AddItemToArray(
        requiredRegion,
        cJSON_CreateString("original")
    );

    cJSON_AddItemToArray(
        requiredRegion,
        cJSON_CreateString("translated")
    );

    cJSON_AddItemToObject(
        regionItems,
        "required",
        requiredRegion
    );

    cJSON_AddItemToObject(
        regionsSchema,
        "items",
        regionItems
    );

    cJSON_AddItemToObject(
        properties,
        "regions",
        regionsSchema
    );

    cJSON_AddItemToObject(
        responseSchema,
        "properties",
        properties
    );

    cJSON* requiredRoot =
        cJSON_CreateArray();

    cJSON_AddItemToArray(
        requiredRoot,
        cJSON_CreateString("regions")
    );

    cJSON_AddItemToObject(
        responseSchema,
        "required",
        requiredRoot
    );

    cJSON_AddItemToObject(
        generationConfig,
        "responseSchema",
        responseSchema
    );

    const std::string selectedModel =
        model.empty() ? "gemini-3.1-flash-lite" : model;

    const std::string selectedThinking =
        thinking.empty() ? "minimal" : thinking;

    cJSON* thinkingConfig =
        cJSON_CreateObject();

    cJSON_AddStringToObject(
        thinkingConfig,
        "thinkingLevel",
        selectedThinking.c_str()
    );

    cJSON_AddItemToObject(
        generationConfig,
        "thinkingConfig",
        thinkingConfig
    );

    cJSON_AddItemToObject(
        root,
        "generationConfig",
        generationConfig
    );

    char* json = cJSON_PrintUnformatted(root);
    std::string body(json ? json : "");

    if (json)
        free(json);

    cJSON_Delete(root);

    const std::string url =
        "https://generativelanguage.googleapis.com/v1beta/models/" +
        selectedModel +
        ":generateContent?key=" + apiKey;

    HttpResponse resp = HttpClient::post(
        url,
        body,
        {"Content-Type: application/json"}
    );

    if (!resp.ok()) {
        if (resp.statusCode == 0) {
            result.errorMsg =
                "Gemini AI: Connection failed — " +
                resp.errorStr;
        } else {
            result.errorMsg =
                "Gemini AI: HTTP " +
                std::to_string(resp.statusCode);
        }

        return result;
    }

    cJSON* responseRoot =
        cJSON_Parse(resp.body.c_str());

    if (!responseRoot) {
        result.errorMsg =
            "Gemini AI: Invalid JSON response";
        return result;
    }

    cJSON* candidates =
        cJSON_GetObjectItem(
            responseRoot,
            "candidates"
        );

    std::string modelText;

    if (cJSON_IsArray(candidates) &&
        cJSON_GetArraySize(candidates) > 0) {

        cJSON* candidate =
            cJSON_GetArrayItem(candidates, 0);

        cJSON* contentNode =
            cJSON_GetObjectItem(
                candidate,
                "content"
            );

        cJSON* partsNode =
            contentNode
                ? cJSON_GetObjectItem(
                      contentNode,
                      "parts")
                : nullptr;

        if (cJSON_IsArray(partsNode)) {
            for (int i = 0;
                 i < cJSON_GetArraySize(partsNode);
                 ++i) {

                cJSON* part =
                    cJSON_GetArrayItem(
                        partsNode,
                        i
                    );

                cJSON* textNode =
                    cJSON_GetObjectItem(
                        part,
                        "text"
                    );

                if (textNode &&
                    cJSON_IsString(textNode) &&
                    textNode->valuestring) {

                    modelText =
                        textNode->valuestring;

                    break;
                }
            }
        }
    }

    cJSON_Delete(responseRoot);

    if (modelText.empty()) {
        result.errorMsg =
            "Gemini AI: Empty response";
        return result;
    }

    cJSON* answer =
        cJSON_Parse(modelText.c_str());

    if (!answer) {
        result.errorMsg =
            "Gemini AI: Could not parse structured response";
        return result;
    }

    cJSON* regions =
        cJSON_GetObjectItem(
            answer,
            "regions"
        );

    if (!cJSON_IsArray(regions)) {
        cJSON_Delete(answer);
        result.errorMsg =
            "Gemini AI: Response has no regions";
        return result;
    }

    const int regionCount =
        cJSON_GetArraySize(regions);

    for (int i = 0;
         i < regionCount;
         ++i) {

        cJSON* region =
            cJSON_GetArrayItem(
                regions,
                i
            );

        cJSON* box =
            cJSON_GetObjectItem(
                region,
                "box_2d"
            );

        cJSON* original =
            cJSON_GetObjectItem(
                region,
                "original"
            );

        cJSON* translated =
            cJSON_GetObjectItem(
                region,
                "translated"
            );

        if (!cJSON_IsArray(box) ||
            cJSON_GetArraySize(box) != 4 ||
            !original ||
            !cJSON_IsString(original) ||
            !translated ||
            !cJSON_IsString(translated)) {

            continue;
        }

        cJSON* n0 = cJSON_GetArrayItem(box, 0);
        cJSON* n1 = cJSON_GetArrayItem(box, 1);
        cJSON* n2 = cJSON_GetArrayItem(box, 2);
        cJSON* n3 = cJSON_GetArrayItem(box, 3);

        if (!n0 || !n1 || !n2 || !n3 ||
            !cJSON_IsNumber(n0) ||
            !cJSON_IsNumber(n1) ||
            !cJSON_IsNumber(n2) ||
            !cJSON_IsNumber(n3)) {

            continue;
        }

        float ymin = static_cast<float>(n0->valueint);
        float xmin = static_cast<float>(n1->valueint);
        float ymax = static_cast<float>(n2->valueint);
        float xmax = static_cast<float>(n3->valueint);

        // Gemini coordinates are normalized to 0..1000.
        ymin = std::max(0.0f, std::min(1000.0f, ymin));
        xmin = std::max(0.0f, std::min(1000.0f, xmin));
        ymax = std::max(0.0f, std::min(1000.0f, ymax));
        xmax = std::max(0.0f, std::min(1000.0f, xmax));

        if (xmax <= xmin ||
            ymax <= ymin) {
            continue;
        }

        TranslationRegion r;

        r.x = (xmin / 1000.0f) * 1280.0f;
        r.y = (ymin / 1000.0f) * 720.0f;

        r.w = ((xmax - xmin) / 1000.0f) * 1280.0f;
        r.h = ((ymax - ymin) / 1000.0f) * 720.0f;

        r.original =
            original->valuestring;

        r.translated =
            translated->valuestring;

        if (r.original.empty() ||
            r.translated.empty()) {
            continue;
        }

        result.regions.push_back(
            std::move(r)
        );
    }

    cJSON_Delete(answer);

    if (result.regions.empty()) {
        result.errorMsg =
            "Gemini AI: No text regions detected";
        return result;
    }

    // Compatibility with the existing result system.
    result.translatedText =
        result.regions.front().translated;

    for (const auto& r : result.regions) {
        result.translatedLines.push_back(
            r.translated
        );
    }

    result.success = true;
    return result;
}

} // namespace Translate
