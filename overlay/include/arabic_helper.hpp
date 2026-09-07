#ifndef ARABIC_HELPER_HPP
#define ARABIC_HELPER_HPP

#include <string>
#include <cstdint>
#include <vector>

class ArabicHelper {
private:
    struct Form {
        char32_t isolated;
        char32_t final_form;
        char32_t initial;
        char32_t medial;
        bool joins_previous;
        bool joins_next;
    };

    static bool getForm(char32_t c, Form& f) {
        switch (c) {
            case 0x0621: f = {0xFE80, 0xFE80, 0, 0, false, false}; return true; // ء
            case 0x0622: f = {0xFE81, 0xFE82, 0, 0, true,  false}; return true; // آ
            case 0x0623: f = {0xFE83, 0xFE84, 0, 0, true,  false}; return true; // أ
            case 0x0624: f = {0xFE85, 0xFE86, 0, 0, true,  false}; return true; // ؤ
            case 0x0625: f = {0xFE87, 0xFE88, 0, 0, true,  false}; return true; // إ
            case 0x0627: f = {0xFE8D, 0xFE8E, 0, 0, true,  false}; return true; // ا

            case 0x0628: f = {0xFE8F, 0xFE90, 0xFE91, 0xFE92, true, true}; return true; // ب
            case 0x0629: f = {0xFE93, 0xFE94, 0, 0, true, false}; return true; // ة
            case 0x062A: f = {0xFE95, 0xFE96, 0xFE97, 0xFE98, true, true}; return true; // ت
            case 0x062B: f = {0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, true, true}; return true; // ث
            case 0x062C: f = {0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, true, true}; return true; // ج
            case 0x062D: f = {0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, true, true}; return true; // ح
            case 0x062E: f = {0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, true, true}; return true; // خ
            case 0x062F: f = {0xFEA9, 0xFEAA, 0, 0, true, false}; return true; // د
            case 0x0630: f = {0xFEAB, 0xFEAC, 0, 0, true, false}; return true; // ذ
            case 0x0631: f = {0xFEAD, 0xFEAE, 0, 0, true, false}; return true; // ر
            case 0x0632: f = {0xFEAF, 0xFEB0, 0, 0, true, false}; return true; // ز
            case 0x0633: f = {0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, true, true}; return true; // س
            case 0x0634: f = {0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, true, true}; return true; // ش
            case 0x0635: f = {0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, true, true}; return true; // ص
            case 0x0636: f = {0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, true, true}; return true; // ض
            case 0x0637: f = {0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, true, true}; return true; // ط
            case 0x0638: f = {0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, true, true}; return true; // ظ
            case 0x0639: f = {0xFEC9, 0xFECA, 0xFECB, 0xFECC, true, true}; return true; // ع
            case 0x063A: f = {0xFECD, 0xFECE, 0xFECF, 0xFED0, true, true}; return true; // غ
            case 0x0641: f = {0xFED1, 0xFED2, 0xFED3, 0xFED4, true, true}; return true; // ف
            case 0x0642: f = {0xFED5, 0xFED6, 0xFED7, 0xFED8, true, true}; return true; // ق
            case 0x0643: f = {0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, true, true}; return true; // ك
            case 0x0644: f = {0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, true, true}; return true; // ل
            case 0x0645: f = {0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, true, true}; return true; // م
            case 0x0646: f = {0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, true, true}; return true; // ن
            case 0x0647: f = {0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, true, true}; return true; // ه
            case 0x0648: f = {0xFEED, 0xFEEE, 0, 0, true, false}; return true; // و
            case 0x0649: f = {0xFEEF, 0xFEF0, 0, 0, true, false}; return true; // ى
            case 0x064A: f = {0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, true, true}; return true; // ي

            default:
                return false;
        }
    }

    static bool isMark(char32_t c) {
        return (c >= 0x064B && c <= 0x065F) || c == 0x0670;
    }

