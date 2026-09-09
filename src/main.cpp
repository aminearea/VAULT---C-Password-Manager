#include "vault.h"
#include "crypto.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <cstdlib>
#endif

// --- ANSI COLORS ---
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"

// --- UI HELPERS ---
const int INNER_WIDTH = 58;

void enable_ansi_colors() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    #endif
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void typewriter(const std::string& text, int delay_ms = 10) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
}

void print_banner(bool animate = true) {
    std::string banner = 
        "  ██╗   ██╗ █████╗ ██╗   ██╗██╗  ████████╗\n"
        "  ██║   ██║██╔══██╗██║   ██║██║  ╚══██╔══╝\n"
        "  ██║   ██║███████║██║   ██║██║     ██║   \n"
        "  ╚██╗ ██╔╝██╔══██║██║   ██║██║     ██║   \n"
        "   ╚████╔╝ ██║  ██║╚██████╔╝███████╗██║   \n"
        "    ╚═══╝  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝   \n";

    if (animate) {
        std::cout << CYAN << BOLD;
        typewriter(banner, 2);
    } else {
        std::cout << CYAN << BOLD << banner;
    }
    std::cout << RESET << std::endl;
}

void fake_loading() {
    std::cout << YELLOW << "  [";
    for (int i = 0; i < 10; ++i) {
        std::cout << "#";
        std::cout.flush();
        Sleep(80);
    }
    std::cout << "] " << BOLD << "Initializing Vault System" << RESET << std::endl;
    Sleep(300);
}

void press_enter() {
    std::cout << DIM << "\n  [Press Enter to continue]" << RESET;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print_success(const std::string& msg) {
    std::cout << GREEN << BOLD << "  [ OK ] " << msg << RESET << std::endl;
}

void print_error(const std::string& msg) {
    std::cout << RED << BOLD << " [ ERR ] " << msg << RESET << std::endl;
}

void print_sub_header(const std::string& title) {
    clear_screen();
    std::cout << CYAN << "  +" << std::string(INNER_WIDTH, '-') << "+\n";
    std::cout << CYAN << "  |" << std::string(INNER_WIDTH, ' ') << "|\n";
    
    std::string content = "  " + title;
    int pad_left = (INNER_WIDTH - content.length()) / 2;
    if (pad_left < 0) pad_left = 0;
    int pad_right = INNER_WIDTH - pad_left - content.length();

    std::cout << CYAN << "  |" << std::string(pad_left, ' ') << BOLD << content << RESET;
    std::cout << CYAN << std::string(pad_right, ' ') << "|" << RESET << "\n";
    
    std::cout << CYAN << "  |" << std::string(INNER_WIDTH, ' ') << "|\n";
    std::cout << CYAN << "  +" << std::string(INNER_WIDTH, '-') << "+" << RESET << std::endl << std::endl;
}

// --- CONSOLE CLASS (Fixed Hidden Input) ---
class Console {
public:
    static std::string read_hidden(const std::string& prompt) {
        std::cout << YELLOW << "  [>>] " << prompt << RESET << std::flush;
        std::string input;
        
    #ifdef _WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
        std::getline(std::cin, input);
        SetConsoleMode(hStdin, mode);
    #else
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::getline(std::cin, input);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif
        
        // *** THE FIX ***: Clean \r and trailing spaces from the password
        if (!input.empty() && input.back() == '\r') {
            input.pop_back();
        }
        input.erase(std::find_if(input.rbegin(), input.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), input.end());

        std::cout << '\n';
        return input;
    }
    
    static std::string read_line(const std::string& prompt) {
        std::cout << YELLOW << "  [>>] " << prompt << RESET;
        std::string line;
        std::getline(std::cin, line);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), line.end());
        return line;
    }
    
    static int read_int(const std::string& prompt, int min, int max) {
        while (true) {
            std::string line = read_line(prompt);
            try {
                int val = std::stoi(line);
                if (val >= min && val <= max) return val;
            } catch (...) {}
            print_error("Please enter a number between " + std::to_string(min) + " and " + std::to_string(max));
        }
    }
};

// --- LOGIC FUNCTIONS ---
void print_entry(size_t index, const Entry& entry, bool reveal = false) {
    std::cout << BLUE << "  [" << index << "] " << WHITE << entry.service << RESET << '\n'
              << "        username: " << entry.username << '\n'
              << "        password: " << (reveal ? entry.password : std::string(entry.password.size() ? 8 : 0, '*')) << '\n';
    if (!entry.notes.empty()) {
        std::cout << "        notes:    " << entry.notes << '\n';
    }
}

