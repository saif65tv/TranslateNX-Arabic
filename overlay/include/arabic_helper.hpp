#ifndef ARABIC_HELPER_HPP
#define ARABIC_HELPER_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

class ArabicHelper {
private:
    static bool isArabic(uint32_t c) {
        return (c >= 0x0600 && c <= 0x06FF) || (c >= 0xFE70 && c <= 0xFEFF);
    }

    static std::vector<uint32_t> utf8ToUtf32(const std::string& str) {
        std::vector<uint32_t> result;
        size_t i = 0;
        while (i < str.length()) {
            uint32_t c = (unsigned char)str[i];
            if (c <= 0x7F) {
                result.push_back(c); i += 1;
            } else if ((c & 0xE0) == 0xC0) {
                result.push_back(((c & 0x1F) << 6) | ((unsigned char)str[i+1] & 0x3F)); i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                result.push_back(((c & 0x0F) << 12) | (((unsigned char)str[i+1] & 0x3F) << 6) | ((unsigned char)str[i+2] & 0x3F)); i += 3;
            } else { i += 1; }
        }
        return result;
    }

    static std::string utf32ToUtf8(const std::vector<uint32_t>& str) {
        std::string result = "";
        for (uint32_t c : str) {
            if (c <= 0x7F) {
                result += (char)c;
            } else if (c <= 0x7FF) {
                result += (char)(0xC0 | ((c >> 6) & 0x1F));
                result += (char)(0x80 | (c & 0x3F));
            } else if (c <= 0xFFFF) {
                result += (char)(0xE0 | ((c >> 12) & 0x0F));
                result += (char)(0x80 | ((c >> 6) & 0x3F));
                result += (char)(0x80 | (c & 0x3F));
            }
        }
        return result;
    }

public:
    static std::string process(const std::string& input) {
        if (input.empty()) return input;
        std::vector<uint32_t> utf32 = utf8ToUtf32(input);
        bool hasArabic = false;
        for (uint32_t c : utf32) { if (isArabic(c)) { hasArabic = true; break; } }
        if (!hasArabic) return input;

        std::vector<uint32_t> processed;
        std::vector<uint32_t> line;

        for (size_t i = 0; i < utf32.size(); ++i) {
            if (utf32[i] == '\n') {
                std::reverse(line.begin(), line.end());
                processed.insert(processed.end(), line.begin(), line.end());
                processed.push_back('\n');
                line.clear();
            } else {
                line.push_back(utf32[i]);
            }
        }
        if (!line.empty()) {
            std::reverse(line.begin(), line.end());
            processed.insert(processed.end(), line.begin(), line.end());
        }
        return utf32ToUtf8(processed);
    }
};

#endif
