#ifndef ARABIC_HELPER_HPP
#define ARABIC_HELPER_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

class ArabicHelper {
private:
    struct Glyphs {
        char32_t isolated, final_form, medial, initial;
    };

    static bool isArabic(char32_t c) {
        return (c >= 0x0600 && c <= 0x06FF) || (c >= 0xFE70 && c <= 0xFEFF);
    }

    static bool canConnectNext(char32_t c) {
        switch (c) {
            case 0x0621: case 0x0622: case 0x0623: case 0x0624: case 0x0625:
            case 0x0627: case 0x0629: case 0x062F: case 0x0630: case 0x0631:
            case 0x0632: case 0x0648: case 0x0649:
                return false;
            default:
                return isArabic(c);
        }
    }

    static bool canConnectPrev(char32_t c) {
        if (c == 0x0621) return false;
        return isArabic(c);
    }

    static const std::unordered_map<char32_t, Glyphs>& getArabicMap() {
        static const std::unordered_map<char32_t, Glyphs> map = {
            {0x0621, {0xFE80, 0xFE80, 0xFE80, 0xFE80}}, // ء
            {0x0622, {0xFE81, 0xFE82, 0xFE82, 0xFE81}}, // آ
            {0x0623, {0xFE83, 0xFE84, 0xFE84, 0xFE83}}, // أ
            {0x0624, {0xFE85, 0xFE86, 0xFE86, 0xFE85}}, // ؤ
            {0x0625, {0xFE87, 0xFE88, 0xFE88, 0xFE87}}, // إ
            {0x0626, {0xFE89, 0xFE8A, 0xFE8C, 0xFE8B}}, // ئ
            {0x0627, {0xFE8D, 0xFE8E, 0xFE8E, 0xFE8D}}, // ا
            {0x0628, {0xFE8F, 0xFE90, 0xFE92, 0xFE91}}, // ب
            {0x0629, {0xFE93, 0xFE94, 0xFE94, 0xFE93}}, // ة
            {0x062A, {0xFE95, 0xFE96, 0xFE98, 0xFE97}}, // ت
            {0x062B, {0xFE99, 0xFE9A, 0xFE9C, 0xFE9B}}, // ث
            {0x062C, {0xFE9D, 0xFE9E, 0xFEA0, 0xFE9F}}, // ج
            {0x062D, {0xFEA1, 0xFEA2, 0xFEA4, 0xFEA3}}, // ح
            {0x062E, {0xFEA5, 0xFEA6, 0xFEA8, 0xFEA7}}, // خ
            {0x062F, {0xFEA9, 0xFEAA, 0xFEAA, 0xFEA9}}, // د
            {0x0630, {0xFEAB, 0xFEAC, 0xFEAC, 0xFEAB}}, // ذ
            {0x0631, {0xFEAD, 0xFEAE, 0xFEAE, 0xFEAD}}, // ر
            {0x0632, {0xFEAF, 0xFEB0, 0xFEB0, 0xFEAF}}, // ز
            {0x0633, {0xFEB1, 0xFEB2, 0xFEB4, 0xFEB3}}, // س
            {0x0634, {0xFEB5, 0xFEB6, 0xFEB8, 0xFEB7}}, // ش
            {0x0635, {0xFEB9, 0xFEBA, 0xFEBC, 0xFEBB}}, // ص
            {0x0636, {0xFEBD, 0xFEBE, 0xFEC0, 0xFEBF}}, // ض
            {0x0637, {0xFEC1, 0xFEC2, 0xFEC4, 0xFEC3}}, // ط
            {0x0638, {0xFEC5, 0xFEC6, 0xFEC8, 0xFEC7}}, // ظ
            {0x0639, {0xFEC9, 0xFECA, 0xFECC, 0xFECB}}, // ع
            {0x063A, {0xFECD, 0xFECE, 0xFED0, 0xFECF}}, // غ
            {0x0641, {0xFED1, 0xFED2, 0xFED4, 0xFED3}}, // ف
            {0x0642, {0xFED5, 0xFED6, 0xFED8, 0xFED7}}, // ق
            {0x0643, {0xFED9, 0xFEDA, 0xFEDC, 0xFEDB}}, // ك
            {0x0644, {0xFEDD, 0xFEDE, 0xFEE0, 0xFEDF}}, // ل
            {0x0645, {0xFEE1, 0xFEE2, 0xFEE4, 0xFEE3}}, // م
            {0x0646, {0xFEE5, 0xFEE6, 0xFEE8, 0xFEE7}}, // ن
            {0x0647, {0xFEE9, 0xFEEA, 0xFEEC, 0xFEEB}}, // ه
            {0x0648, {0xFEED, 0xFEEE, 0xFEEE, 0xFEED}}, // و
            {0x0649, {0xFEEF, 0xFEF0, 0xFEF0, 0xFEEF}}, // ى
            {0x064A, {0xFEF1, 0xFEF2, 0xFEF4, 0xFEF3}}  // ي
        };
        return map;
    }

