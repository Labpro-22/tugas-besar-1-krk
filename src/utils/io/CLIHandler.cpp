#include "utils/io/CLIHandler.hpp"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

const char* RESET = "\033[0m";

const char* colorCodeForTag(const std::string& tag) {
    if (tag == "CK") return "\033[38;5;130m";
    if (tag == "BM") return "\033[96m";
    if (tag == "PK") return "\033[95m";
    if (tag == "OR") return "\033[38;5;208m";
    if (tag == "MR") return "\033[91m";
    if (tag == "KN") return "\033[93m";
    if (tag == "HJ") return "\033[92m";
    if (tag == "BT") return "\033[94m";
    if (tag == "AB") return "\033[37m";
    if (tag == "DF") return "\033[97m";
    return 0;
}

bool stdoutIsTerminal() {
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

bool enableAnsiOutput() {
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(console, &mode)) {
        return false;
    }

    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        if (!SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            return false;
        }
    }
#endif
    return true;
}

bool shouldColorBoardOutput() {
    static const bool enabled = []() {
        return stdoutIsTerminal() && enableAnsiOutput();
    }();
    return enabled;
}

bool looksLikeBoardOutput(const std::string& message) {
    return message.find("+----------+") != std::string::npos
        && message.find("[DF]") != std::string::npos;
}

void replaceAll(std::string& text, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return;
    }

    std::size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

std::string wrapColor(const std::string& text, const char* color) {
    return std::string(color) + text + RESET;
}

std::string extractTag(const std::string& cell) {
    if (cell.size() >= 4 && cell[0] == '[' && cell[3] == ']') {
        return cell.substr(1, 2);
    }
    return "";
}

std::vector<std::string> extractPipeLineTags(const std::string& line,
                                             const std::vector<std::string>& fallbackTags) {
    std::vector<std::string> tags;
    if (line.empty() || line[0] != '|') {
        return tags;
    }

    std::size_t start = 0;
    std::size_t tileIndex = 0;
    while (start < line.size()) {
        std::size_t end = line.find('|', start + 1);
        if (end == std::string::npos) {
            break;
        }

        std::string segment = line.substr(start + 1, end - start - 1);
        if (segment.size() == 10) {
            std::string tag = extractTag(segment);
            if (tag.empty() && tileIndex < fallbackTags.size()) {
                tag = fallbackTags[tileIndex];
            }
            tags.push_back(tag);
            ++tileIndex;
        }

        start = end;
    }

    return tags;
}

std::size_t countBorderCells(const std::string& line) {
    if (line.empty() || line[0] != '+') {
        return 0;
    }

    std::size_t count = 0;
    std::size_t start = 0;
    while (start < line.size()) {
        std::size_t end = line.find('+', start + 1);
        if (end == std::string::npos) {
            break;
        }

        if (end - start - 1 == 10) {
            ++count;
        }
        start = end;
    }

    return count;
}

std::string colorizeLegendSegment(std::string segment) {
    replaceAll(segment, "[CK]", wrapColor("[CK]", colorCodeForTag("CK")));
    replaceAll(segment, "[BM]", wrapColor("[BM]", colorCodeForTag("BM")));
    replaceAll(segment, "[PK]", wrapColor("[PK]", colorCodeForTag("PK")));
    replaceAll(segment, "[OR]", wrapColor("[OR]", colorCodeForTag("OR")));
    replaceAll(segment, "[MR]", wrapColor("[MR]", colorCodeForTag("MR")));
    replaceAll(segment, "[KN]", wrapColor("[KN]", colorCodeForTag("KN")));
    replaceAll(segment, "[HJ]", wrapColor("[HJ]", colorCodeForTag("HJ")));
    replaceAll(segment, "[BT]", wrapColor("[BT]", colorCodeForTag("BT")));
    replaceAll(segment, "[AB]", wrapColor("[AB]", colorCodeForTag("AB")));
    replaceAll(segment, "[DF]", wrapColor("[DF]", colorCodeForTag("DF")));
    return segment;
}