void list_entries(const Vault& vault, bool reveal = false) {
    if (vault.empty()) {
        std::cout << DIM << "  (vault is empty)" << RESET << "\n";
        return;
    }
    for (size_t i = 0; i < vault.size(); ++i) {
        print_entry(i, vault.entries()[i], reveal);
    }
}

void add_entry(Vault& vault) {
    print_sub_header("ADD NEW ENTRY");
    Entry entry;
    entry.service = Console::read_line("Service: ");
    entry.username = Console::read_line("Username: ");
    
    std::string choice = Console::read_line("Generate password? [Y/n]: ");
    if (choice.empty() || choice == "y" || choice == "Y") {
        int length = Console::read_int("Length (8-64) [16]: ", 0, 64);
        if (length == 0) length = 16;
        if (length < 8) length = 8;
        entry.password = generate_password(length);
        std::cout << GREEN << "  Generated: " << entry.password << RESET << '\n';
    } else {
        entry.password = Console::read_hidden("Password: ");
    }
    
    entry.notes = Console::read_line("Notes (optional): ");
    vault.add_entry(entry);
    print_success("Entry added successfully!");
    press_enter();
}

void search_entries(const Vault& vault) {
    print_sub_header("SEARCH ENTRIES");
    std::string query = Console::read_line("Search: ");
    auto results = vault.search(query);
    
    if (results.empty()) {
        std::cout << DIM << "  No matches." << RESET << "\n";
        press_enter();
        return;
    }
    
    std::string reveal_choice = Console::read_line("Reveal passwords? [y/N]: ");
    bool reveal = (reveal_choice == "y" || reveal_choice == "Y");
    
    std::cout << "  Found " << results.size() << " entries:\n";
    for (size_t idx : results) {
        print_entry(idx, vault.entries()[idx], reveal);
    }
    press_enter();
}

void delete_entry(Vault& vault) {
    print_sub_header("DELETE ENTRY");
    if (vault.empty()) {
        std::cout << DIM << "  (vault is empty)" << RESET << "\n";
        press_enter();
        return;
    }
    
    list_entries(vault, false);
    int idx = Console::read_int("Index to delete: ", 0, vault.size() - 1);
    
    std::string confirm = Console::read_line("Confirm? [y/N]: ");
    if (confirm == "y" || confirm == "Y") {
        vault.remove_entry(idx);
        print_success("Entry deleted.");
    } else {
        std::cout << DIM << "  Cancelled." << RESET << "\n";
    }
    press_enter();
}

void update_entry(Vault& vault) {
    print_sub_header("UPDATE ENTRY");
    if (vault.empty()) {
        std::cout << DIM << "  (vault is empty)" << RESET << "\n";
        press_enter();
        return;
    }
    
    list_entries(vault, false);
    int idx = Console::read_int("Index to update: ", 0, vault.size() - 1);
    
    Entry entry = vault.entries()[idx];
    std::cout << DIM << "  Leave blank to keep current value." << RESET << "\n";
    
    std::string val;
    val = Console::read_line("Service [" + entry.service + "]: ");
    if (!val.empty()) entry.service = val;
    
    val = Console::read_line("Username [" + entry.username + "]: ");
    if (!val.empty()) entry.username = val;
    
    val = Console::read_hidden("Password (blank to keep): ");
    if (!val.empty()) entry.password = val;
    
    val = Console::read_line("Notes [" + entry.notes + "]: ");
    if (!val.empty()) entry.notes = val;
    
    vault.update_entry(idx, entry);
    print_success("Entry updated.");
    press_enter();
}

void generate_password_cmd() {
    print_sub_header("GENERATE PASSWORD");
    int length = Console::read_int("Length (8-64): ", 8, 64);
    std::string pw = generate_password(length);
    std::cout << GREEN << "  Password: " << pw << RESET << '\n';
    press_enter();
}

bool file_exists(const std::string& path) {
    std::ifstream f(path);
    if (f.good()) return true;
    std::ifstream f_parent("../" + path);
    return f_parent.good();
}

