#include "bench_framework.hpp"
#include <mapnik/unicode.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_grammar.hpp>

class test : public benchmark::test_case
{
    std::string expr_;
public:
    test(mapnik::parameters const& params)
     : test_case(params),
       expr_("((([mapnik::geometry_type]=2) and ([oneway]=1)) and ([class]='path'))") {}
    bool validate() const
    {
        mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
        std::string result = mapnik::to_expression_string(*expr);
        bool ret = (result == expr_);
        if (!ret)
        {
            std::clog << result  << " != " << expr_ << "\n";
        }
        return ret;
    }
    void operator()() const
    {
         for (std::size_t i=0;i<iterations_;++i) {
             mapnik::expression_ptr expr = mapnik::parse_expression(expr_,"utf-8");
         }
    }
};


int main(int argc, char** argv)
{
    mapnik::parameters params;
    benchmark::handle_args(argc,argv,params);
    test test_runner(params);
    run(test_runner,"expr parsing");
}
