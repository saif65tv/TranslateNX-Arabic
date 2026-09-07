#ifndef ARABIC_HELPER_HPP
#define ARABIC_HELPER_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

class ArabicHelper {
private:
    struct Glyphs {
        char32_t isolated, initial, medial, final_form;
    };

    static bool isArabic(char32_t c) {
        return (c >= 0x0600 && c <= 0x06FF) || (c >= 0x0750 && c <= 0x077F) || (c >= 0xFE70 && c <= 0xFEFF);
    }

    static bool connectsToNext(char32_t c) {
        if (c == 0x0621 || c == 0x0622 || c == 0x0623 || c == 0x0624 || c == 0x0625 || 
            c == 0x0627 || c == 0x0629 || c == 0x062F || c == 0x0630 || c == 0x0631 || 
            c == 0x0632 || c == 0x0648 || c == 0x0649) {
            return false;
        }
        return isArabic(c);
    }

    static const std::unordered_map<char32_t, Glyphs>& getMap() {
        static const std::unordered_map<char32_t, Glyphs> map = {
            {0x0621, {0xFE80, 0xFE80, 0xFE80, 0xFE80}}, // ء
            {0x0622, {0xFE81, 0xFE81, 0xFE82, 0xFE82}}, // آ
            {0x0623, {0xFE83, 0xFE83, 0xFE84, 0xFE84}}, // أ
            {0x0624, {0xFE85, 0xFE85, 0xFE86, 0xFE86}}, // ؤ
            {0x0625, {0xFE87, 0xFE87, 0xFE88, 0xFE88}}, // إ
            {0x0626, {0xFE89, 0xFE8B, 0xFE8C, 0xFE8A}}, // ئ
            {0x0627, {0xFE8D, 0xFE8D, 0xFE8E, 0xFE8E}}, // ا
            {0x0628, {0xFE8F, 0xFE91, 0xFE92, 0xFE90}}, // ب
            {0x0629, {0xFE93, 0xFE93, 0xFE94, 0xFE94}}, // ة
            {0x062A, {0xFE95, 0xFE97, 0xFE98, 0xFE96}}, // ت
            {0x062B, {0xFE99, 0xFE9B, 0xFE9C, 0xFE9A}}, // ث
            {0x062C, {0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E}}, // ج
            {0x062D, {0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2}}, // ح
            {0x062E, {0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6}}, // خ
            {0x062F, {0xFEA9, 0xFEA9, 0xFEAA, 0xFEAA}}, // د
            {0x0630, {0xFEAB, 0xFEAB, 0xFEAC, 0xFEAC}}, // ذ
            {0x0631, {0xFEAD, 0xFEAD, 0xFEAE, 0xFEAE}}, // ر
            {0x0632, {0xFEAF, 0xFEAF, 0xFEB0, 0xFEB0}}, // ز
            {0x0633, {0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2}}, // س
            {0x0634, {0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6}}, // ش
            {0x0635, {0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA}}, // ص
            {0x0636, {0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE}}, // ض
            {0x0637, {0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2}}, // ط
            {0x0638, {0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6}}, // ظ
            {0x0639, {0xFEC9, 0xFECB, 0xFECC, 0xFECA}}, // ع
            {0x063A, {0xFECD, 0xFECF, 0xFED0, 0xFECE}}, // غ
            {0x0641, {0xFED1, 0xFED3, 0xFED4, 0xFED2}}, // ف
            {0x0642, {0xFED5, 0xFED7, 0xFED8, 0xFED6}}, // ق
            {0x0643, {0xFED9, 0xFEDB, 0xFEDC, 0xFEDA}}, // ك
            {0x0644, {0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE}}, // ل
            {0x0645, {0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2}}, // م
            {0x0646, {0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6}}, // ن
            {0x0647, {0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA}}, // ه
            {0x0648, {0xFEED, 0xFEED, 0xFEEE, 0xFEEE}}, // و
            {0x0649, {0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0}}, // ى
            {0x064A, {0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2}}  // ي
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

public:
    static std::string process(const std::string& input) {
        if (input.empty()) return input;

        std::u32string u32 = utf8_to_utf32(input);
        const auto& map = getMap();
        std::u32string shaped;

        for (size_t i = 0; i < u32.size(); ++i) {
            char32_t current = u32[i];

            // معالجة اللام-ألف (لا)
            if (current == 0x0644 && i + 1 < u32.size()) {
                char32_t next = u32[i+1];
                char32_t ligature = 0;
                if (next == 0x0622) ligature = 0xFEF5;      // لآ
                else if (next == 0x0623) ligature = 0xFEF7; // لأ
                else if (next == 0x0625) ligature = 0xFEF9; // لإ
                else if (next == 0x0627) ligature = 0xFEFB; // لا

                if (ligature != 0) {
                    bool prev_connects = (i > 0) && connectsToNext(u32[i-1]);
                    shaped.push_back(prev_connects ? (ligature + 1) : ligature);
                    i++; 
                    continue;
                }
            }

            auto it = map.find(current);
            if (it == map.end()) {
                shaped.push_back(current);
                continue;
            }

            bool prev_connects = (i > 0) && connectsToNext(u32[i-1]);
            bool next_connects = (i + 1 < u32.size()) && isArabic(u32[i+1]);

            const auto& g = it->second;
            if (prev_connects && next_connects) shaped.push_back(g.medial);
            else if (prev_connects) shaped.push_back(g.final_form);
            else if (next_connects) shaped.push_back(g.initial);
            else shaped.push_back(g.isolated);
        }

        // فصل الأسطر وعكس النص بداخل كل سطر بمفرده لمنع تشوه الترتيب
        std::u32string final_result;
        std::u32string current_line;

        for (char32_t c : shaped) {
            if (c == U'\n') {
                std::reverse(current_line.begin(), current_line.end());
                final_result += current_line + U'\n';
                current_line.clear();
            } else {
                current_line.push_back(c);
            }
        }
        std::reverse(current_line.begin(), current_line.end());
        final_result += current_line;

        return utf32_to_utf8(final_result);
    }
};

#endif