// --- MAIN MENU & START MENU ---
int print_start_menu() {
    clear_screen();
    print_banner(false);

    std::cout << CYAN << BOLD << "  +" << std::string(INNER_WIDTH, '=') << "+\n";
    std::string title = "WELCOME TO VAULT";
    int pad_left = (INNER_WIDTH - title.length()) / 2;
    std::cout << CYAN << BOLD << "  |" << std::string(pad_left, ' ') << title << std::string(INNER_WIDTH - pad_left - title.length(), ' ') << "|\n";
    std::cout << CYAN << BOLD << "  +" << std::string(INNER_WIDTH, '=') << "+\n" << RESET;

    std::cout << "\n";
    std::cout << CYAN << "  [1] " << WHITE << "Authenticate (ID + Password)" << RESET << "\n";
    std::cout << GREEN << "  [2] " << WHITE << "Create New Vault Account" << RESET << "\n";
    std::cout << RED  << "  [0] " << WHITE << "Exit" << RESET << "\n";
    std::cout << "\n";
    std::cout << MAGENTA << "  +" << std::string(INNER_WIDTH, '-') << "+\n" << RESET;
    
    return Console::read_int(YELLOW BOLD "  [>>] Choose option [0-2]: " RESET, 0, 2);
}

int print_menu(bool unlocked) {
    clear_screen();
    print_banner(false);

    std::string left_text = "  VAULT STATUS: ";
    std::string status_value = unlocked ? "[UNLOCKED]" : "[LOCKED]";
    std::string right_text = "VERSION 1.0";
    int padding = INNER_WIDTH - (left_text + status_value).length() - right_text.length();
    if (padding < 0) padding = 0;

    std::cout << DIM << "  +" << std::string(INNER_WIDTH, '=') << "+\n";
    std::cout << DIM << "  |" << BOLD << left_text << RESET;
    if (unlocked) std::cout << GREEN << status_value << RESET;
    else std::cout << YELLOW << status_value << RESET;
    std::cout << DIM << std::string(padding, ' ') << right_text << "|" << RESET << std::endl;
    std::cout << DIM << "  +" << std::string(INNER_WIDTH, '=') << "+\n\n" << RESET;

    std::cout << CYAN << BOLD << "  +" << std::string(INNER_WIDTH, '-') << "+\n";
    std::string menu_title = "M A I N   M E N U";
    int pad_left = (INNER_WIDTH - menu_title.length()) / 2;
    int pad_right = INNER_WIDTH - pad_left - menu_title.length();
    std::cout << CYAN << "  |" << std::string(pad_left, ' ') << BOLD << menu_title << RESET;
    std::cout << CYAN << std::string(pad_right, ' ') << "|" << RESET << "\n";
    std::cout << CYAN << BOLD << "  +" << std::string(INNER_WIDTH, '-') << "+\n" << RESET;
    
    std::cout << "\n";
    std::cout << BLUE << "  [1] " << WHITE << "List Entries" << RESET << "\n";
    std::cout << CYAN << "  [2] " << WHITE << "Add Entry" << RESET << "\n";
    std::cout << CYAN << "  [3] " << WHITE << "Search Entries" << RESET << "\n";
    std::cout << CYAN << "  [4] " << WHITE << "Update Entry" << RESET << "\n";
    std::cout << CYAN << "  [5] " << WHITE << "Delete Entry" << RESET << "\n";
    std::cout << CYAN << "  [6] " << WHITE << "Generate Password" << RESET << "\n";
    std::cout << CYAN << "  [7] " << WHITE << "Change Master Password" << RESET << "\n";
    std::cout << CYAN << "  [8] " << WHITE << "Save" << RESET << "\n";
    std::cout << YELLOW << "  [9] " << WHITE << "Save & Logout" << RESET << "\n";
    std::cout << RED  << "  [0] " << WHITE << "Back to Login (No Save)" << RESET << "\n";
    
    std::cout << MAGENTA << "\n  +" << std::string(INNER_WIDTH, '-') << "+\n" << RESET;
    return Console::read_int(YELLOW BOLD "  [>>] Choose option [0-9]: " RESET, 0, 9);
}