std::string colorizeContentLine(const std::string& line,
                                const std::vector<std::string>& tags) {
    std::string result;
    std::size_t start = 0;
    std::size_t tileIndex = 0;

    while (start < line.size()) {
        std::size_t end = line.find('|', start + 1);
        if (end == std::string::npos) {
            if (start < line.size() && line[start] == '|' && !tags.empty()) {
                const char* color = colorCodeForTag(tags.back());
                result += (color != 0) ? wrapColor("|", color) : "|";
            } else {
                result += colorizeLegendSegment(line.substr(start));
            }
            break;
        }

        std::string segment = line.substr(start + 1, end - start - 1);
        if (segment.size() == 10 && tileIndex < tags.size()) {
            const char* color = colorCodeForTag(tags[tileIndex]);
            std::string chunk = "|" + segment;
            result += (color != 0) ? wrapColor(chunk, color) : chunk;
            ++tileIndex;
        } else {
            const char* color = 0;
            if (tileIndex > 0 && tileIndex - 1 < tags.size()) {
                color = colorCodeForTag(tags[tileIndex - 1]);
            }
            result += (color != 0) ? wrapColor("|", color) : "|";
            result += colorizeLegendSegment(segment);
        }

        start = end;
    }

    return result;
}

std::string colorizeBorderLine(const std::string& line,
                               const std::vector<std::string>& tags) {
    std::string result;
    std::size_t start = 0;
    std::size_t tileIndex = 0;

    while (start < line.size()) {
        std::size_t end = line.find('+', start + 1);
        if (end == std::string::npos) {
            result += line.substr(start);
            break;
        }

        result += "+";
        std::string segment = line.substr(start + 1, end - start - 1);
        if (segment.size() == 10) {
            const char* color = tileIndex < tags.size() ? colorCodeForTag(tags[tileIndex]) : 0;
            result += (color != 0) ? wrapColor(segment, color) : segment;
            ++tileIndex;
        } else {
            result += segment;
        }

        start = end;
    }

    return result;
}

std::string colorizeBoardLine(const std::string& line,
                              const std::vector<std::string>& previousTags,
                              const std::vector<std::string>& nextTags,
                              std::vector<std::string>& rememberedTags) {
    if (line.empty()) {
        return line;
    }

    if (line[0] == '|') {
        std::vector<std::string> currentTags = extractPipeLineTags(line, previousTags);
        if (!currentTags.empty()) {
            rememberedTags = currentTags;
        }
        return colorizeContentLine(line, currentTags);
    }

    if (line[0] == '+') {
        const std::size_t borderCellCount = countBorderCells(line);
        std::vector<std::string> borderTags;
        if (!previousTags.empty() && previousTags.size() == borderCellCount) {
            borderTags = previousTags;
        } else if (!nextTags.empty() && nextTags.size() == borderCellCount) {
            borderTags = nextTags;
        } else if (!previousTags.empty()) {
            borderTags = previousTags;
        } else {
            borderTags = nextTags;
        }
        return colorizeBorderLine(line, borderTags);
    }

    return colorizeLegendSegment(line);
}

std::string colorizeBoardMessage(const std::string& message) {
    std::vector<std::string> lines;
    std::istringstream input(message);
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    std::ostringstream colored;
    std::vector<std::string> rememberedTags;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) {
            colored << '\n';
        }

        std::vector<std::string> nextTags;
        if (i + 1 < lines.size()) {
            nextTags = extractPipeLineTags(lines[i + 1], rememberedTags);
        }

        colored << colorizeBoardLine(lines[i], rememberedTags, nextTags, rememberedTags);
    }

    if (!message.empty() && message[message.size() - 1] == '\n') {
        colored << '\n';
    }
    return colored.str();
}

}  // namespace

void CLIHandler::printMessage(const std::string& message) {
    const std::string& printable =
        (shouldColorBoardOutput() && looksLikeBoardOutput(message))
            ? colorizeBoardMessage(message)
            : message;

    std::cout << printable;
    if (!message.empty() && message[message.size() - 1] != '\n') {
        std::cout << std::endl;
    }
}

std::string CLIHandler::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}
