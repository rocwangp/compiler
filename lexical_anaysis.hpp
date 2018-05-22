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
#include <exception>
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
        DO = 9,
        SWITCH = 10,
        CASE = 11,
        DEFAULT = 12,
        DEF,
        AND,
        OR,
        LOG_AND,
        LOG_OR,


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
        MAOHAO = 210,

        CHAR = 300,
        INT = 301,
        LONG = 302,
        FLOAT = 303,
        DOUBLE = 304,
        BOOL = 305,
        VOID = 306,


        VALUE = 500,
        NUMBER = 501,
        STRING = 502,

        COMMENT = 600,
        UNKNOWN = -1
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

        Item(const Item& other)
            : dot(other.dot),
              prefix_code(other.prefix_code),
              suffix_code(other.suffix_code),
              prod_vec(other.prod_vec)
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
        std::size_t dot;
        std::string prefix_code;
        std::string suffix_code;
        std::vector<std::string> prod_vec;
    };


    struct Node
    {
        std::string symbol;
        std::string attr;
    };

    struct Token
    {
        Token() {}
        Token(std::size_t i, std::string tt, std::string n)
            : index(i), token_type(tt), name(n) {}
        std::size_t index;
        std::string token_type;
        std::string name;
        std::string value;
        std::string type;

        std::vector<std::string> jump_stmt;
        std::map<std::size_t, std::string> true_list, false_list, next_list;
        std::size_t instr;
    };

    struct Symbol
    {
        Symbol() {}
        Symbol(std::string t, std::string n)
            : type(t), name(n) {}
        std::string type;
        std::string name;
        std::string value;
    };

    class LexicalAnaysis
    {
    public:
        void anaysis(const std::string& filepath = "lexical") {
            read_grammer(filepath);
            anaysis_token("test_program.cc");
            anaysis_lr();
        }
        void anaysis_operation() {

            std::cout << "...................anaysis................\n";
            std::list<std::size_t> state_list;
            std::list<Token> token_list;

            state_list.emplace_front(0);
            token_list.emplace_back();

            Token empty_token{ 0, "$", "$" };
            while(!tokens_.empty()) {
                std::string s;
                for(auto it = state_list.rbegin(); it != state_list.rend(); ++it) {
                    s.append(std::to_string(*it));
                    s.append(1, ' ');
                }
                if(!s.empty())  s.pop_back();
                std::printf("%s", s.c_str());

                s.clear();
                for(auto& token : tokens_) {
                    s.append(token.token_type);
                    s.append(1, ' ');
                }
                if(!s.empty())  s.pop_back();
                std::printf("\n%s", s.c_str());

                auto& token = tokens_.front();
                std::size_t state = state_list.front();
                if(!action_[state].count(token.token_type)) {
                    for(auto& [input, output] : goto_[state]) {
                        std::cout << state << " " << input << " " << output << std::endl;
                    }
                    throw std::runtime_error("tokens_ is error in LR(1)");
                }
                auto& transfer = action_[state][token.token_type];
                if(transfer.type() == typeid(std::size_t)) {

                    state_list.emplace_front(std::any_cast<std::size_t>(transfer));
                    token_list.emplace_front(token);

                    tokens_.pop_front();
                    std::printf("\n%s\n\n", "shift");
                }
                else if(transfer.type() == typeid(std::string)) {
                    std::printf("\n%s\n\n", "acc");
                    break;
                }
                else {
                    auto [prefix, pp] = std::any_cast<std::pair<std::string, std::pair<std::size_t, std::vector<std::string>>>>(transfer);
                    auto [cnt, pv] = pp;

                    while(cnt--) {
                        state_list.pop_front();
                    }

                    /* tokens_.emplace_front(0, prefix, prefix); */
                    state = state_list.front();
                    auto& t = action_[state][prefix];

                    state_list.emplace_front(std::any_cast<std::size_t>(t));
                    /* attr_list.emplace_front(get_attr_from_token(prefix)); */

                    std::string production = prefix;
                    production.append(" ->");
                    for(auto& str : pv) {
                        production.append(" " + str);
                    }
                    std::cout << "\n";

                    if(production == "var_decl -> ident = expr") {
                        Token expr = token_list.front();
                        token_list.pop_front();
                        Token assign = token_list.front();
                        token_list.pop_front();
                        Token idn = token_list.front();
                        token_list.pop_front();

                        idn.token_type = "var_decl";
                        idn.value = expr.value;
                        idn.type = expr.type;

                        std::cout << "......................................." << std::endl;
                        Symbol symbol = symbol_table_[idn.index];
                        std::string s(symbol.name + " = " + expr.name);
                        append_to_conv_result(new_list_no(), s);
                        /* std::printf("%s = %s\n", symbol.name.data(), expr.name.data()); */
                        std::cout << "......................................." << std::endl;

                        token_list.push_front(idn);
                    }
                    else if(production == "numeric -> INT") {
                        token_list.front().token_type = "numeric";
                        token_list.front().type = "int";
                        token_list.front().value = token_list.front().name;
                    }
                    else if(production == "numeric -> REAL") {
                        token_list.front().token_type = "numeric";
                        token_list.front().type = "double";
                        token_list.front().value = token_list.front().name;
                    }
                    else if(production == "expr -> numeric") {
                        Token num = token_list.front();
                        token_list.pop_front();

                        Token expr;
                        expr.token_type = "expr";
                        expr.name = new_tmp_name();
                        expr.value = num.value;
                        expr.type = num.type;

                        std::cout << "......................................." << std::endl;
                        std::cout << expr.name << " " << expr.value << " " << expr.type << std::endl;
                        std::string s(expr.name + " = " + num.value);
                        append_to_conv_result(new_list_no(), s);
                        std::cout << "......................................." << std::endl;

                        token_list.push_front(expr);
                    }
                    else if(production == "expr -> expr operation expr") {
                        Token expr2 = token_list.front();
                        token_list.pop_front();
                        Token op = token_list.front();
                        token_list.pop_front();
                        Token expr1 = token_list.front();
                        token_list.pop_front();

                        std::cout << expr1.type << " " << expr2.type << std::endl;
                        Token expr;
                        expr.token_type = "expr";
                        expr.name = new_tmp_name();
                        if(expr1.type == "double" || expr2.type == "double") {
                            expr.value = std::to_string(binary_operation<double>(op.name, expr1.value, expr2.value));
                            expr.type = "double";
                        }
                        else if(expr1.type == "int" && expr2.type == "int") {
                            expr.value = std::to_string(binary_operation<int>(op.name, expr1.value, expr2.value));
                            expr.type = "int";
                        }
                        else {
                            /* throw std::runtime_error("binary operation type error: " + expr1.type + op.name + expr2.type); */
                        }

                        std::cout << "......................................." << std::endl;
                        std::cout << expr.name << " " << expr.value << " " << expr.type << std::endl;
                        /* std::printf("%s = %s %s %s\n", expr.name.data(), expr1.name.data(), op.name.data(), expr2.name.data()); */
                        std::string s(expr.name + " = " + expr1.name + " " + op.name + " " + expr2.name);
                        append_to_conv_result(new_list_no(), s);
                        std::cout << "......................................." << std::endl;

                        token_list.push_front(expr);
                    }
                    else if(production == "expr -> ident") {
                        Token idn = token_list.front();
                        token_list.pop_front();

                        if(idn.index >= symbol_table_.size()) {
                            throw std::runtime_error("no variable: " + idn.name + " in symbol table");
                        }
                        Symbol symbol = symbol_table_[idn.index];
                        Token expr;
                        expr.token_type = "expr";
                        expr.name = new_tmp_name();
                        expr.type = symbol.type;
                        expr.value = symbol.value;

                        std::cout << "......................................." << std::endl;
                        std::string s(expr.name + " = " + symbol.name + "\t(" + expr.name + " type: " + expr.type + ")");
                        append_to_conv_result(new_list_no(), s);
                        std::cout << "......................................." << std::endl;

                        token_list.push_front(expr);
                    }
                    else if(production == "expr -> ident = expr") {
                        Token expr = token_list.front();
                        token_list.pop_front();
                        Token assign = token_list.front();
                        token_list.pop_front();
                        Token idn = token_list.front();
                        token_list.pop_front();

                        idn.value = expr.value;
                        idn.type = expr.type;

                        std::cout << "......................................." << std::endl;
                        for(auto& s : symbol_table_) {
                            std::cout << s.name << " " << s.type << " " << s.value << std::endl;
                        }
                        Symbol& symbol = symbol_table_[idn.index];
                        symbol.type = idn.type;
                        symbol.name = idn.name;
                        symbol.value = idn.value;
                        for(auto& s : symbol_table_) {
                            std::cout << s.name << " " << s.type << " " << s.value << std::endl;
                        }
                        std::string s(symbol.name + " = " + expr.name + "\t(" + symbol.name + " type: " + idn.type + ")");
                        append_to_conv_result(new_list_no(), s);
                        std::cout << "......................................." << std::endl;

                        idn.token_type = "expr";
                        token_list.push_front(idn);
                    }
                    else if(production == "M -> @") {
                        Token m;
                        m.name = "M";
                        m.instr = new_list_no(false);
                        std::cout << m.instr << std::endl;

                        token_list.push_front(m);
                    }
                    else if(production == "N -> @") {
                        Token n;
                        n.name = "N";
                        std::size_t num = new_list_no();
                        n.next_list[num] = std::to_string(num) + ":goto ";
                        append_to_conv_result(num, n.next_list[num]);
                        token_list.push_front(n);
                    }
                    else if(production == "logical_stmt -> expr comparsion expr") {
                        Token expr2 = front_and_pop(token_list);
                        Token comp = front_and_pop(token_list);
                        Token expr1 = front_and_pop(token_list);

                        Token stmt;
                        std::size_t n = new_list_no();
                        stmt.true_list[n] = std::to_string(n) + std::string(":if ") + expr1.name + " " + comp.name + " " + expr2.name + std::string( " goto " );
                        append_to_conv_result(n, stmt.true_list[n]);
                        n = new_list_no();
                        stmt.false_list[n] = std::to_string(n) + std::string(":goto ");
                        append_to_conv_result(n, stmt.false_list[n]);

                        stmt.name = "logical_stmt";
                        token_list.push_front(stmt);
                    }
                    else if(production == "logical_stmt -> ( logical_stmt )") {
                        Token ls1 = front_and_pop_n(3, token_list)[1];
                        ls1.name = "logical_stmt";
                        token_list.push_front(ls1);
                    }
                    else if(production == "logical_stmt -> logical_stmt lop M logical_stmt") {
                        Token ls2 = front_and_pop(token_list);
                        Token m = front_and_pop(token_list);
                        Token lop = front_and_pop(token_list);
                        Token ls1 = front_and_pop(token_list);

                        Token ls;
                        if(lop.name == "||") {
                            back_patch(ls1.false_list, m.instr);
                            ls.true_list = merge(ls1.true_list, ls2.true_list);
                            ls.false_list = ls2.false_list;
                        }
                        else {
                            back_patch(ls1.true_list, m.instr);
                            ls.false_list = merge(ls1.false_list, ls2.false_list);
                            ls.true_list = ls2.true_list;
                        }

                        ls.name = "logical_stmt";
                        token_list.push_front(ls);
                    }
                    else if(production == "if_stmt -> if ( logical_stmt ) M block N else M block") {
                        std::vector<Token> tokens = front_and_pop_n(10, token_list);
                        back_patch(tokens[2].true_list, tokens[4].instr);
                        back_patch(tokens[2].false_list, tokens[8].instr);
                        back_patch(tokens[6].next_list, new_list_no(false));
                        auto temp = merge(tokens[5].next_list, tokens[6].next_list);

                        Token ls;
                        ls.next_list = merge(temp, tokens.back().next_list);

                        for(auto& p : ls.next_list) {
                            std::cout << p.first << ": " << p.second << std::endl;
                        }

                        ls.name = "if_stmt";
                        token_list.push_front(ls);
                    }
                    else if(production == "block -> { stmts }") {
                        Token s = front_and_pop_n(3, token_list)[1];
                        s.name = "block";
                        token_list.push_front(s);
                    }
                    else if(production == "stmts -> stmts stmt") {
                        Token s = front_and_pop_n(2, token_list)[0];
                        s.name = "stmts";
                        token_list.push_front(s);
                    }
                    else if(production == "while_stmt -> while M ( logical_stmt ) M block") {
                        std::vector<Token> tokens = front_and_pop_n(7, token_list);
                        back_patch(tokens.back().next_list, tokens[1].instr);
                        back_patch(tokens[3].true_list, tokens[5].instr);

                        Token ws;
                        ws.next_list = tokens[3].false_list;

                        std::size_t n = new_list_no();
                        std::string s("goto " + std::to_string(tokens[1].instr));
                        append_to_conv_result(n, s);

                        back_patch(ws.next_list, new_list_no(false));

                        ws.name = "while_stmt";
                        token_list.push_front(ws);
                    }
                    std::printf("%s\n%s\n\n", production.data(), "reduction");
                }
                if(tokens_.empty()) {
                    tokens_.emplace_back(empty_token);
                }
            }

            for(auto& p : conv_results_) {
                std::cout << p.first << ": " << p.second << std::endl;
            }
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
        template <typename T>
        T binary_operation(std::string& op, std::string& s1, std::string& s2) {
            T t1, t2;
            if constexpr (std::is_same_v<T, int>) {
                t1 = std::atoi(s1.data());
                t2 = std::atoi(s2.data());
            }
            else if constexpr (std::is_same_v<T, double>) {
                t1 = std::atof(s1.data());
                t2 = std::atof(s2.data());
            }
            if(op == "+") {
                return t1 + t2;
            }
            else if(op == "-") {
                return t1 - t2;
            }
            else if(op == "*") {
                return t1 * t2;
            }
            else if(op == "/") {
                return t1 / t2;
            }
            else {
                return T{};
            }
        }
        void back_patch(std::map<std::size_t, std::string>& l, std::size_t n) {
            for(auto& [m, str] : l) {
                str.append(" " + std::to_string(n));

                auto pos = str.find_first_of(':');
                auto num_str = str.substr(0, pos);
                auto nn = std::atoi(num_str.data());
                str = str.substr(pos + 1);
                append_to_conv_result(nn, str);
            }
        }
        std::map<std::size_t, std::string> merge(std::map<std::size_t, std::string>& l1, std::map<std::size_t, std::string>& l2) {
            std::map<std::size_t, std::string> l = l1;
            for(auto& p : l2) {
                l[p.first] = p.second;
            }
            return l;
        }
        Token front_and_pop(std::list<Token>& token_list) {
            Token t = token_list.front();
            token_list.pop_front();
            return t;
        }
        std::vector<Token> front_and_pop_n(std::size_t n, std::list<Token>& token_list) {
            std::vector<Token> tokens(n);
            for(std::size_t i = 0; i != n; ++i) {
                tokens[n - i - 1] = front_and_pop(token_list);
            }
            return tokens;
        }
        std::string new_tmp_name() {
            static std::size_t n = 0;
            ++n;
            return "t" + std::to_string(n);
        }
        std::size_t new_list_no(bool add = true) {
            static std::size_t n = 100;
            if(add) {
                return n++;
            }
            else {
                return n;
            }
        }
        void anaysis_token(const std::string& filepath) {
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
                    if(auto code = query_keyword(token); code != ID::UNKNOWN) {
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        if(!tokens_.empty() && tokens_.back().name == "def") {
                            symbol_table_.emplace_back("def", token);
                            tokens_.emplace_back(symbol_table_.size() - 1, "def", token);
                        }
                        else {
                            std::size_t i { 0 };
                            for(; i != symbol_table_.size(); ++i) {
                                /* std::cout << symbol_table_[i].name << " " << token << std::endl; */
                                if(symbol_table_[i].name == token) {
                                    break;
                                }
                            }
                            if(i != symbol_table_.size()) {
                                tokens_.emplace_back(i, "ident", token);
                            }
                            else {
                                symbol_table_.emplace_back("ident", token);
                                tokens_.emplace_back(symbol_table_.size() - 1, "ident", token);
                            }
                        }
                    }
                    token.clear();
                }
                else if(is_digit(ch)) {
                    token.append(1, ch);
                    ch = fin.get();
                    bool is_real = false;
                    while(is_digit(ch) || ch == '.' || ch == 'e') {
                        if(ch == '.') {
                            is_real = true;
                        }
                        token.append(1, ch);
                        ch = fin.get();
                    }
                    fin.unget();
                    if(is_real) {
                        tokens_.emplace_back(0, "REAL", token);
                    }
                    else {
                        tokens_.emplace_back(0, "INT", token);
                    }
                    token.clear();
                }
                else if(ch == '\"') {
                    while((ch = fin.get()) != '\"') {
                        token.append(1, ch);
                    }
                    tokens_.emplace_back(0, "STRING", token);
                    token.clear();
                }
                else if(ch == '\'') {
                    token.append(1, ch);
                    while((ch = fin.get()) != '\'') {
                        token.append(1, ch);
                    }
                    token.append(1, ch);
                    tokens_.emplace_back(0, "CHAR", token);
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
                    }
                    else {
                        while(is_operator(ch)) {
                            token.append(1, ch);
                            ch = fin.get();
                        }
                        fin.unget();
                        tokens_.emplace_back(0, token, token);
                    }
                    token.clear();
                }
                else if(is_delimiter(ch)) {
                    token.append(1, ch);
                    if(auto id = query_delimiter(token); id != -1) {
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        std::cerr << "unknown delimiter " << ch << " " << token << std::endl;
                        exit(0);
                    }
                    token.clear();
                }
                else if(ch == '<') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        fin.unget();
                        tokens_.emplace_back(0, token, token);
                    }
                    token.clear();
                }
                else if(ch == '>') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        fin.unget();
                        tokens_.emplace_back(0, token, token);
                    }
                    token.clear();
                }
                else if(ch == '=') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        fin.unget();
                        tokens_.emplace_back(0, token, token);
                    }
                    token.clear();
                }
                else if(ch == '!') {
                    token.append(1, ch);
                    if(ch = fin.get(); ch == '=') {
                        token.append(1, ch);
                        tokens_.emplace_back(0, token, token);
                    }
                    else {
                        fin.unget();
                        tokens_.emplace_back(0, token, token);
                    }
                    token.clear();
                }
                else if(ch == '\n') {
                    ++row;
                }
            }
            for(auto& symbol : symbol_table_) {
                std::cout << symbol.name << " " << symbol.type << " " << symbol.value << std::endl;
            }
            /* for(auto& token : tokens_) { */
            /*     std::cout << token.code << std::endl; */
            /* } */
        }

        /* 读取文法，解析出
         * code1 : { node1, node2, ... } -> { node1, node2, ... }
         * code2 : { node1, node2, ... } -> { node1, node2, ... }
         * 二维链表形式 */
        /* 记录当前已经找到First集的code */
        void read_grammer(const std::string& filepath) {
            std::ifstream fin{ filepath, std::ios_base::in };
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
                            if(code_list.front() != code) {
                                done = false;
                                break;
                            }
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
                            else if(it->front() != code && done_set.count(it->front())) {
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
            /* std::cout << "first set..." << std::endl; */
            /* for(auto& [code, code_set]: first_set_) { */
            /*     std::cout << code << "\n"; */
            /*     std::copy(code_set.begin(), code_set.end(), std::ostream_iterator<std::string>{std::cout, " "}); */
            /*     std::cout << std::endl; */
            /* } */

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
            /* std::cout << "follow set..." << std::endl; */
            /* for(auto& [code, code_list] : follow_set_) { */
            /*     code_list.sort(); */
            /*     code_list.unique(); */
            /*     std::cout << code << "\n"; */
            /*     std::copy(code_list.begin(), code_list.end(), std::ostream_iterator<std::string>{std::cout, " "}); */
            /*     std::cout << std::endl; */
            /* } */
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
            std::unordered_set<std::string> fs;
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
                        fs.emplace(s);
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
        void check_first_set() {
            /* std::cout << ".............................................." << std::endl; */
            /* for(auto& [code, code_list] : first_set_) { */
            /*     std::cout << code << std::endl; */
            /*     for(auto& c : code_list) { */
            /*         std::cout << c << " "; */
            /*     } */
            /*     std::cout << std::endl; */
            /* } */
            /* std::ifstream fin{ "first_set", std::ios_base::in }; */
            /* while(!fin.eof()) { */
            /*     std::string head; */
            /*     std::list<std::string> tails; */
            /*     std::getline(fin, head); */
            /*     auto pos = head.find_last_not_of(' '); */
            /*     if(pos != std::string::npos) { */
            /*         head = head.substr(0, pos + 1); */
            /*     } */
            /*     std::string line; */
            /*     std::getline(fin, line); */
            /*     std::stringstream oss(line); */
            /*     std::cout << head << std::endl; */
            /*     std::cout << line << std::endl; */
            /*     for(auto& tail : first_set_[head]) { */
            /*         std::cout << tail << " "; */
            /*     } */
            /*     std::cout << std::endl; */
            /* } */
            /* fin.close(); */
            /* std::cout << ".............................................." << std::endl; */
        }
        void anaysis_lr() {
            anaysis_items();
            anaysis_action();
        }
        void anaysis_closure(std::vector<Item>& item_set) {
            while(true) {
                bool done = true;
                for(std::size_t i = 0; i != item_set.size(); ++i) {
                    if(item_set[i].dot >= item_set[i].prod_vec.size()) {
                        continue;
                    }
                    std::string s;
                    for(std::size_t j = item_set[i].dot + 1; j < item_set[i].prod_vec.size(); ++j) {
                        s.append(item_set[i].prod_vec[j]);
                        s.append(1, ' ');
                    }
                    s.append(item_set[i].suffix_code);
                    auto fs = query_first_set(s);
                    if(fs.empty()) {
                        continue;
                    }
                    if(!grammer_.count(item_set[i].prod_vec[item_set[i].dot])) {
                        continue;
                    }
                    for(auto& code_list : grammer_[item_set[i].prod_vec[item_set[i].dot]]) {
                        for(auto& symbol : fs) {
                            if(grammer_.count(symbol)) {
                                continue;
                            }
                            if(symbol.empty()) {
                                continue;
                            }
                            Item new_item(0, item_set[i].prod_vec[item_set[i].dot], symbol);
                            new_item.prod_vec.reserve(code_list.size());
                            new_item.prod_vec.insert(new_item.prod_vec.end(), code_list.begin(), code_list.end());
                            if(new_item.prod_vec[0] == "@") {
                                ++new_item.dot;
                            }
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
            check_first_set();

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

            std::cout << "items start............." << std::endl;
            std::vector<Item> item_set;
            item_set.emplace_back( 0, start_code_ + "0", "$", std::vector{ start_code_ }) ;
            anaysis_closure(item_set);
            closure_.emplace_back(std::move(item_set));

            for(std::size_t i = 0; i != closure_.size(); ++i) {
                for(auto& symbol : symbols) {
                    if(symbol == "@") {
                        continue;
                    }
                    anaysis_goto(i, symbol);
                }
            }
            /* std::cout << "items done..............." << std::endl; */

            /* std::size_t id = 0; */
            /* for(auto& items : closure_) { */
            /*     std::cout << id++ << std::endl; */
            /*     for(auto& item : items) { */
            /*         std::cout << item.prefix_code << " -> "; */
            /*         for(std::size_t i = 0; i != item.dot; ++i) { */
            /*             std::cout << item.prod_vec[i] << " "; */
            /*         } */
            /*         std::cout << "."; */
            /*         for(std::size_t i = item.dot; i != item.prod_vec.size(); ++i) { */
            /*             std::cout << " " << item.prod_vec[i]; */
            /*         } */
            /*         std::cout << " , " << item.suffix_code << std::endl; */
            /*     } */
            /*     std::cout << std::endl; */
            /* } */
        }
        void anaysis_action() {
            for(std::size_t i = 0; i != closure_.size(); ++i) {
                for(std::size_t j = 0; j != closure_[i].size(); ++j) {
                /* for(auto& proj : closure_[i]) { */
                    auto& proj = closure_[i][j];
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
                        if(pv.size() == 1 && pv.back() == "@") {
                            action_[i][suffix] = std::make_pair(prefix, std::make_pair(std::size_t{ 0 }, pv));
                            /* action_[i][suffix] = std::make_pair(prefix, std::size_t{ 0 }); */
                        }
                        else {
                            action_[i][suffix] = std::make_pair(prefix, std::make_pair(std::size_t{ dot }, pv));
                            /* action_[i][suffix] = std::make_pair(prefix, dot); */
                        }
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
        void append_to_conv_result(std::size_t n, std::string s) {
            conv_results_[n] = s;
        }
    private:
        std::map<std::size_t, std::string> conv_results_;
        std::string start_code_;
        /* std::stringstream tokens_; */
        /* std::list<std::string> tokens_; */
        std::list<Token> tokens_;
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

        static std::unordered_set<std::string> keywords;
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::keyword_table = {
        { "if", IF }, { "else", ELSE },
        { "for", FOR }, { "while", WHILE}, { "break", BREAK }, { "continue", CONTINUE },
        { "return", RETURN }, { "include", INCLUDE },
        { "char", CHAR }, { "int", INT }, { "float", FLOAT }, { "double", DOUBLE },
        { "string", STRING }, { "bool", BOOL }, { "void", VOID }, { "do", DO },
        { "switch", SWITCH }, { "case", CASE }, { "default", DEFAULT },
        { "def", DEF }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::operator_table = {
        { "+", ADD }, { "-", DEC }, { "*", MUL }, { "/", DIV }, { "=", ASSIGN },
        { "++", ADD_ADD }, { "--", DEC_DEC }, { "+=", ADD_ASSIGN }, { "-=", DEC_ASSIGN },
        { "<", LESS }, { ">", MORE }, { "==", EQUAL }, { "!=", NO_EQUAL },
        { "<=", LESS_EQUAL }, { ">=", MORE_EQUAL }, { "&&", AND }, { "||", OR },
        { "&", LOG_AND }, { "|", LOG_OR }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::delim_table = {
        { "[", LEFT_SQUARE }, { "]", RIGHT_SQUARE },
        { "(", LEFT_ROUND }, { ")", RIGHT_ROUND },
        { "{", LEFT_CURLY }, { "}", RIGHT_CURLY },
        { ",", COMMA }, { ";", SEMICOLON }, { ":", MAOHAO }
    };

    std::unordered_map<std::string_view, ID> LexicalAnaysis::type_table = {
    };
}