// --- MAIN ---
int main() {
    enable_ansi_colors();
    
    clear_screen();
    print_banner(true);
    fake_loading();
    
    Vault* vault = nullptr;
    bool unlocked = false;
    bool app_running = true;
    
    std::string VAULT_PATH;

    while (app_running) {
        int start_choice = print_start_menu();
        
        if (start_choice == 0) {
            std::cout << DIM << "  Goodbye!" << RESET << std::endl;
            break;
        }

        try {
            if (start_choice == 1) { // Authenticate
                clear_screen();
                print_sub_header("AUTHENTICATE");
                
                std::string user_id = Console::read_line("User ID: ");
                VAULT_PATH = "vault_" + user_id + ".dat"; 
                
                if (!file_exists(VAULT_PATH)) {
                    print_error("Account not found for ID: " + user_id);
                    press_enter();
                    continue; 
                }

                std::string password = Console::read_hidden("Master password: ");

                for (int attempts = 1; attempts <= 3; ++attempts) {
                    try {
                        vault = new Vault(Vault::load(VAULT_PATH, user_id, password));
                        unlocked = true;
                        break;
                    } catch (const std::exception& e) {
                        print_error("Invalid ID or Password (Attempt " + std::to_string(attempts) + "/3)");
                        if (attempts < 3) {
                            password = Console::read_hidden("Retry Master password: ");
                        } else {
                            print_error("Too many failed attempts. Returning to menu.");
                            vault = nullptr;
                            unlocked = false;
                            press_enter();
                        }
                    }
                }
            } 
            else if (start_choice == 2) { // Create New Vault
                clear_screen();
                print_sub_header("CREATE NEW VAULT ACCOUNT");
                
                std::string user_id = Console::read_line("Choose a User ID: ");
                VAULT_PATH = "vault_" + user_id + ".dat"; 
                
                if (file_exists(VAULT_PATH)) {
                    print_error("Account already exists for ID: " + user_id);
                    press_enter();
                    continue; 
                }
                
                std::string pw1 = Console::read_hidden("New master password: ");
                std::string pw2 = Console::read_hidden("Confirm: ");
                
                if (pw1 != pw2 || pw1.empty() || user_id.empty()) {
                    print_error("Passwords don't match, or ID/Password is empty.");
                    press_enter();
                    continue; 
                }
                
                vault = new Vault(user_id, pw1);
                fake_loading();
                vault->save(VAULT_PATH);
                unlocked = true;
                print_success("Vault created for " + user_id + " successfully!");
                press_enter();
            }
        } catch (const std::exception& e) {
            std::cerr << RED << BOLD << "  Fatal error: " << e.what() << RESET << "\n";
            press_enter();
            continue;
        }

        if (!vault) {
            continue;
        }

        bool logged_in = true;
        while (logged_in) {
            int choice = print_menu(unlocked);
            
            try {
                switch (choice) {
                    case 1: {
                        print_sub_header("LIST ENTRIES");
                        std::string reveal = Console::read_line("Reveal passwords? [y/N]: ");
                        list_entries(*vault, reveal == "y" || reveal == "Y");
                        press_enter();
                        break;
                    }
                    case 2: add_entry(*vault); break;
                    case 3: search_entries(*vault); break;
                    case 4: update_entry(*vault); break;
                    case 5: delete_entry(*vault); break;
                    case 6: generate_password_cmd(); break;
                    case 7: {
                        print_sub_header("CHANGE MASTER PASSWORD");
                        std::string new_pw1 = Console::read_hidden("  New master password: ");
                        std::string new_pw2 = Console::read_hidden("  Confirm: ");
                        if (new_pw1 == new_pw2 && !new_pw1.empty()) {
                            vault->change_master_password(new_pw1);
                            std::cout << YELLOW << "  Saving..." << RESET << std::endl;
                            fake_loading();
                            vault->save(VAULT_PATH); 
                            print_success("Password changed successfully.");
                        } else {
                            print_error("Passwords don't match.");
                        }
                        press_enter();
                        break;
                    }
                    case 8:
                        std::cout << YELLOW << "  Saving..." << RESET << std::endl;
                        fake_loading();
                        vault->save(VAULT_PATH); 
                        print_success("Vault saved to disk.");
                        press_enter();
                        break;
                    case 9: // SAVE & LOGOUT
                        std::cout << YELLOW << "  Saving..." << RESET << std::endl;
                        fake_loading();
                        vault->save(VAULT_PATH); 
                        print_success("Saved. Logging out...");
                        delete vault;
                        vault = nullptr;
                        unlocked = false;
                        press_enter();
                        logged_in = false;
                        break;
                    case 0: // BACK TO LOGIN (NO SAVE)
                        std::cout << DIM << "  Logging out without saving..." << RESET << "\n";
                        delete vault;
                        vault = nullptr;
                        unlocked = false;
                        press_enter();
                        logged_in = false; 
                        break;
                }
            } catch (const std::exception& e) {
                print_error(std::string("Error: ") + e.what());
                press_enter();
            }
        }
    }
    
    return 0;
}