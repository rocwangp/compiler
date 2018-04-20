#pragma once

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
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <algorithm>

namespace complier
{
    enum ID {
        IF = 1,
        ELSE  = 2,
        FOR = 3,
        WHILE = 4,
        BREAK = 5,
        CONTINUE = 6,
        RETURN = 7,
        INCLUDE = 8,

        ADD = 100,
        DEC = 101,
        MUL = 102,
        DIV = 103,
        ASSIGN = 104,
        ADD_ADD = 105,
        DEC_DEC = 106,
        ADD_ASSIGN = 107,
        DEC_ASSIGN = 108,
        LESS = 109,
        MORE = 110,
        EQUAL = 111,
        LESS_EQUAL = 112,
        MORE_EQUAL = 113,
        NO_EQUAL = 114,

        LEFT_SQUARE = 200,
        RIGHT_SQUARE = 201,
        LEFT_ROUND = 202,
        RIGHT_ROUND = 203,
        LEFT_CURLY = 204,
        RIGHT_CURLY = 205,
        COMMA = 206,
        SEMICOLON = 207,
        SINGLE_QUOTES = 208,
        DOUBLE_QUOTES = 209,

        CHAR = 300,
        INT = 301,
        LONG = 302,
        FLOAT = 303,
        DOUBLE = 304,

        VALUE = 500,
        NUMBER = 501,
        STRING = 502,

        COMMENT = 600,
        UNKNOWN = -1
    };

    struct Symbol
    {
        Symbol(ID id, std::string n, std::size_t l)
            : attr(id), name(n), loc(l)
        {  }
        ID attr;
        std::string name;
        std::size_t loc;
    };

    struct FirstSet
    {
        std::string token;
        std::vector<std::string> first_set;
    };
    struct Item {
        Item() {}
        Item(std::size_t d, const std::string& pc, const std::string& sc)
            : dot(d), prefix_code(pc), suffix_code(sc)
        {  }

        Item(std::size_t d, const std::string& pc, const std::string& sc, const std::vector<std::string>& prod)
            : dot(d), prefix_code(pc), suffix_code(sc), prod_vec(prod)
        {  }

        Item(Item&& other)
            : dot(std::move(other.dot)),
              prefix_code(std::move(other.prefix_code)),
              suffix_code(std::move(other.suffix_code)),
              prod_vec(std::move(other.prod_vec))
        {  }

        Item& operator=(const Item& other) {
            if(this != &other) {
                dot = other.dot;
                prefix_code = other.prefix_code;
                suffix_code = other.suffix_code;
                prod_vec = other.prod_vec;
            }
            return *this;
        }
        bool operator==(const Item& other) const {
            return dot == other.dot &&
                   prefix_code == other.prefix_code &&
                   suffix_code == other.suffix_code &&
                   prod_vec == other.prod_vec;
        }
        std::size_t id;
        std::size_t dot;
        std::string prefix_code;
        std::string suffix_code;
        std::vector<std::string> prod_vec;
    };

    class LexicalAnaysis
    {
    public:
        void anaysis(const std::string& filepath = "lexical") {
            read_grammer_(filepath);
            anaysis_lr();
        }
        void anaysis_predict(const std::vector<std::string>& operation) {
            std::size_t index = 0;
            std::list<std::string> pl;
            std::printf("%-10s%-10s\n\n", "stack", "operation");
            pl.emplace_front("$");
            pl.emplace_front(start_code_);
            while(index < operation.size()) {
                std::string line;
                for(auto& code : pl)
                    line.append(code);
                std::printf("%-10s", line.c_str());
                line.clear();
                for(std::size_t i = index; i < operation.size(); ++i)
                    line.append(operation[i]);
                std::printf("%-10s\n", line.c_str());

                if(operation[index] == pl.front()) {
                    pl.pop_front();
                    ++index;
                }
                else {
                    auto t = pl.front();
                    pl.pop_front();
                    if(t == "$") {
                        std::cerr << "error" << std::endl;
                        break;
                    }
                    for(auto it = predict_table_[t][operation[index]].rbegin();
                        it != predict_table_[t][operation[index]].rend(); ++it) {
                        pl.emplace_front(*it);
                    }
                }
            }
        }

