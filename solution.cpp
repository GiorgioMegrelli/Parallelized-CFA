#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <gmpxx.h>

using namespace std;


#define DO_OUTPUT false


struct ParsedArgs {
    string modules_file;
    string tmp_dir_name;
    string result_file;
    int number_system;
    ParsedArgs() {
        // Default Program Arguments
        this->modules_file = "moduli.txt";
        this->tmp_dir_name = "tmp/";
        this->number_system = 16;
        this->result_file = "result.txt";
    }
};


void parse_cmd_args(int, char **, ParsedArgs &);

void read_modules(ParsedArgs &, vector<mpz_class> &);
void read_numbers(ParsedArgs &, string, vector<mpz_class> &);
void write_numbers(ParsedArgs &, string, vector<mpz_class> &);

void println(string);
void build_product_tree(ParsedArgs &, vector<mpz_class> &, vector<string> &);
void build_remainder_tree(ParsedArgs &, vector<string> &, vector<string> &);

void find_bad_modules(ParsedArgs &, vector<mpz_class> &, map<mpz_class, mpz_class> &);
void show_results(ParsedArgs &, map<mpz_class, mpz_class> &);


/*
Usage:
    --modules-file=<path to modules file>
    --tmp-dir=<name of sub directory>
    --num-system=<value of number of system>
    --result-file=<path to result file>
*/
int
main(int argn, char ** argv)
{
    ParsedArgs parsed_args;
    vector<mpz_class> modulus;
    map<mpz_class, mpz_class> result;

    parse_cmd_args(argn, argv, parsed_args);

    read_modules(parsed_args, modulus);
    find_bad_modules(parsed_args, modulus, result);
    show_results(parsed_args, result);
    return 0;
}


void
find_bad_modules(ParsedArgs & pargs, vector<mpz_class> & modulus, map<mpz_class, mpz_class> & result)
{
    vector<string> product_files, remainder_files;
    vector<mpz_class> remainders;

    build_product_tree(pargs, modulus, product_files);
    build_remainder_tree(pargs, product_files, remainder_files);

    read_numbers(pargs, remainder_files[0], remainders);

    mpz_class one("1", pargs.number_system);
    for(int i = 0; i < modulus.size(); i++) {
        mpz_class r = gcd(remainders[i] / modulus[i], modulus[i]);
        if(cmp(r, one) != 0) {
            result[modulus[i]] = r;
        }
    }
}


void
show_results(ParsedArgs & pargs, map<mpz_class, mpz_class> & result)
{
    ofstream result_file(pargs.result_file);

    result_file << "Modulus Found: (=" << result.size() << ")" << endl;

    map<mpz_class, mpz_class>::iterator it;
    for(it = result.begin(); it != result.end(); it++) {
        mpz_class third = it->first / it->second;
        result_file
        << it->first.get_str(pargs.number_system) << " =" << endl
        << it->second.get_str(pargs.number_system) << " * "
        << third.get_str(pargs.number_system) << endl;
    }
}


void
file_create_msg(string file_name)
{
    println("File '" + file_name + "' created!");
}


string
build_file_name(ParsedArgs & pargs, string type, int index)
{
    string file_name;
    file_name += pargs.tmp_dir_name;
    file_name += type;
    file_name += "_tmp_";
    file_name += to_string(index);
    file_name += ".txt";
    return file_name;
}


void
build_product_tree(ParsedArgs & pargs, vector<mpz_class> & modulus, vector<string> & product_files)
{
    println("Building Product Tree...");

    vector<mpz_class> cloned_modulus(modulus.begin(), modulus.end());

    int index = 0;
    int size = cloned_modulus.size();

    while(true) {
        string file_name = build_file_name(pargs, "product", index);

        cloned_modulus.erase(cloned_modulus.begin() + size, cloned_modulus.end());

        write_numbers(pargs, file_name, cloned_modulus);
        file_create_msg(file_name);

        product_files.push_back(file_name);

        index++;

        if(size == 1) {
            break;
        }

        int i, j;
        for(i = 0, j = 0; i < size; i += 2, j++) {
            if(i == size - 1) {
                cloned_modulus[j] = cloned_modulus[i];
            } else {
                cloned_modulus[j] = cloned_modulus[i] * cloned_modulus[i + 1];
            }
        }
        size = j;
    }

    println("Product Tree Built!\n");
}


