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

    static bool isArabicLetter(char32_t c) {
        return (c >= 0x0621 && c <= 0x063A) || 
               (c >= 0x0641 && c <= 0x064A) ||
               (c >= 0xFE70 && c <= 0xFEFF);
    }

    static bool connectsToNext(char32_t c) {
        if (!isArabicLetter(c)) return false;
        switch (c) {
            case 0x0621: case 0x0622: case 0x0623: case 0x0624: case 0x0625: 
            case 0x0627: case 0x0629: case 0x062F: case 0x0630: case 0x0631: 
            case 0x0632: case 0x0648: case 0x0649:
                return false;
            default:
                return true;
        }
    }

    static const std::unordered_map<char32_t, Glyphs>& getMap() {
        static const std::unordered_map<char32_t, Glyphs> map = {
            {0x0621, {0xFE80, 0xFE80, 0xFE80, 0xFE80}},
            {0x0622, {0xFE81, 0xFE81, 0xFE82, 0xFE82}},
            {0x0623, {0xFE83, 0xFE83, 0xFE84, 0xFE84}},
            {0x0624, {0xFE85, 0xFE85, 0xFE86, 0xFE86}},
            {0x0625, {0xFE87, 0xFE87, 0xFE88, 0xFE88}},
            {0x0626, {0xFE89, 0xFE8B, 0xFE8C, 0xFE8A}},
            {0x0627, {0xFE8D, 0xFE8D, 0xFE8E, 0xFE8E}},
            {0x0628, {0xFE8F, 0xFE91, 0xFE92, 0xFE90}},
            {0x0629, {0xFE93, 0xFE93, 0xFE94, 0xFE94}},
            {0x062A, {0xFE95, 0xFE97, 0xFE98, 0xFE96}},
            {0x062B, {0xFE99, 0xFE9B, 0xFE9C, 0xFE9A}},
            {0x062C, {0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E}},
            {0x062D, {0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2}},
            {0x062E, {0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6}},
            {0x062F, {0xFEA9, 0xFEA9, 0xFEAA, 0xFEAA}},
            {0x0630, {0xFEAB, 0xFEAB, 0xFEAC, 0xFEAC}},
            {0x0631, {0xFEAD, 0xFEAD, 0xFEAE, 0xFEAE}},
            {0x0632, {0xFEAF, 0xFEAF, 0xFEB0, 0xFEB0}},
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
            {0x0648, {0xFEED, 0xFEED, 0xFEEE, 0xFEEE}},
            {0x0649, {0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0}},
            {0x064A, {0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2}}
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

    struct Token {
        std::u32string text;
        bool is_arabic;
    };

    static std::string processLine(const std::u32string& line) {
        if (line.empty()) return "";
        
        const auto& map = getMap();
        std::u32string shaped;
        
        for (size_t i = 0; i < line.size(); ++i) {
            char32_t current = line[i];
            
            if (current >= 0x064B && current <= 0x065F) continue;
            if (current == 0x0670) continue;

            if (current == 0x0644) {
                size_t next_idx = i + 1;
                while (next_idx < line.size() && ((line[next_idx] >= 0x064B && line[next_idx] <= 0x065F) || line[next_idx] == 0x0670)) {
                    next_idx++;
                }
                if (next_idx < line.size()) {
                    char32_t next = line[next_idx];
                    char32_t ligature = 0;
                    if (next == 0x0622) ligature = 0xFEF5;
                    else if (next == 0x0623) ligature = 0xFEF7;
                    else if (next == 0x0625) ligature = 0xFEF9;
                    else if (next == 0x0627) ligature = 0xFEFB;

                    if (ligature != 0) {
                        bool prev_connects = false;
                        int prev_idx = static_cast<int>(i) - 1;
                        while (prev_idx >= 0 && ((line[prev_idx] >= 0x064B && line[prev_idx] <= 0x065F) || line[prev_idx] == 0x0670)) {
                            prev_idx--;
                        }
                        if (prev_idx >= 0) prev_connects = connectsToNext(line[prev_idx]);
                        
                        shaped.push_back(prev_connects ? (ligature + 1) : ligature);
                        i = next_idx;
                        continue;
                    }
                }
            }

            auto it = map.find(current);
            if (it == map.end()) {
                shaped.push_back(current);
                continue;
            }

            bool prev_connects = false;
            int prev_idx = static_cast<int>(i) - 1;
            while (prev_idx >= 0 && ((line[prev_idx] >= 0x064B && line[prev_idx] <= 0x065F) || line[prev_idx] == 0x0670)) prev_idx--;
            if (prev_idx >= 0) prev_connects = connectsToNext(line[prev_idx]);

            bool next_connects = false;
            size_t next_idx = i + 1;
            while (next_idx < line.size() && ((line[next_idx] >= 0x064B && line[next_idx] <= 0x065F) || line[next_idx] == 0x0670)) next_idx++;
            if (next_idx < line.size()) next_connects = isArabicLetter(line[next_idx]);

            const auto& g = it->second;
            if (prev_connects && next_connects) shaped.push_back(g.medial);
            else if (prev_connects) shaped.push_back(g.final_form);
            else if (next_connects) shaped.push_back(g.initial);
            else shaped.push_back(g.isolated);
        }

        std::vector<Token> tokens;
        std::u32string current_token;
        bool current_is_arabic = false;

        if (!shaped.empty()) {
            current_is_arabic = isArabicLetter(shaped[0]);
            current_token.push_back(shaped[0]);
        }

        for (size_t i = 1; i < shaped.size(); ++i) {
            char32_t c = shaped[i];
            bool c_is_arabic = isArabicLetter(c);
            
            if (c_is_arabic == current_is_arabic) {
                current_token.push_back(c);
            } else {
                tokens.push_back({current_token, current_is_arabic});
                current_token.clear();
                current_token.push_back(c);
                current_is_arabic = c_is_arabic;
            }
        }
        if (!current_token.empty()) {
            tokens.push_back({current_token, current_is_arabic});
        }

        for (auto& token : tokens) {
            if (token.is_arabic) {
                std::reverse(token.text.begin(), token.text.end());
            }
        }
        
        std::reverse(tokens.begin(), tokens.end());

        std::u32string final_line;
        for (const auto& token : tokens) {
            final_line += token.text;
        }
        
        return utf32_to_utf8(final_line);
    }

public:
    static std::string process(const std::string& input) {
        if (input.empty()) return input;

        std::u32string u32 = utf8_to_utf32(input);
        std::u32string current_line;
        std::string final_result;

        for (char32_t c : u32) {
            if (c == U'\n') {
                final_result += processLine(current_line) + "\n";
                current_line.clear();
            } else {
                current_line.push_back(c);
            }
        }
        final_result += processLine(current_line);

        return final_result;
    }
};

#endif