    static std::u32string decode(const std::string& s) {
        std::u32string result;

        for (size_t i = 0; i < s.size();) {
            uint8_t c = static_cast<uint8_t>(s[i]);

            if (c <= 0x7F) {
                result.push_back(c);
                i++;
            } else if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
                result.push_back(
                    ((c & 0x1F) << 6) |
                    (static_cast<uint8_t>(s[i + 1]) & 0x3F)
                );
                i += 2;
            } else if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
                result.push_back(
                    ((c & 0x0F) << 12) |
                    ((static_cast<uint8_t>(s[i + 1]) & 0x3F) << 6) |
                    (static_cast<uint8_t>(s[i + 2]) & 0x3F)
                );
                i += 3;
            } else if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
                result.push_back(
                    ((c & 0x07) << 18) |
                    ((static_cast<uint8_t>(s[i + 1]) & 0x3F) << 12) |
                    ((static_cast<uint8_t>(s[i + 2]) & 0x3F) << 6) |
                    (static_cast<uint8_t>(s[i + 3]) & 0x3F)
                );
                i += 4;
            } else {
                i++;
            }
        }

        return result;
    }

    static std::string encode(const std::u32string& s) {
        std::string result;

        for (char32_t c : s) {
            if (c <= 0x7F) {
                result.push_back(static_cast<char>(c));
            } else if (c <= 0x7FF) {
                result.push_back(static_cast<char>(0xC0 | (c >> 6)));
                result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            } else if (c <= 0xFFFF) {
                result.push_back(static_cast<char>(0xE0 | (c >> 12)));
                result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xF0 | (c >> 18)));
                result.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            }
        }

        return result;
    }

    static bool isArabicPresentationGlyph(char32_t c) {
        return c >= 0xFE70 && c <= 0xFEFF;
    }

    // Reorder shaped Arabic runs for an LTR renderer.
    // Arabic combining marks remain attached to their base glyph.
    static std::u32string reorderVisual(const std::u32string& input) {
        std::u32string output;

        size_t i = 0;
        while (i < input.size()) {
            if (!isArabicPresentationGlyph(input[i])) {
                output.push_back(input[i]);
                i++;
                continue;
            }

            size_t runStart = i;

            while (i < input.size()) {
                char32_t c = input[i];

                if (isArabicPresentationGlyph(c) ||
                    isMark(c) ||
                    c == U' ' ||
                    c == U'\t') {
                    i++;
                } else {
                    break;
                }
            }

            size_t end = i;

            // Copy Arabic clusters in reverse visual order.
            while (end > runStart) {
                size_t base = end - 1;

                while (base > runStart && isMark(input[base])) {
                    base--;
                }

                for (size_t j = base; j < end; j++) {
                    output.push_back(input[j]);
                }

                end = base;
            }
        }

        return output;
    }

public:
    static std::string process(const std::string& input) {
        if (input.empty()) {
            return input;
        }

        std::u32string source = decode(input);
        std::u32string output;

        for (size_t i = 0; i < source.size(); i++) {
            Form current;

            if (!getForm(source[i], current)) {
                output.push_back(source[i]);
                continue;
            }

            size_t previous = i;

            while (previous > 0) {
                previous--;

                if (!isMark(source[previous])) {
                    break;
                }
            }

            size_t next = i + 1;

            while (next < source.size() && isMark(source[next])) {
                next++;
            }

            bool connectPrevious = false;
            bool connectNext = false;

            if (previous < i) {
                Form previousForm;

                if (getForm(source[previous], previousForm)) {
                    connectPrevious =
                        previousForm.joins_next &&
                        current.joins_previous;
                }
            }

            if (next < source.size()) {
                Form nextForm;

                if (getForm(source[next], nextForm)) {
                    connectNext =
                        current.joins_next &&
                        nextForm.joins_previous;
                }
            }

            if (connectPrevious && connectNext && current.medial != 0) {
                output.push_back(current.medial);
            } else if (connectPrevious && current.final_form != 0) {
                output.push_back(current.final_form);
            } else if (connectNext && current.initial != 0) {
                output.push_back(current.initial);
            } else {
                output.push_back(current.isolated);
            }
        }

        return encode(reorderVisual(output));
    }
};

#endif
