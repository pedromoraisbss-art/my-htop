#include <assert.h>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <ncurses.h>
#include <sched.h>
#include <csignal>
#include <signal.h>
#include <fstream>
#include <array>
#include <filesystem>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

typedef const char* htop_nomeclature;
typedef const char essential_characteres;
static constexpr htop_nomeclature _PATH_ = "/proc/";
static constexpr htop_nomeclature _PATH_STATUS = "/status";
// static const bool htop_true_checker = true;
static const bool htop_false_checker = false;
constexpr htop_nomeclature display_inf = "ID\t\t\t|\t\tNAME\t\t|\t\tSTATE\t\t|\t\tVMSIZE\t\t|\t(CTRL C TO EXIT)";

struct leque{
    std::string pid_;
    std::string name_;
    std::string status_;
    std::string vmsize_;
};

struct pos_x_y{
    int x;
    int y;
};

__attribute__((always_inline)) inline void signal_exit(int sig = EXIT_FAILURE){
    exit(sig);
}

static bool filesystem_path_check(){
    namespace fs_system = std::filesystem;
    fs_system::path directory = _PATH_;
    if(!fs_system::exists(directory)){
        std::cerr << "Error: Dont exists this directory" << std::endl;
        return false;
    }else{
        for(const auto &read_directory : directory){
            std::cout << read_directory;
        }
        std::cout << std::endl;
        return true;
    }
}

static std::vector<int> pid_scanner(){
    std::vector<int> number_pid;
    int num;
    std::stringstream ss;

    std::filesystem::path directory_pid = _PATH_;
    if(!std::filesystem::exists(directory_pid) && std::filesystem::is_directory(directory_pid)){
        std::cerr << "Error to read pid";
    }

    for(auto &pid_i : std::filesystem::directory_iterator(_PATH_)){
        for(char c : pid_i.path().string())
            if(std::isdigit(c)) ss << c; else ss << ' '; 
    }

    while(ss >> num)
        number_pid.push_back(num);

    return number_pid;
}

static leque opn_proc(){
    using ifs =  std::ifstream;
    std::string read_word;
    std::vector<int> npid;
    leque rcv;
    //while(!htop_false_checker){
        npid = pid_scanner();
        for(int scan : npid){
            std::string recev = std::to_string(scan);
            ifs file_("/proc/" + recev + "/status");
            rcv.pid_ = recev;
            while(std::getline(file_, read_word)){
                if(read_word.rfind("Name:", 0) ==  0){
                    rcv.name_ = read_word;
                }else if(read_word.rfind("State:", 0) ==  0){
                    rcv.status_ = read_word;
                }else if (read_word.rfind("VmSize:", 0) ==  0){
                    rcv.vmsize_ = read_word;
                    break;
                }
                //return recev += "\t" + rcv.name_ += "\t" +rcv.status_ += "\t" +rcv.vmsize_;
            }   
        }
   // }
    return rcv;
}



int main(){
    std::array<essential_characteres, 3> c_essential = {'|', '-', '*'};
    if(filesystem_path_check() ==  htop_false_checker){
        signal_exit();
    }

    pos_x_y win_cpanel_pos;
    win_cpanel_pos.x = 0;
    win_cpanel_pos.y = 0;

    initscr();
    cbreak();
    noecho();
    stdscr = initscr();
    getmaxyx(stdscr, win_cpanel_pos.y, win_cpanel_pos.x);
    WINDOW *win_cpanel = newwin(3, 
        ((win_cpanel_pos.x/4)+100), win_cpanel_pos.y-63, ((win_cpanel_pos.x/4)-25));
    

    WINDOW *win_cpanelroad = newwin(60, ((win_cpanel_pos.x/4)+100), 
        win_cpanel_pos.y-60, ((win_cpanel_pos.x/4)-25));
    refresh();
    int row = 0;
    while(!htop_false_checker) {    
        box(win_cpanel, c_essential[0], c_essential[1]);
        mvwprintw(win_cpanel, 1, 1,display_inf);
        wrefresh(win_cpanel);

        if(row == 60) row = 0; 
        //const char *c = opn_proc().c_str();
        box(win_cpanelroad, c_essential[0], c_essential[1]);
        mvwprintw(win_cpanelroad, row, 2, 
            "%s\t\t\t\t%s\t\t\t\t%s\t\t\t\t%s",
            opn_proc().pid_.c_str(),
            opn_proc().name_.c_str(), 
            opn_proc().status_.c_str(), 
            opn_proc().vmsize_.c_str());

        wrefresh(win_cpanelroad);
        ++row;
        napms(0);
        //refresh();
    }

    if(SIGINT == 0){
        getch();
    }
    refresh();
    endwin();
    return 0;
}
