#ifndef ARABIC_HELPER_HPP
#define ARABIC_HELPER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

class ArabicHelper {
private:
    static constexpr bool isArabic(char32_t c) {
        return (c >= 0x0600 && c <= 0x06FF);
    }

    static bool canConnectLeft(char32_t c) {
        static const std::vector<char32_t> connectors = {
            0x0628, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x0633, 0x0634,
            0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x0641, 0x0642,
            0x0643, 0x0644, 0x0645, 0x0646, 0x064A
        };
        return std::find(connectors.begin(), connectors.end(), c) != connectors.end();
    }

    static bool canConnectRight(char32_t c) {
        if (!isArabic(c)) return false;
        return c != 0x0627 && c != 0x0629 && c != 0x062F &&
               c != 0x0630 && c != 0x0631 && c != 0x0632 && c != 0x0648;
    }

    struct GlyphForms {
        char32_t isolated;
        char32_t initial;
        char32_t medial;
        char32_t final_form;
    };

    static std::unordered_map<char32_t, GlyphForms> getGlyphMap() {
        return {
            {0x0628, {0xFE8F, 0xFE91, 0xFE92, 0xFE90}},
            {0x062A, {0xFE93, 0xFE95, 0xFE96, 0xFE94}},
            {0x062B, {0xFE97, 0xFE99, 0xFE9A, 0xFE98}},
            {0x062C, {0xFE9B, 0xFE9D, 0xFE9E, 0xFE9C}},
            {0x062D, {0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2}},
            {0x062E, {0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6}},
            {0x0633, {0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2}},
            {0x0634, {0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6}},
            {0x0635, {0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA}},
            {0x0636, {0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE}},
            {0x0637, {0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2}},
            {0x0638, {0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6}},
            {0x0639, {0xFEC9, 0xFECB, 0xFECC, 0xFECA}},
            {0x063A, {0xFECD, 0xFECF, 0xFED0, 0xFECE}},
            {0x0641, {0xFED1, 0xFED3, 0xFED4, 0xFED2}},
            {0x0642, {0xFED5, 0xFED7, 0xFED8, 0xFED6}},
            {0x0643, {0xFED9, 0xFEDB, 0xFEDC, 0xFEDA}},
            {0x0644, {0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE}},
            {0x0645, {0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2}},
            {0x0646, {0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6}},
            {0x0647, {0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA}},
            {0x064A, {0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2}},
            {0x0629, {0xFE93, 0xFE93, 0xFE94, 0xFE94}},
            {0x0649, {0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0}}
        };
    }

    static std::u32string utf8_to_utf32(const std::string& str) {
        std::u32string result;
        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];
            char32_t u = 0;

            if ((c & 0x80) == 0) {
                u = c;
                i += 1;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < str.length()) {
                u = ((static_cast<char32_t>(c) & 0x1F) << 6) |
                    (static_cast<char32_t>(str[i+1]) & 0x3F);
                i += 2;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < str.length()) {
                u = ((static_cast<char32_t>(c) & 0x0F) << 12) |
                    ((static_cast<char32_t>(str[i+1]) & 0x3F) << 6) |
                    (static_cast<char32_t>(str[i+2]) & 0x3F);
                i += 3;
            } else if ((c & 0xF8) == 0xF0 && i + 3 < str.length()) {
                u = ((static_cast<char32_t>(c) & 0x07) << 18) |
                    ((static_cast<char32_t>(str[i+1]) & 0x3F) << 12) |
                    ((static_cast<char32_t>(str[i+2]) & 0x3F) << 6) |
                    (static_cast<char32_t>(str[i+3]) & 0x3F);
                i += 4;
            } else {
                i += 1;
                continue;
            }
            result.push_back(u);
        }
        return result;
    }

    static std::string utf32_to_utf8(const std::u32string& str) {
        std::string result;
        for (char32_t u : str) {
            if (u <= 0x7F) {
                result.push_back(static_cast<char>(u));
            } else if (u <= 0x7FF) {
                result.push_back(static_cast<char>(0xC0 | (u >> 6)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            } else if (u <= 0xFFFF) {
                result.push_back(static_cast<char>(0xE0 | (u >> 12)));
                result.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xF0 | (u >> 18)));
                result.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            }
        }
        return result;
    }

public:
    static std::string process(const std::string& input) {
        if (input.empty()) return input;

        std::u32string u32 = utf8_to_utf32(input);
        auto glyphMap = getGlyphMap();
        std::u32string shaped;

        for (size_t i = 0; i < u32.size(); ++i) {
            char32_t current = u32[i];

            if (!isArabic(current)) {
                shaped.push_back(current);
                continue;
            }

            if (glyphMap.find(current) == glyphMap.end()) {
                shaped.push_back(current);
                continue;
            }

            bool leftConnect = (i > 0) && canConnectLeft(u32[i-1]);
            bool rightConnect = (i + 1 < u32.size()) && canConnectRight(u32[i+1]);

            const auto& forms = glyphMap.at(current);

            if (leftConnect && rightConnect) {
                shaped.push_back(forms.medial);
            } else if (leftConnect) {
                shaped.push_back(forms.final_form);
            } else if (rightConnect) {
                shaped.push_back(forms.initial);
            } else {
                shaped.push_back(forms.isolated);
            }
        }

        return utf32_to_utf8(shaped);
    }
};

#endif
