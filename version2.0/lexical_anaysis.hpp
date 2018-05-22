#pragma once

#include <type_traits>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <any>
#include <exception>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <algorithm>

namespace complier
{
    struct Node
    {
        std::string symbol;
        std::string attr;
    };

    struct Symbol
    {
        std::string name;
        std::string value;
        std::string type;
    };

    class LexicalAnaysis
    {
    public:
        void read_grammer(std::ifstream& fin) {
            while(!fin.eof()) {
                std::string line, head;
                std::getline(fin, line);
                std::size_t front = 0, back = 0;
                while(back != line.size() && front != std::string::npos) {
                    back = line.find_first_of(' ', front);
                    back = (back == std::string::npos) ? line.size() : back;
                    std::string code = line.substr(front, back - front);
                    if(front == 0) {
                        head.swap(code);
                        grammer_[head].emplace_back();
                        if(start_code_.empty()) {
                            start_code_ = head;
                        }
                    }
                    else if(code == ";") {
                        break;
                    }
                    else if(code != "->") {
                        if(code == "|") {
                            grammer_[head].emplace_back();
                        }
                        else {
                            if(code == "\\;") {
                                code = ";";
                            }
                            grammer_[head].back().emplace_back(std::move(code));
                        }
                    }
                    front = line.find_first_not_of(' ', back + 1);
                }
            }
        }
        void anaysis_first_set() {

        }
        void anaysis_tokens(std::ifstream& fin) {
            std::string token;
            while(!fin.eof()) {
                char ch = fin.get();
                if(std::isalpha(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    while(std::isalpha(ch) || std::isdigit(ch) || ch == '_') {
                        token.append(1, ch);
                        ch = fin.get();
                    }
                    fin.unget();
                    if(keywords_.count(token)) {
                        tokens_.emplace_back(token, "", token);
                    }
                    else {
                        tokens_.emplace_back(token, "", "IDN");
                    }
                }
                else if(std::isdigit(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    bool is_real{ false };
                    while(std::isdigit(ch) || ch == '.' || ch == 'e') {
                        if(ch == '.') {
                            is_real = true;
                        }
                        token.append(1, ch);
                        ch = fin.get();
                    }
                    fin.unget();
                    if(is_real) {
                        tokens_.emplace_back(token, token, "REAL");
                    }
                    else {
                        tokens_.emplace_back(token, token, "INT");
                    }
                }
                else if(ch == '\"') {
                    token.append(1, ch);
                    while((ch = fin.get()) != '\"') {
                        token.append(1, ch);
                    }
                    token.append(1, ch);
                    tokens_.emplace_back(token, token, "STRING");
                }
                else if(ch == '\'') {
                    token.append(1, ch);
                    while((ch = fin.get()) != '\'') {
                        token.append(1, ch);
                    }
                    token.append(1, ch);
                    tokens_.emplace_back(token, token, "CHAR");
                }
                else  if(is_operation(ch)) {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '*') {
                        token.pop_back();
                        while(!fin.eof() && (ch = fin.get()) != '/') {
                            ;
                        }
                    }
                    else {
                        while(is_operation(ch)) {
                            token.append(1, ch);
                            ch = fin.get();
                        }
                        fin.unget();
                        tokens_.emplace_back(token, "", token);
                    }
                }
                else if(is_delimiter(ch)) {
                    token.append(1, ch);
                    if(delimiters_.count(token)) {
                        tokens_.emplace_back(token, "", token);
                    }
                    else {
                        exit(0);
                    }
                }
                else if(ch == '<') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                    }
                    else {
                        fin.unget();
                    }
                    tokens_.emplace_back(token, "", token);
                    token.clear();
                }
                else if(ch == '>') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                    }
                    else {
                        fin.unget();
                    }
                    tokens_.emplace_back(token, "", token);
                    token.clear();
                }
                else if(ch == '=') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                    }
                    else {
                        fin.unget();
                    }
                    tokens_.emplace_back(token, "", token);
                    token.clear();
                }
                else if(ch == '!') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                    }
                    else {
                        fin.unget();
                    }
                    tokens_.emplace_back(token, "", token);
                    token.clear();
                }
                token.clear();
            }
        }
        bool is_operation(char ch) {
            return ch == '+' || ch == '-' || ch == '*' || ch == '/';
        }
        bool is_delimiter(char ch) {
            return delimiters_.count(std::string(&ch, 1));
        }

    private:
        std::string start_code_;
        std::unordered_map<std::string, std::list<std::list<std::string>>> grammer_;
        std::unordered_map<std::string, std::string> keywords_;
        std::unordered_map<std::string, std::string> delimiters_;
        std::vector<Symbol> tokens_;
    };
}