    private:
        void anaysis_token(const std::string& filepath) {
            print_type_define();
            std::ifstream fin{ filepath, std::ios_base::in };
            std::string token;
            std::size_t row = 1;
            while(!fin.eof()) {
                char ch = fin.get();
                if(is_letter(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    while(is_letter(ch) || is_digit(ch) || ch == '_') {
                        token.append(1, ch);
                        ch = fin.get();
                    }
                    fin.unget();
                    if(auto code = query_keyword(token); code == ID::UNKNOWN) {
                        if(code = query_type(token); code == ID::UNKNOWN) {
                            insert_symbol_table(ID::VALUE, token);
                        }
                        else {
                            print_keyword(code, token, row);
                            /* insert_symbol_table(code, token); */
                        }
                    }
                    else {
                        print_keyword(code, token, row);
                    }
                    token.clear();
                }
                else if(is_digit(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    while(is_digit(ch) || ch == '.' || ch == 'e') {
                        token.append(1, ch);
                        ch = fin.get();
                    }
                    fin.unget();
                    /* print_keyword(ID::NUMBER, token, row); */
                    insert_symbol_table(ID::NUMBER, token);
                    token.clear();
                }
                else if(ch == '"') {
                    while((ch = fin.get()) != '\"') {
                        token.append(1, ch);
                    }
                    insert_symbol_table(ID::STRING, token);
                    token.clear();
                }
                else if(is_operator(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    if(ch == '*') {
                        token.pop_back();
                        while(!fin.eof() && (ch = fin.get()) != '*') {
                            token.append(1, ch);
                        }
                        if(fin.eof() || (ch = fin.get()) != '/') {
                            std::cout << "comment isn't match" << std::endl;
                            exit(1);
                        }
                        print_comment(ID::COMMENT, token, row);
                    }
                    else {
                        while(is_operator(ch)) {
                            token.append(1, ch);
                            ch = fin.get();
                        }
                        fin.unget();
                        print_operator(query_operator(token), token, row);
                    }
                    token.clear();
                }
                else if(is_delimiter(ch)) {
                    token.append(1, ch);
                    if(auto id = query_delimiter(token); id != -1) {
                        print_delimiter(id, token, row);
                    }
                    else {
                        std::cerr << "unknown delimiter" << std::endl;
                        exit(0);
                    }
                    token.clear();
                }
                else if(ch == '<') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        print_operator(ID::LESS_EQUAL, token, row);
                    }
                    else {
                        fin.unget();
                        print_operator(ID::LESS, token, row);
                    }
                    token.clear();
                }
                else if(ch == '>') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        print_operator(ID::MORE_EQUAL, token, row);
                    }
                    else {
                        fin.unget();
                        print_operator(ID::MORE, token, row);
                    }
                    token.clear();
                }
                else if(ch == '=') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        print_operator(ID::EQUAL, token, row);
                    }
                    else {
                        fin.unget();
                        print_operator(ID::ASSIGN, token, row);
                    }
                    token.clear();
                }
                else if(ch == '!') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        print_operator(ID::NO_EQUAL, token, row);
                    }
                    else {
                        fin.unget();
                        print_keyword(ID::VALUE, token, row);
                    }
                    token.clear();
                }
                else if(ch == '\n') {
                    ++row;
                }
            }
            print_symbol_table();
        }
        std::string token() {
            return tokens_.str();
        }

        /* 读取文法，解析出
         * code1 : { node1, node2, ... } -> { node1, node2, ... }
         * code2 : { node1, node2, ... } -> { node1, node2, ... }
         * 二维链表形式 */
        /* 记录当前已经找到First集的code */
        void read_grammer_(const std::string& filepath) {
            std::ifstream fin{ filepath, std::ios_base::in };
            while(!fin.eof()) {
                std::string line, head;
                std::getline(fin, line);
                std::size_t front = 0, back = 0;
                while(back != line.size()) {
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
                    else if(code != "->") {
                        if(code == "|") {
                            grammer_[head].emplace_back();
                        }
                        else {
                            grammer_[head].back().emplace_back(std::move(code));
                        }
                    }
                    front = back + 1;
                }
            }
            fin.close();
        }
        /* 计算每个非终结符的First集，规则如下
         * 如果非终结符A可以推出终结符，那么将终结符添加到A的First集中
         *  如A -> (* | + | @，则将(,*,@三个终结符添加到A的First集中
         * 如果非终结符A可以推出其它非终结符，那么将推出的非终结符的First集添加到A的First集中
         *  如A -> BC，则将B的First集添加到A的First集中
         * 如果A推出的非终结符的First集中包含空字符@，则接着添加
         *  如A -> BCD，B的First集中包含空字符@，则将B的First集添加后还需要继续添加C的First集，持续向后直到不再出现@
         */
        void anaysis_first_set() {
            auto grammer = grammer_;
            std::unordered_set<std::string> done_set;
            while(done_set.size() != grammer.size()) {
                for(auto& [code, code_lists] : grammer) {
                    bool done = true;
                    /* 对于每个code和它对应的文法链表，判断后面的内容是否可以添加到First集中
                     * 规则是不存在一个目前还没有找到First集的code */
                    for(auto& code_list : code_lists) {
                        if(code_list.empty()) {
                            continue;
                        }
                        if(grammer.count(code_list.front()) && !done_set.count(code_list.front())) {
                            done = false;
                            break;
                        }
                    }
                    /* code下的每个链表的第一个字符
                     * 或者可以添加到code的First集中
                     * 或者它的First集可以添加到code的First集中 */
                    if(done) {
                        for(auto it = code_lists.begin(); it != code_lists.end();) {
                            if(it->empty()) {
                                code_lists.erase(it++);
                            }
                            /* 非终结符，直接添加到code的First集中 */
                            else if(!grammer.count(it->front())) {
                                first_set_[code].emplace_back(std::move(it->front()));
                                /* 删除该链表 */
                                code_lists.erase(it++);
                            }
                            /* 已经找到First集的字符，将它的First集添加到code的First集中 */
                            else if(done_set.count(it->front())) {
                                bool has_empty = false;
                                /* 判断是否存在空字符，
                                 * 如果存在，说明需要考虑它后面的字符，此时不能删除这个链表，而只删除第一个字符即可
                                 * 如果不存在，直接删除链表 */
                                for(auto& s : first_set_[it->front()]) {
                                    if(s == "@")
                                        has_empty = true;
                                    first_set_[code].emplace_back(s);
                                }
                                if(has_empty) {
                                    it->pop_front();
                                    ++it;
                                }
                                else {
                                    code_lists.erase(it++);
                                }
                            }
                            else {
                                code_lists.erase(it++);
                            }
                        }
                        /* 如果code下已经没有链表存在，说明已经找到code的First集，记录在done_set中 */
                        if(code_lists.empty()) {
                            /* 排序去重 */
                            first_set_[code].sort();
                            first_set_[code].unique();
                            done_set.emplace(code);
                        }
                    }
                }
            }
            std::cout << "first set..." << std::endl;
            for(auto& [code, code_set]: first_set_) {
                std::cout << code << "\n";
                std::copy(code_set.begin(), code_set.end(), std::ostream_iterator<std::string>{std::cout, " "});
                std::cout << std::endl;
            }

        }
        /* 计算Follow集，规则如下
         * 设S为文法的开始符号，则将结束符$添加到Follow(S)中
         * 若 A -> aBC是一个产生式，则把First(C)的非空元素添加到Follow(B)中
         * 若 A -> aB是一个产生式，则把Follow(A)添加到Follow(B)中
         * 反复执行
         */
        void anaysis_follow_set() {
            /* 寻找每个非终结符的后缀字符，如存在产生式
             * A -> BC
             * D -> BE
             * 则非终结符B的后缀元素是C和E */
            std::unordered_map<std::string, std::list<std::string>> suffix_map;
            for(auto& [code, code_lists] : grammer_) {
                for(auto& code_list : code_lists) {
                    auto next = code_list.begin();
                    auto cur = next++;
                    while(next != code_list.end()) {
                        if(grammer_.count(*cur)) {
                            suffix_map[*cur].emplace_back(*next);
                        }
                        cur = next;
                        ++next;
                    }
                }
            }
            /* 将结束符添加到文法的开始符号的Follow中 */
            follow_set_[start_code_].emplace_back("$");
            /* 对每个非终结符，如果它的后缀字符
             * 是终结符，则直接添加到Follow集中
             * 是非终结符，则将该非终结符的First集的非空字符添加到Follow集中 */
            for(auto& [code, suffix_list] : suffix_map) {
                for(auto& suffix : suffix_list) {
                    if(!grammer_.count(suffix)) {
                        follow_set_[code].emplace_back(suffix);
                        continue;
                    }
                    for(auto& s : first_set_[suffix]) {
                        if(s != "@") {
                            follow_set_[code].emplace_back(s);
                        }
                    }
                }
            }
            /* 寻找每个非终结符的前缀字符，如存在产生式
             * A -> BC
             * D -> BE
             * 则非终结符C的前缀字符是A，E的前缀字符是D
             * 如果C的First集中存在空字符，则B的前缀字符是A
             * 如果E的First集中存在空字符，则B的前缀字符还有D */
            std::unordered_map<std::string, std::list<std::string>> prefix_map;
            for(auto& [code, code_lists] : grammer_) {
                for(auto& code_list : code_lists) {
                    if(grammer_.count(code_list.back())) {
                        if(code != code_list.back())
                            prefix_map[code_list.back()].emplace_back(code);
                    }
                    if(code_list.size() > 1) {
                        auto prev_end = --(code_list.end());
                        auto end = prev_end--;
                        if(!grammer_.count(*end)) {
                            continue;
                        }
                        if(std::find(first_set_[*end].begin(), first_set_[*end].end(), "@") != first_set_[*end].end()) {
                            if(code != *prev_end)
                                prefix_map[*prev_end].emplace_back(code);
                        }
                    }
                }
            }
            /* 对于每个非终结符，将它的前缀字符的Follow集添加到自己的Follow集中
             * done_set表示已经完整求出Follow集并且以前在prefix_map中的非终结符 */
            std::unordered_set<std::string> done_set;
            while(done_set.size() != prefix_map.size()) {
                for(auto& [code, code_list] : prefix_map) {
                    for(auto it = code_list.begin(); it != code_list.end();) {
                        if(done_set.count(*it) || !prefix_map.count(*it)) {
                            follow_set_[code].insert(follow_set_[code].end(), follow_set_[*it].begin(), follow_set_[*it].end());
                            code_list.erase(it++);
                        }
                        else {
                            ++it;
                        }
                    }
                    if(code_list.empty()) {
                        done_set.emplace(code);
                    }
                }
            }
            std::cout << "follow set..." << std::endl;
            for(auto& [code, code_list] : follow_set_) {
                code_list.sort();
                code_list.unique();
                std::cout << code << "\n";
                std::copy(code_list.begin(), code_list.end(), std::ostream_iterator<std::string>{std::cout, " "});
                std::cout << std::endl;
            }
        }
        /* 构造select集，实际上是直接求预测分析表，对于形如
         * A -> BC的产生式，select(A)是B的First集
         * A -> *C的产生式，select(A)是非空终结符*
         * A -> @C的产生式，select(A)是A的Follow集
         *
         * 预测分析表的构造
         * 当A遇到select(A)中的字符时，将A替换成构造select(A)的产生式 */
        void anaysis_select_set() {
            for(auto& [code, code_lists] : grammer_) {
                for(auto& code_list : code_lists) {
                    if(!grammer_.count(code_list.front())) {
                        if(code_list.front() != "@") {
                            predict_table_[code][code_list.front()] = code_list;
                        }
                        else {
                            for(auto& follow : follow_set_[code])
                                predict_table_[code][follow] = code_list;
                        }
                    }
                    else {
                        for(auto& first : first_set_[code_list.front()])
                            predict_table_[code][first] = code_list;
                    }
                }
            }
        }
        void print_predict_table() {
            std::unordered_set<std::string> ter_symbols, noter_symbols;
            for(auto& [code, code_lists] : grammer_) {
                noter_symbols.emplace(code);
                for(auto& code_list : code_lists) {
                    for(auto& s : code_list) {
                        if(!grammer_.count(s)) {
                            ter_symbols.emplace(s);
                        }
                    }
                }
            }
            std::printf("%-7s", "");
            for(auto& symbol : ter_symbols) {
                std::printf("%-7s", symbol.c_str());
            }
            std::printf("\n");

            for(auto& symbol : noter_symbols) {
                std::printf("%-7s", symbol.c_str());
                for(auto& ts : ter_symbols) {
                    if(predict_table_[symbol].count(ts)) {
                        std::string code;
                        for(auto& s : predict_table_[symbol][ts])
                            code.append(s);
                        std::printf("%-7s", code.c_str());
                    }
                    else {
                        std::printf("%-7s", "");
                    }
                }
                std::printf("\n");
            }
        }
        std::vector<std::string> query_first_set(std::string_view code) {
            std::set<std::string> fs;
            std::list<std::string_view> code_list;
            std::size_t front = 0, back = 0;
            while(back < code.length()) {
                back = code.find_first_of(' ', front);
                back = (back == std::string_view::npos) ? code.length() : back;
                code_list.emplace_back(code.substr(front, back - front));
                front = back + 1;
            }
            while(!code_list.empty()) {
                std::string head{ code_list.front().data(), code_list.front().length() };
                if(!first_set_.count(head)) {
                    fs.emplace(std::move(head));
                    break;
                }
                else {
                    bool has_empty = false;
                    for(auto& s : first_set_[head]) {
                        if(s == "@")
                            has_empty = true;
                        fs.emplace(std::move(s));
                    }
                    if(has_empty)
                        code_list.pop_front();
                    else
                        break;
                }
            }
            /* return std::vector<std::string>{ fs.begin(), fs.end() }; */
            std::vector<std::string> code_fs;
            code_fs.reserve(fs.size());
            for(auto& s : fs) {
                code_fs.emplace_back(std::move(s));
            }
            return code_fs;
        }
        void anaysis_lr() {
            anaysis_items();
            anaysis_action();
            std::cout << "..........closure..............\n";
            for(std::size_t i = 0; i != closure_.size(); ++i) {
                std::cout << i << std::endl;
                for(auto& item : closure_[i]) {
                    std::cout << item.prefix_code << " -> ";
                    for(std::size_t i = 0; i != item.dot; ++i) {
                        std::cout << item.prod_vec[i] << " ";
                    }
                    std::cout << ". ";
                    for(std::size_t i = item.dot; i != item.prod_vec.size(); ++i) {
                        std::cout << item.prod_vec[i] << " ";
                    }
                    std::cout << ", " << item.suffix_code << std::endl;
                }
                std::cout << std::endl;
            }

            std::cout << "..........goto..............\n";
            for(auto& [s, m] : goto_) {
                for(auto& [token, d] : m) {
                    std::cout << s << " " << token << " " << d << std::endl;
                }
            }

            std::cout << "..........action..............\n";
            /* for(auto& [s, m] : action_) { */
            /*     for(auto& [token, d] : m) { */
            /*         std::cout << s << "--" << token << "--"; */
            /*         if(d.type() == typeid(std::size_t)) { */
            /*             std::cout << "s " << std::any_cast<std::size_t>(d); */
            /*         } */
            /*         else if(d.type() == typeid(std::string)) { */
            /*             std::cout << "acc"; */
            /*         } */
            /*         else { */
            /*             auto [prefix, n] = std::any_cast<std::pair<std::string, std::size_t>>(d); */
            /*             std::cout << "r " << prefix << " " << n; */
            /*         } */
            /*         std::cout << "\n\n"; */
            /*     } */
            /* } */

            std::unordered_set<std::string> symbol_set;
            for(auto& [code, code_lists] : grammer_) {
                symbol_set.insert(code);
                for(auto& code_list : code_lists) {
                    for(auto& s : code_list) {
                        symbol_set.insert(s);
                    }
                }
            }
            std::vector<std::string> ter_symbols, noter_symbols;
            for(auto& symbol : symbol_set) {
                if(grammer_.count(symbol)) {
                    if(symbol == start_code_ + "0") {
                        continue;
                    }
                    noter_symbols.emplace_back(symbol);
                }
                else {
                    ter_symbols.emplace_back(symbol);
                }
            }
            ter_symbols.emplace_back("$");
            std::printf("%-10s", "");
            for(auto& symbol : ter_symbols) {
                std::printf("%-10s", symbol.c_str());
            }
            for(auto& symbol : noter_symbols) {
                std::printf("%-10s", symbol.c_str());
            }
            std::printf("\n");
            for(std::size_t i = 0; i != closure_.size(); ++i) {
                std::printf("%-10d", (unsigned int)i);
                auto print_action = [&](auto& symbol) {
                    if(action_[i].count(symbol)) {
                        auto& d = action_[i][symbol];
                        if(d.type() == typeid(std::size_t)) {
                            std::string s = "s" + std::to_string(std::any_cast<std::size_t>(d));
                            std::printf("%-10s", s.c_str());
                            /* std::cout << "s " << std::any_cast<std::size_t>(d); */
                        }
                        else if(d.type() == typeid(std::string)) {
                            std::printf("%-10s", "acc");
                            /* std::cout << "acc"; */
                        }
                        else {
                            auto [prefix, n] = std::any_cast<std::pair<std::string, std::size_t>>(d);
                            prefix.append(std::to_string(n));
                            prefix = "r" + prefix;
                            std::printf("%-10s", prefix.c_str());
                            /* std::cout << "r " << prefix << " " << n; */
                        }
                    }
                    else {
                        std::printf("%-10s", "");
                    }
                };
                for(auto& symbol : ter_symbols) {
                    print_action(symbol);
                }
                for(auto& symbol : noter_symbols) {
                    print_action(symbol);
                }
                std::printf("\n");
            }
        }
        void anaysis_closure(std::vector<Item>& item_set) {
            while(true) {
                bool done = true;
                for(std::size_t i = 0; i != item_set.size(); ++i) {
                    std::size_t dot = item_set[i].dot;
                    if(dot >= item_set[i].prod_vec.size()) {
                        continue;
                    }
                    std::string s;
                    for(auto it = item_set[i].prod_vec.begin() + dot + 1; it != item_set[i].prod_vec.end(); ++it) {
                        s.append(*it);
                        s.append(1, ' ');
                    }
                    s.append(item_set[i].suffix_code);
                    auto fs = query_first_set(s);
                    if(fs.empty()) {
                        continue;
                    }
                    for(auto& [code, code_lists] : grammer_) {
                        if(code != item_set[i].prod_vec[dot]) {
                            continue;
                        }
                        for(auto& code_list : code_lists) {
                            for(auto& symbol : fs) {
                                if(grammer_.count(symbol)) {
                                    continue;
                                }
                                if(symbol.empty()) {
                                    continue;
                                }
                                Item new_item{ 0, code, symbol };
                                new_item.prod_vec = std::vector<std::string>{ code_list.begin(), code_list.end() };
                                if(std::find(item_set.begin(),
                                             item_set.end(),
                                             new_item) != item_set.end()) {
                                    continue;
                                }
                                item_set.emplace_back(std::move(new_item));
                                done = false;
                            }
                        }
                    }
                }
                if(done) {
                    break;
                }
            }
        }

        void anaysis_goto(std::size_t closure_id, const std::string& suffix) {
            std::vector<Item> item_set;
            for(auto& item : closure_[closure_id]) {
                auto& prod = item.prod_vec;
                if(item.dot != prod.size() && prod[item.dot] == suffix) {
                    Item new_item { item.dot + 1, item.prefix_code, item.suffix_code };
                    new_item.prod_vec = prod;
                    if(std::find(item_set.begin(),
                                 item_set.end(),
                                 new_item) != item_set.end()) {
                        continue;
                    }
                    item_set.emplace_back(std::move(new_item));
                }
            }
            anaysis_closure(item_set);
            if(!item_set.empty()) {
                if(auto it = std::find(closure_.begin(), closure_.end(), item_set);
                        it == closure_.end()) {
                    closure_.emplace_back(std::move(item_set));
                    goto_[closure_id][suffix] = closure_.size() - 1;
                }
                else {
                    goto_[closure_id][suffix] = it - closure_.begin();
                }
            }
        }
        void anaysis_items() {
            grammer_[start_code_ + "0"].emplace_back(std::list{ start_code_ });
            anaysis_first_set();

            std::unordered_set<std::string> symbol_set;
            for(auto& [code, code_lists] : grammer_) {
                symbol_set.insert(code);
                for(auto& code_list : code_lists) {
                    for(auto& s : code_list) {
                        symbol_set.insert(s);
                    }
                }
            }
            std::vector<std::string> symbols;
            symbols.reserve(symbol_set.size());
            for(auto& symbol : symbol_set) {
                symbols.emplace_back(symbol);
            }

            std::vector<Item> item_set;
            item_set.emplace_back( 0, start_code_ + "0", "$", std::vector{ start_code_ }) ;
            anaysis_closure(item_set);
            closure_.emplace_back(std::move(item_set));

            for(std::size_t i = 0; i != closure_.size(); ++i) {
                for(auto& symbol : symbols) {
                    anaysis_goto(i, symbol);
                }
            }
        }
        void anaysis_action() {
            for(std::size_t i = 0; i != closure_.size(); ++i) {
                for(auto& proj : closure_[i]) {
                    auto dot = proj.dot;
                    auto& pv = proj.prod_vec;
                    auto& prefix = proj.prefix_code;
                    auto& suffix = proj.suffix_code;
                    if(pv.size() == 1 && pv.front() == start_code_ && dot == 1 && prefix == start_code_ + "0" && suffix == "$") {
                        action_[i]["$"] = std::string{ "acc" };
                        continue;
                    }
                    else if(dot != pv.size() && goto_[i].count(pv[dot])) {
                        action_[i][pv[dot]] = std::size_t{ goto_[i][pv[dot]] };
                    }
                    else if(dot == pv.size() && prefix != start_code_ + "0") {
                        action_[i][suffix] = std::make_pair(prefix, dot);
                    }
                }
            }
        }
    private:
        bool is_letter(char ch) {
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        }
        bool is_digit(char ch) {
            return (ch >= '0' && ch <= '9');
        }
        bool is_operator(char ch) {
            return operator_table.count({ &ch, 1 }) == 1;
        }
        bool is_delimiter(char ch) {
            return delim_table.count({ &ch, 1 }) == 1;
        }
        bool is_type(char ch) {
            return type_table.count({ &ch, 1 }) == 1;
        }
        ID query_keyword(std::string_view token) {
            if(auto it = keyword_table.find(token); it != keyword_table.end()) {
                return it->second;
            }
            else {
                return ID::UNKNOWN;
            }
        }
        ID query_operator(std::string_view token) {
            if(auto it = operator_table.find(token); it != operator_table.end()) {
                return it->second;
            }
            else {
                return ID::UNKNOWN;
            }
        }
        ID query_delimiter(std::string_view token) {
            if(auto it = delim_table.find(token); it != delim_table.end()) {
                return it->second;
            }
            else {
                return ID::UNKNOWN;
            }
        }
        ID query_type(std::string_view token) {
            if(auto it = type_table.find(token); it != type_table.end()) {
                return it->second;
            }
            else {
                return ID::UNKNOWN;
            }
        }
        void print_keyword(ID id, std::string_view token, std::size_t row) {
            tokens_ << id << " " << token << " " << row << "\n";
            if(id == ID::VALUE) {
                std::cout << "<VARIABLE> <" << token << "> <" << row << ">" << std::endl;
            }
            else if(id == ID::NUMBER) {
                std::cout << "<NUMBER> <" << token << "> <" << row << ">" << std::endl;
            }
            else {
                std::cout << "<KEYWORD> <" << token << "> <" << row << ">" << std::endl;
            }
        }
        void print_operator(ID id, std::string_view token, std::size_t row) {
            std::cout << "<OPERATOR> <" << token << "> <" << row << ">" << std::endl;
            tokens_ << id << " " << token << " " << row << "\n";
        }
        void print_delimiter(ID id, std::string_view token, std::size_t row) {
            std::cout << "<DELIMITER> <" << token << "> <" << row << ">" << std::endl;
            tokens_ << id << " " << token << " " << row << "\n";
        }
        void print_type(ID id, std::string_view token, std::size_t row) {
            std::cout << "<TYPE> <" << token << "> <" << row << ">" << std::endl;
            tokens_ << id << " " << token << " " << row << "\n";
        }
        void print_comment(ID id, std::string_view token, std::size_t row) {
            std::cout << "<COMMENT> <" << token << "> <" << row << ">" << std::endl;
            tokens_ << id << " " << token << " " << row << "\n";
        }
        void print_symbol_table() {
            for(auto& symbol : symbol_table_) {
                std::cout << "token: {"
                          << "name = " << symbol.name << " "
                          << "attr = " << symbol.attr << " "
                          << "loc = " << symbol.loc << " "
                          << "}"
                          << std::endl;
            }
        }
        void print_type_define() {
            std::cout << "variable name: VARIABLE\n"
                      << "delimiter flags: DELIMITER\n"
                      << "keywords: KEYWORD\n"
                      << "type: TYPE\n"
                      << "operator: OPERATOR\n"
                      << std::endl;
        }
        void insert_symbol_table(ID id, std::string_view token) {
            if(symbol_set_.count(token) == 0) {
                std::string str{ token.data(), token.length() };
                symbol_set_.insert(str);
                symbol_table_.emplace_back(id, str, symbol_table_.size());
            }
        }
    private:
        std::string start_code_;
        std::stringstream tokens_;
        std::vector<Symbol> symbol_table_;
        std::unordered_set<std::string_view> symbol_set_;

        std::unordered_map<std::string, std::list<std::list<std::string>>> grammer_;

        std::vector<std::vector<Item>> closure_;
        std::unordered_map<std::size_t, std::unordered_map<std::string, std::size_t>> goto_;
        std::unordered_map<std::size_t, std::unordered_map<std::string, std::any>> action_;


        std::unordered_map<std::string, std::vector<std::vector<std::string>>> project_set_;

        std::unordered_map<std::string, std::list<std::string>> first_set_;
        std::unordered_map<std::string, std::list<std::string>> follow_set_;
        std::unordered_map<std::string, std::list<std::string>> select_set_;
        std::unordered_map<std::string, std::unordered_map<std::string, std::list<std::string>>> predict_table_;

        static std::unordered_map<std::string_view, ID> keyword_table;
        static std::unordered_map<std::string_view, ID> operator_table;
        static std::unordered_map<std::string_view, ID> delim_table;
        static std::unordered_map<std::string_view, ID> type_table;
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::keyword_table = {
        { "if", IF }, { "else", ELSE },
        { "for", FOR }, { "while", WHILE}, { "break", BREAK }, { "continue", CONTINUE },
        { "return", RETURN }, { "include", INCLUDE }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::operator_table = {
        { "+", ADD }, { "-", DEC }, { "*", MUL }, { "/", DIV }, { "=", ASSIGN },
        { "++", ADD_ADD }, { "--", DEC_DEC }, { "+=", ADD_ASSIGN }, { "-=", DEC_ASSIGN },
        { "<", LESS }, { ">", MORE }, { "==", EQUAL }, { "!=", NO_EQUAL },
        { "<=", LESS_EQUAL }, { ">=", MORE_EQUAL }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::delim_table = {
        { "[", LEFT_SQUARE }, { "]", RIGHT_SQUARE },
        { "(", LEFT_ROUND }, { ")", RIGHT_ROUND },
        { "{", LEFT_CURLY }, { "}", RIGHT_CURLY },
        { ",", COMMA }, { ";", SEMICOLON },
        { "'", SINGLE_QUOTES }, { "\"", DOUBLE_QUOTES }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::type_table = {
        { "char", CHAR }, { "int", INT }, { "float", FLOAT }, { "double", DOUBLE },
        { "string", STRING }
    };
}
