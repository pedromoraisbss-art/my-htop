#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static constexpr const char* PATH_ = "/proc/";
static constexpr int REFRESH_MS = 500;

struct ProcInfo {
    std::string pid;
    std::string name;
    std::string state;
    std::string vmsize;
};

static bool is_pid_dir(const std::string& name) {
    return !name.empty() &&
           std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isdigit(c); });
}

static std::string extract_value(const std::string& line) {
    auto pos = line.find(':');
    if (pos == std::string::npos) return "";
    std::string value = line.substr(pos + 1);
    auto start = value.find_first_not_of(" \t");
    auto end = value.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return value.substr(start, end - start + 1);
}

static bool read_status(const std::string& pid, ProcInfo& out) {
    std::ifstream status_file(std::string(PATH_) + pid + "/status");
    if (!status_file.is_open()) return false;

    std::string line;
    bool found_name = false, found_state = false, found_vmsize = false;

    while (std::getline(status_file, line)) {
        if (line.rfind("Name:", 0) == 0) {
            out.name = extract_value(line);
            found_name = true;
        } else if (line.rfind("State:", 0) == 0) {
            out.state = extract_value(line);
            found_state = true;
        } else if (line.rfind("VmSize:", 0) == 0) {
            out.vmsize = extract_value(line);
            found_vmsize = true;
            break;
        }
    }

    out.pid = pid;
    if (!found_vmsize) out.vmsize = "N/A";
    return found_name && found_state;
}

extern "C" void handle_signal(int) {
    endwin();
    std::exit(0);
}

static std::vector<ProcInfo> collect_processes() {
    std::vector<ProcInfo> processes;

    if (!fs::exists(PATH_)) {
        std::cerr << "Erro fatal: " << PATH_ << " nao encontrado neste sistema.\n";
        std::exit(1);
    }

    for (const auto& entry : fs::directory_iterator(PATH_)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        if (!is_pid_dir(name)) continue;

        ProcInfo info;
        if (read_status(name, info)) {
            processes.push_back(std::move(info));
        }
    }

    return processes;
}

int main() {
    initscr();
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(REFRESH_MS);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    int win_height = 3;
    int win_width = std::max(40, max_x - 4);
    int list_height = std::max(5, max_y - win_height - 3);

    WINDOW* win = newwin(win_height, win_width, 1, 2);
    WINDOW* box_info = newwin(list_height, win_width, win_height + 1, 2);

    bool running = true;
    while (running) {
        auto processes = collect_processes();

        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "| (PID)| (PROCESS NAME)       (STATUS)      (VmSize)   (q para sair)");
        wrefresh(win);

        werase(box_info);
        box(box_info, 0, 0);
        int row = 1;
        int max_rows = list_height - 2;
        for (const auto& p : processes) {
            if (row > max_rows) break;
            mvwprintw(box_info, row, 1, "%-8s %-20s %-12s %-12s",
                       p.pid.c_str(), p.name.c_str(), p.state.c_str(), p.vmsize.c_str());
            row++;
        }
        wrefresh(box_info);

        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }
    }

    delwin(win);
    delwin(box_info);
    endwin();

    return 0;
}
