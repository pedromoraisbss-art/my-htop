#include <cassert>
#include <filesystem>
#include <fstream>
#include <ncurses_dll.h>
#include <string>
#include <ncurses.h>

//#include <ncurses_dll.h>
#define PATH_ "/proc/"
#define BARRER '-'

#ifdef NCURSES_ENABLE_STDBOOL_H
    #else
        static_assert("ncurses.h/ncurses.dll dont defined on system");
#endif

int gen = 0;


std::string check_process(){
    while(1){
        std::string conv_gen_to_string = std::to_string(gen);
        gen++;
        if(gen >= 1000000){
            gen = 0;
        }
        
        std::ifstream read_path(PATH_ + conv_gen_to_string + "/status");
        if(!read_path.is_open()){
            
        }
        std::string name; std::string id; std::string balance_eq; std::string vmsize;
        int line_1 = 0;
        int line_2 = 0;
        int line_3 = 0;
        
        std::string capture_return;
        while(std::getline(read_path, name)){
            std::string block;
            while(read_path >> block){
                if(block == "(running)"/* or block == "(sleeping)"*/){        
                    while(std::getline(read_path, balance_eq)){
                        line_1++;
                        if(line_1 == 1) break;
                    }    
                    while(std::getline(read_path, id)){
                        line_2++;
                        if(line_2 == 3) break;
                    }
                    while(std::getline(read_path, vmsize)){
                        line_3++;
                        if(line_3 ==13) break;
                    }
                    capture_return = (" * " + id + " | " + name + " | " + block + " | " + vmsize);
                    return capture_return;
                }
            }
        }  
    } 
}

int main(){        
    std::filesystem::path directory_check = PATH_;
    if(!std::filesystem::exists(directory_check)){
        assert(std::filesystem::exists(PATH_) && "DONT FOUND THIS PATH");
    }else{
        for(const auto &print_files : std::filesystem::directory_iterator(PATH_)){
            /*OPTIONAL PRINT ALL FILES*/
        }
    }
    initscr();
    cbreak();
    noecho();
    //contin
    /* WINDOW PROCESS */
    
    int x, y;
    getmaxyx(stdscr, y, x);
    WINDOW * win = newwin(3, x-60, y-60, 30);
    WINDOW *box_info = newwin(50, x-60, y-55, 30);
    refresh();

    box(win, static_cast<int>(BARRER), static_cast<int>(BARRER));
    mvwprintw(win, 1, 1, "| (PID)|");
    mvwprintw(win, 1, 10, "(PROCESS NAME)|");
    mvwprintw(win, 1, 30, "(STATUS)|");
    mvwprintw(win, 1, 45, "(VmSize or Virtual memory usage) |");
    wrefresh(win);

    box(box_info, static_cast<int>(BARRER), static_cast<int>(BARRER));
    int i = 0;
    while(1){
        i++;
        //const char *ar = check_process().c_str();
        mvwprintw(box_info, i+1, 1, "%s", check_process().c_str());
        if(i == 47){
            i = 0;
        }
        wrefresh(box_info);
        napms(20);
    }    
    int c = getch();
    
    /*WINDOW END*/
    endwin();
    return 0;
}