    static std::u32string utf8_to_utf32(const std::string& str) {
        std::u32string result;
        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = str[i];
            char32_t u = 0;
            if (c <= 0x7F) { u = c; i += 1; }
            else if ((c & 0xE0) == 0xC0) { u = ((c & 0x1F) << 6) | (str[i+1] & 0x3F); i += 2; }
            else if ((c & 0xF0) == 0xE0) { u = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F); i += 3; }
            else if ((c & 0xF8) == 0xF0) { u = ((c & 0x07) << 18) | ((str[i+1] & 0x3F) << 12) | ((str[i+2] & 0x3F) << 6) | (str[i+3] & 0x3F); i += 4; }
            else { i++; }
            result.push_back(u);
        }
        return result;
    }

    static std::string utf32_to_utf8(const std::u32string& str) {
        std::string result;
        for (char32_t u : str) {
            if (u <= 0x7F) { result.push_back(static_cast<char>(u)); }
            else if (u <= 0x7FF) {
                result.push_back(static_cast<char>(0xC0 | ((u >> 6) & 0x1F)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            } else if (u <= 0xFFFF) {
                result.push_back(static_cast<char>(0xE0 | ((u >> 12) & 0x0F)));
                result.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xF0 | ((u >> 18) & 0x07)));
                result.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (u & 0x3F)));
            }
        }
        return result;
    }

    static std::u32string reshape(const std::u32string& u32) {
        const auto& map = getArabicMap();
        std::u32string shaped;
        size_t n = u32.size();

        for (size_t i = 0; i < n; ++i) {
            char32_t current = u32[i];

            if (current == 0x0644 && (i + 1 < n)) {
                char32_t next = u32[i + 1];
                char32_t lig_iso = 0, lig_fin = 0;
                if (next == 0x0622) { lig_iso = 0xFEF5; lig_fin = 0xFEF6; }
                else if (next == 0x0623) { lig_iso = 0xFEF7; lig_fin = 0xFEF8; }
                else if (next == 0x0625) { lig_iso = 0xFEF9; lig_fin = 0xFEFA; }
                else if (next == 0x0627) { lig_iso = 0xFEFB; lig_fin = 0xFEFC; }

                if (lig_iso != 0) {
                    bool prev_conn = (i > 0) && canConnectNext(u32[i - 1]) && canConnectPrev(current);
                    shaped.push_back(prev_conn ? lig_fin : lig_iso);
                    i++;
                    continue;
                }
            }

            auto it = map.find(current);
            if (it == map.end()) {
                shaped.push_back(current);
                continue;
            }

            bool prev_conn = (i > 0) && canConnectNext(u32[i - 1]) && canConnectPrev(current);
            bool next_conn = (i + 1 < n) && canConnectNext(current) && canConnectPrev(u32[i + 1]);

            const auto& g = it->second;
            if (prev_conn && next_conn) shaped.push_back(g.medial);
            else if (prev_conn) shaped.push_back(g.final_form);
            else if (next_conn) shaped.push_back(g.initial);
            else shaped.push_back(g.isolated);
        }

        return shaped;
    }

    struct Token {
        std::u32string content;
        bool is_arabic;
    };

    static std::string processLine(const std::u32string& line) {
        if (line.empty()) return "";

        std::u32string reshaped = reshape(line);

        bool has_arabic = false;
        for (char32_t c : reshaped) {
            if (isArabic(c)) {
                has_arabic = true;
                break;
            }
        }

        if (!has_arabic) {
            return utf32_to_utf8(reshaped);
        }

        std::vector<Token> tokens;
        Token current_token = {U"", false};

        for (char32_t c : reshaped) {
            bool c_arabic = isArabic(c);
            if (tokens.empty() && current_token.content.empty()) {
                current_token.is_arabic = c_arabic;
                current_token.content.push_back(c);
            } else if (current_token.is_arabic == c_arabic) {
                current_token.content.push_back(c);
            } else {
                tokens.push_back(current_token);
                current_token = {std::u32string(1, c), c_arabic};
            }
        }
        if (!current_token.content.empty()) {
            tokens.push_back(current_token);
        }

        for (auto& tok : tokens) {
            if (tok.is_arabic) {
                std::reverse(tok.content.begin(), tok.content.end());
            }
        }

        std::reverse(tokens.begin(), tokens.end());

        std::u32string result;
        for (const auto& tok : tokens) {
            result += tok.content;
        }

        return utf32_to_utf8(result);
    }

public:
    static std::string process(const std::string& input) {
        if (input.empty()) return input;

        std::u32string u32 = utf8_to_utf32(input);
        std::u32string current_line;
        std::string final_result;

        for (char32_t c : u32) {
            if (c == '\n') {
                final_result += processLine(current_line) + "\n";
                current_line.clear();
            } else {
                current_line.push_back(c);
            }
        }
        if (!current_line.empty() || u32.back() == '\n') {
            final_result += processLine(current_line);
        }

        return final_result;
    }
};

#endif