void
build_remainder_tree(ParsedArgs & pargs, vector<string> & product_files, vector<string> & result)
{
    println("Building Remainder Tree...");

    int file_index = product_files.size() - 1;

    vector<mpz_class> parent_node, child_node, file_output;
    read_numbers(pargs, product_files[file_index], parent_node);

    while(file_index > 0) {
        read_numbers(pargs, product_files[--file_index], child_node);

        for(int i = 0; i < parent_node.size(); i++) {
            file_output.push_back(parent_node[i] % (child_node[2*i] * child_node[2*i]));
            if(2 * i < child_node.size() - 1) {
                file_output.push_back(parent_node[i] % (child_node[2*i + 1] * child_node[2*i + 1]));
            }
        }

        string file_name = build_file_name(pargs, "remainder", file_index);

        write_numbers(pargs, file_name, file_output);
        file_create_msg(file_name);

        result.insert(result.begin(), file_name);

        parent_node.clear();
        parent_node.insert(parent_node.end(), file_output.begin(), file_output.end());

        child_node.clear();
        file_output.clear();
    }

    println("Remainder Tree Built\n");
}


void
trim_string(string & to_trim)
{
    int start_i = 0;
    int end_i = (int) to_trim.size() - 1;

    while(start_i < to_trim.size() && isspace(to_trim[start_i])) {
        start_i++;
    }

    while(end_i >= start_i && isspace(to_trim[end_i])) {
        end_i--;
    }

    to_trim.erase(to_trim.begin() + end_i + 1, to_trim.end());
    to_trim.erase(to_trim.begin(), to_trim.begin() + start_i);
}


void
read_modules(ParsedArgs & pargs, vector<mpz_class> & modulus)
{
    ifstream modulus_file(pargs.modules_file);

    string line;
    while(true) {
        getline(modulus_file, line);
        trim_string(line);
        if(line.empty()) {
            break;
        }
        modulus.push_back(mpz_class(line, pargs.number_system));
    }

    modulus_file.close();
}


void
read_numbers(ParsedArgs & pargs, string file_name, vector<mpz_class> & modulus)
{
    ifstream modulus_file(file_name);

    string line;
    getline(modulus_file, line);

    int size = stoi(line);
    modulus.reserve(modulus.size() + size);

    for(int i = 0; i < size; i++) {
        getline(modulus_file, line);
        modulus.push_back(mpz_class(line, pargs.number_system));
    }

    modulus_file.close();
}


void
write_numbers(ParsedArgs & pargs, string file_name, vector<mpz_class> & modulus)
{
    ofstream modulus_file(file_name);

    modulus_file << modulus.size();

    for(int i = 0; i < modulus.size(); i++) {
        modulus_file << "\n" << modulus[i].get_str(pargs.number_system);
    }

    modulus_file.close();
}


void
println(string s)
{
    if(DO_OUTPUT) {
        cout << s << endl;
    }
}


bool
cmp_ignore_case(string str1, string str2)
{
    if(str1.size() != str2.size()) {
        return false;
    }
    for(size_t i = 0; i < str1.size(); i++) {
        if(toupper(str1[i]) != toupper(str2[i])) {
            return false;
        }
    }
    return true;
}


void
parse_cmd_args(int argn, char ** argv, ParsedArgs & pargs)
{
    const char equal_sign = '=';
    vector<string> args;
    for(int i = 1; i < argn; i++) {
        string str(argv[i]);
        size_t eq_index = str.find(equal_sign);

        if(eq_index == string::npos) {
            if(i + 1 != argn) {
                args.push_back(str);
                args.push_back(string(argv[++i]));
            }
        } else {
            args.push_back(string(str.begin(), str.begin() + eq_index));
            args.push_back(string(str.begin() + eq_index + 1, str.end()));
        }
    }

    for(size_t i = 0; i < args.size(); i += 2) {
        if(cmp_ignore_case(args[i], "--modules-file")) {
            pargs.modules_file = args[i + 1];
        } else if(cmp_ignore_case(args[i], "--tmp-dir")) {
            pargs.tmp_dir_name = args[i + 1];
        } else if(cmp_ignore_case(args[i], "--result-file")) {
            pargs.result_file = args[i + 1];
        } else if(cmp_ignore_case(args[i], "--num-system")) {
            pargs.number_system = stoi(args[i + 1]);
        }
    }
}
