# Mindnf

Mindnf finds minimal [disjunctive normal forms](https://en.wikipedia.org/wiki/Disjunctive_normal_form) (DNF) for a given incomplete truth table. For this purpose, [Quine–McCluskey algorithm](https://en.wikipedia.org/wiki/Quine%E2%80%93McCluskey_algorithm) is a well-known method. However, this algorithm is so slow for the problems with many variables and *a large number of "don't care" output values*. Mindnf tries to find minimal DNFs fast by another simple method:

1. Let (x1[n], x2[n], ..., xm[n]) -> out[n] be the nth row of a given truth table. Consider all possible implicants. For each implicant f,
   1. If f(x1[n], x2[n], ..., xm[n]) = 0 for all n with out[n] = 1, then break.
   1. If f(x1[n], x2[n], ..., xm[n]) = 1 for some n with out[n] = 0, then break.
   1. If the implicant f passes the two tests above, then add f to the list of prime implicants.
1. Find minimal DNFs (essential prime implicants) by solving the set cover problem of prime implicants.

## Compile

The program is written in C++. You must prepare a C++ compiler implemented C++11 features.
Type

    $ g++ -O2 -march=native -o mindnf mindnf.cc

Then the binary `mindnf` will be generated.

## Usage

    $ ./mindnf [options] <truth table file>

Then the program find minimal DNF compatible with the truth table in &lt;truth table file&gt;.

For example, the file [testtruthtable.txt](https://github.com/kmaed/mindnf/blob/master/testtruthtable.txt) is an incomplete truth table representing X = F(A, B, C):

|A|B|C|X|
|-|-|-|-|
|1|1|1|1|
|1|1|0|?|
|1|0|1|0|
|0|1|1|0|
|0|1|0|0|
|0|0|1|0|
|1|0|0|1|
|0|0|0|0|

In the truth table file, (A, B, C, X) = (1, 1, 0, ?) simply lacks. `mindnf` "doesn't care" the output ? and finds DNFs with (i) the minimal number of conjunctions (prime implicants) and (ii) the minimal number of literals. In this case, we find a minimal DNF with two conjunctions and four literals:

    X = (A & B) | (A & ^C)

Other example data files are contained in [ciona](https://github.com/kmaed/mindnf/blob/master/ciona/) directory. These are experimental data of gene expression patterns in early embryos of ascidian ([*Ciona intestinalis* type A](https://en.wikipedia.org/wiki/Ciona_robusta)).

## Truth table data

For example, see the file [testtruthtable.txt](https://github.com/kmaed/mindnf/blob/master/testtruthtable.txt). The first line indicates the names of variables (several input variables and one output variable) and remaining lines indicate truth table. Since space characters are used as delimiters, the names of variables should not include space characters. If there is a # character at the begin of a line, the program ignores the line.

## Contact
Kazuki Maeda "kmaeda at kmaeda.net"
