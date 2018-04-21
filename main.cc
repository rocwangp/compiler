#include "lexical_anaysis.hpp"
int main()
{
    std::ifstream fin{ "test", std::ios_base::in };
    std::list<std::string> operation{ std::istream_iterator<std::string>{fin}, std::istream_iterator<std::string>{} };
    operation.emplace_back("$");
    complier::LexicalAnaysis la;
    la.anaysis();
    la.anaysis_operation(std::move(operation));
    /* std::cout << std::endl; */
    /* la.anaysis_predict({"id", "+", "id", "*", "id", "$"}); */
    return 0;
}
