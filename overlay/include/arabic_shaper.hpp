#pragma once

#include <switch.h>
#include <string>
#include <vector>
#include <algorithm>

namespace tsl {
namespace ArabicShaper {

struct Forms {
    u32 isolated;
    u32 final;
    u32 initial;
    u32 medial;
    bool joinsPrevious;
    bool joinsNext;
};

inline const Forms* getForms(u32 cp) {
    switch (cp) {
        case 0x0622: { static const Forms f{0xFE81,0xFE82,0,0,true,false}; return &f; } // آ
        case 0x0623: { static const Forms f{0xFE83,0xFE84,0,0,true,false}; return &f; } // أ
        case 0x0624: { static const Forms f{0xFE85,0xFE86,0,0,true,false}; return &f; } // ؤ
        case 0x0625: { static const Forms f{0xFE87,0xFE88,0,0,true,false}; return &f; } // إ
        case 0x0627: { static const Forms f{0xFE8D,0xFE8E,0,0,true,false}; return &f; } // ا
        case 0x0628: { static const Forms f{0xFE8F,0xFE90,0xFE91,0xFE92,true,true}; return &f; } // ب
        case 0x0629: { static const Forms f{0xFE93,0xFE94,0,0,true,false}; return &f; } // ة
        case 0x062A: { static const Forms f{0xFE95,0xFE96,0xFE97,0xFE98,true,true}; return &f; } // ت
        case 0x062B: { static const Forms f{0xFE99,0xFE9A,0xFE9B,0xFE9C,true,true}; return &f; } // ث
        case 0x062C: { static const Forms f{0xFE9D,0xFE9E,0xFE9F,0xFEA0,true,true}; return &f; } // ج
        case 0x062D: { static const Forms f{0xFEA1,0xFEA2,0xFEA3,0xFEA4,true,true}; return &f; } // ح
        case 0x062E: { static const Forms f{0xFEA5,0xFEA6,0xFEA7,0xFEA8,true,true}; return &f; } // خ
        case 0x062F: { static const Forms f{0xFEA9,0xFEAA,0,0,true,false}; return &f; } // د
        case 0x0630: { static const Forms f{0xFEAB,0xFEAC,0,0,true,false}; return &f; } // ذ
        case 0x0631: { static const Forms f{0xFEAD,0xFEAE,0,0,true,false}; return &f; } // ر
        case 0x0632: { static const Forms f{0xFEAF,0xFEB0,0,0,true,false}; return &f; } // ز
        case 0x0633: { static const Forms f{0xFEB1,0xFEB2,0xFEB3,0xFEB4,true,true}; return &f; } // س
        case 0x0634: { static const Forms f{0xFEB5,0xFEB6,0xFEB7,0xFEB8,true,true}; return &f; } // ش
        case 0x0635: { static const Forms f{0xFEB9,0xFEBA,0xFEBB,0xFEBC,true,true}; return &f; } // ص
        case 0x0636: { static const Forms f{0xFEBD,0xFEBE,0xFEBF,0xFEC0,true,true}; return &f; } // ض
        case 0x0637: { static const Forms f{0xFEC1,0xFEC2,0xFEC3,0xFEC4,true,true}; return &f; } // ط
        case 0x0638: { static const Forms f{0xFEC5,0xFEC6,0xFEC7,0xFEC8,true,true}; return &f; } // ظ
        case 0x0639: { static const Forms f{0xFEC9,0xFECA,0xFECB,0xFECC,true,true}; return &f; } // ع
        case 0x063A: { static const Forms f{0xFECD,0xFECE,0xFECF,0xFED0,true,true}; return &f; } // غ
        case 0x0641: { static const Forms f{0xFED1,0xFED2,0xFED3,0xFED4,true,true}; return &f; } // ف
        case 0x0642: { static const Forms f{0xFED5,0xFED6,0xFED7,0xFED8,true,true}; return &f; } // ق
        case 0x0643: { static const Forms f{0xFED9,0xFEDA,0xFEDB,0xFEDC,true,true}; return &f; } // ك
        case 0x0644: { static const Forms f{0xFEDD,0xFEDE,0xFEDF,0xFEE0,true,true}; return &f; } // ل
        case 0x0645: { static const Forms f{0xFEE1,0xFEE2,0xFEE3,0xFEE4,true,true}; return &f; } // م
        case 0x0646: { static const Forms f{0xFEE5,0xFEE6,0xFEE7,0xFEE8,true,true}; return &f; } // ن
        case 0x0647: { static const Forms f{0xFEE9,0xFEEA,0xFEEB,0xFEEC,true,true}; return &f; } // ه
        case 0x0648: { static const Forms f{0xFEED,0xFEEE,0,0,true,false}; return &f; } // و
        case 0x0649: { static const Forms f{0xFEEF,0xFEF0,0,0,true,false}; return &f; } // ى
        case 0x064A: { static const Forms f{0xFEF1,0xFEF2,0xFEF3,0xFEF4,true,true}; return &f; } // ي
        default: return nullptr;
    }
}

inline void appendUtf8(std::string& out, u32 cp) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

inline void decodeUtf8(const char* text, std::vector<u32>& cps) {
    const u8* p = reinterpret_cast<const u8*>(text);

    while (*p) {
        u32 cp = 0;
        ssize_t width = decode_utf8(&cp, p);

        if (width <= 0)
            break;

        cps.push_back(cp);
        p += width;
    }
}

inline bool isArabicCodepoint(u32 cp) {
    return getForms(cp) != nullptr ||
           (cp >= 0x064B && cp <= 0x065F) ||
           cp == 0x0670;
}

inline bool isLtrCodepoint(u32 cp) {
    return (cp >= 'A' && cp <= 'Z') ||
           (cp >= 'a' && cp <= 'z') ||
           (cp >= '0' && cp <= '9');
}

inline bool isWhitespace(u32 cp) {
    return cp == ' ' || cp == '\t' || cp == '\r';
}

inline bool tokenHasArabic(const std::vector<u32>& token) {
    for (u32 cp : token) {
        if (getForms(cp))
            return true;
    }
    return false;
}

inline bool tokenHasLtr(const std::vector<u32>& token) {
    for (u32 cp : token) {
        if (isLtrCodepoint(cp))
            return true;
    }
    return false;
}

inline bool tryLamAlef(const std::vector<u32>& cps, size_t& i,
                       size_t runStart, u32& shaped) {
    if (i + 1 >= cps.size() || cps[i] != 0x0644)
        return false;

    u32 alef = cps[i + 1];
    u32 ligature = 0;

    if (alef == 0x0622)
        ligature = 0xFEF5;
    else if (alef == 0x0623)
        ligature = 0xFEF7;
    else if (alef == 0x0625)
        ligature = 0xFEF9;
    else if (alef == 0x0627)
        ligature = 0xFEFB;

    if (!ligature)
        return false;

    bool connectsFromLeft = false;

    if (i > runStart) {
        const Forms* previous = getForms(cps[i - 1]);
        if (previous && previous->joinsNext)
            connectsFromLeft = true;
    }

    shaped = connectsFromLeft ? ligature + 1 : ligature;
    ++i;
    return true;
}

inline std::vector<u32> shapeArabicToken(const std::vector<u32>& token) {
    std::vector<u32> shaped;
    shaped.reserve(token.size());

    for (size_t i = 0; i < token.size(); ++i) {
        const u32 current = token[i];

        if ((current >= 0x064B && current <= 0x065F) || current == 0x0670)
            continue;

        u32 ligature = 0;
        if (tryLamAlef(token, i, 0, ligature)) {
            shaped.push_back(ligature);
            continue;
        }

        const Forms* forms = getForms(current);

        if (!forms) {
            shaped.push_back(current);
            continue;
        }

        const Forms* previous =
            (i > 0) ? getForms(token[i - 1]) : nullptr;

        const Forms* next =
            (i + 1 < token.size()) ? getForms(token[i + 1]) : nullptr;

        const bool joinPrevious =
            forms->joinsPrevious &&
            previous &&
            previous->joinsNext;

        const bool joinNext =
            forms->joinsNext &&
            next &&
            next->joinsPrevious;

        u32 result = forms->isolated;

        if (joinPrevious && joinNext && forms->medial)
            result = forms->medial;
        else if (joinPrevious && forms->final)
            result = forms->final;
        else if (joinNext && forms->initial)
            result = forms->initial;

        shaped.push_back(result);
    }

    std::reverse(shaped.begin(), shaped.end());
    return shaped;
}

inline std::string shape(const char* text) {
    if (!text || !*text)
        return {};

    std::vector<u32> cps;
    decodeUtf8(text, cps);

    std::string out;
    out.reserve(cps.size() * 3);

    size_t lineStart = 0;

    while (lineStart < cps.size()) {
        size_t lineEnd = lineStart;

        while (lineEnd < cps.size() && cps[lineEnd] != '\n')
            ++lineEnd;

        bool hasArabic = false;
        for (size_t i = lineStart; i < lineEnd; ++i) {
            if (getForms(cps[i])) {
                hasArabic = true;
                break;
            }
        }

        /*
         * Pure LTR line: keep it completely unchanged.
         */
        if (!hasArabic) {
            for (size_t i = lineStart; i < lineEnd; ++i)
                appendUtf8(out, cps[i]);
        } else {
            /*
             * Split the line into whitespace-separated tokens.
             * Arabic tokens are shaped and reversed.
             * Latin/number tokens remain unchanged.
             */
            std::vector<std::vector<u32>> tokens;
            std::vector<u32> current;

            for (size_t i = lineStart; i < lineEnd; ++i) {
                if (isWhitespace(cps[i])) {
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(cps[i]);
                }
            }

            if (!current.empty())
                tokens.push_back(current);

            /*
             * Reverse token order only when the line is Arabic.
             */
            std::reverse(tokens.begin(), tokens.end());

            for (size_t t = 0; t < tokens.size(); ++t) {
                const auto& token = tokens[t];

                if (tokenHasArabic(token) && !tokenHasLtr(token)) {
                    std::vector<u32> shaped = shapeArabicToken(token);

                    for (u32 cp : shaped)
                        appendUtf8(out, cp);
                } else {
                    /*
                     * English and numbers stay in their original order.
                     */
                    for (u32 cp : token)
                        appendUtf8(out, cp);
                }

                if (t + 1 < tokens.size())
                    out.push_back(' ');
            }
        }

        if (lineEnd < cps.size() && cps[lineEnd] == '\n')
            out.push_back('\n');

        lineStart = lineEnd + 1;
    }

    return out;
}

} // namespace ArabicShaper
} // namespace tsl
