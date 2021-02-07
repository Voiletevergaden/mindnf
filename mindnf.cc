// mindnf.cc written by Kazuki Maeda <kmaeda at kmaeda.net>, 2021

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <bitset>
#include <algorithm>
#include <memory>
#include <getopt.h>
#include <chrono>

using namespace std;

const int maxnuminputs = 64;        // The max number of input variables
const int dmaxnuminputs = maxnuminputs*2;
const int maxnumprimeimpl =  10000; // The max number of prime implicants
                                    // If there are more input variables, increase this value
                                    // and recompile.

vector<bitset<dmaxnuminputs>> minterms;
vector<bitset<dmaxnuminputs>> nonminterms;

vector<string> inames;

vector<bitset<dmaxnuminputs>> primeimpl;
vector<vector<int>> essprimeimpl;

size_t numinputs, dnuminputs, numminterm, minnumessprimeimpl;

template <size_t N>
bool bitset_d_decrement(bitset<N>& in){
  for(size_t i = 0; i < dnuminputs; i += 2){
    if(in[i+1] == 0){           // in[i] == 1
      in[i] = 1;
      in[i+1] = 1;
    } else if(in[i] == 0){      // in[i+1] == 1
      in[i] = 1;
      in[i+1] = 0;
      break;
    } else {                    // in[i] == 1 && in[i+1] == 1
      in[i] = 0;
      break;
    }
  }
  if(in.count() == dnuminputs)
    return false;
  return true;
}

string impl2str(const bitset<dmaxnuminputs>& impl) {
  string str = "";
  for(size_t i = 0; i < dnuminputs; i += 2){
    if(!(impl[i] == 1 && impl[i+1] == 1)){
      if(str != "")
        str += " & ";
      if(impl[i] == 1 && impl[i+1] == 0)
        str += "^";
      str += inames[i/2];
    }
  }
  return str;
}

void findprimeimpl(){
  // For fast computation, we should use bitset to represent implicants.
  // Let x and bs be an implicant and the bitset corresponding to x:
  // if x[n] ==  0, then bs[2*n] = 1 and bs[2*n+1] = 0;
  //    x[n] ==  1, then bs[2*n] = 0 and bs[2*n+1] = 1;
  //    x[n] == -1, then bs[2*n] = 1 and bs[2*n+1] = 1.

  // primeimpl[impl index][var index used above]
  primeimpl = vector<bitset<dmaxnuminputs>>();
  bitset<dmaxnuminputs> t;
  for(size_t i = 0; i < dnuminputs; ++i)
    t[i] = 1;
  do {
    bool flag = true;
    // remove if it is false for all minterms
    for(auto it = minterms.cbegin(); it != minterms.cend(); ++it)
      if((t & *it) == *it){
        flag = false;
        break;
      }
    if(!flag) {
      flag = false;
      // remove if it is true for (some) nonminterms
      for(auto it = nonminterms.cbegin(); it != nonminterms.cend(); ++it)
        if((t & *it) == *it){
          flag = true;
          break;
        }
      if(!flag){
        flag = false;
        // remove if it is already included in another implicant
        for(auto it = primeimpl.cbegin(); it != primeimpl.cend(); ++it)
          if((t & *it) == t) {
            flag = true;
            break;
          }
        if(!flag)
          primeimpl.push_back(t);
      }
    }
  } while(bitset_d_decrement(t));
}

// solve minimal cover problem
void search(vector<int>& cover,
            unsigned int mintermid,
            bitset<maxnumprimeimpl>& searched) {
  bitset<maxnumprimeimpl> nextsearched = searched;
  while(mintermid < numminterm){
    bool flag = true;
    for(auto it = cover.cbegin(); it != cover.cend(); ++it)
      if((minterms[mintermid] & primeimpl[*it]) == minterms[mintermid]){
        flag = false;
        break;
      }

    if(flag){
      if(cover.size() == minnumessprimeimpl)
        return;
      for(size_t i = 0; i < primeimpl.size(); ++i)
        if(!nextsearched[i] && (minterms[mintermid] & primeimpl[i]) == minterms[mintermid]){
          cover.push_back(i);
          nextsearched.set(i);   // prevent duplicate listing
          search(cover, mintermid+1, nextsearched);
          cover.pop_back();
        }
      return;
    } else
      ++mintermid;
  }
  if(cover.size() < minnumessprimeimpl){
    minnumessprimeimpl = cover.size();
    essprimeimpl.clear();
  }
  essprimeimpl.push_back(cover);
}

int main(int argc, char** argv){
  int opt, longindex;
  bool printhelp = false;
  bool printprimeimplicants = false;
  bool printtime = false;
  bool nosearch = false;

  struct option long_options[] =
    {
     {"help", no_argument, NULL, 'h'},
     {"no-search", no_argument, NULL, 'n'},
     {"print-prime-implicants", no_argument, NULL, 'p'},
     {"time", no_argument, NULL, 't'},
     {0, 0, 0, 0}
    };

  while((opt = getopt_long(argc, argv, "hnpt", long_options, &longindex)) != -1){
    switch(opt){
    case 'h':
      printhelp = true;
      break;
    case 'n':
      nosearch = true;
      break;
    case 'p':
      printprimeimplicants = true;
      break;
    case 't':
      printtime = true;
      break;
    }
  }

  if(printhelp || argc - optind < 1){
    cerr << "Usage: " << argv[0] << " [options] <truth table file>" << endl;
    cerr << "Options:" << endl;
    cerr << "  -n or --no-search             Don't search essential prime implicants (use with --print-prime-implicants)." << endl;
    cerr << "  -p, --print-prime-implicants  Print all prime implicants." << endl;
    cerr << "  -t, --time                    Print timing data about this program run." << endl;
    cerr << "  -h, --help                    Print this message and exit." << endl;
    cerr << endl;
    cerr << "Note: the output value for the input values not included in the truth table file is" << endl;
    cerr << "      regarded as \"don't care\"." << endl;
    return 1;
  }

  // read
  ifstream fin;
  fin.exceptions(ios::failbit);
  try{
    fin.open(argv[optind]);
  } catch (exception& e){
    cerr << argv[0] << ": " << argv[optind] << ": " << strerror(errno) << endl;
    return 1;
  }

  int lineno = 0;
  string oname = "";
  while(!fin.eof()){            // read
    try{
      string tmp;
      getline(fin, tmp);
      ++lineno;
      if(tmp[0] != '#'){
        if(oname == ""){        // header
          stringstream ss(tmp);
          while(!ss.eof()){
            string tmp2;
            ss >> tmp2;
            inames.push_back(tmp2);
          }
          oname = inames[inames.size()-1];
          inames.erase(inames.begin() + inames.size()-1);
          numinputs = inames.size();
          if(numinputs > maxnuminputs){
            cerr << "Error: the number of input variables is too large!" << endl;
            cerr << "       Increase the value of maxnuminputs and recompile the program." << endl;
            return 1;
          }
        } else {                // data
          stringstream ss(tmp);
          bitset<dmaxnuminputs> term;
          string tmp2;
          bool ignoreflag = false;
          for(size_t i = 0; i < numinputs; ++i){ // input
            ss >> tmp2;
            if(tmp2 != "0" && tmp2 != "1"){
              cerr << argv[optind] << ":" << lineno << ": warning: illegal input line \"" << tmp << "\", ignored." << endl;
              ignoreflag = true;
              break;
            }
            if(tmp2 == "0")
              term[2*i] = 1;
            else    // "1"
              term[2*i+1] = 1;
          }
          if(ignoreflag)
            continue;

          ss >> tmp2;           // output
          if(tmp2 != "0" && tmp2 != "1"){
            cerr << argv[optind] << ":" << lineno << ": warning: illegal input line \"" << tmp << "\", ignored." << endl;
            continue;
          }
          if(tmp2 == "1")
            minterms.push_back(term);
          else
            nonminterms.push_back(term);
        }
      }
    } catch (exception& e){
      break;
    }
  }

  dnuminputs = numinputs*2;
  numminterm = minterms.size();


  // find prime implicants
  auto st = chrono::system_clock::now();
  findprimeimpl();
  auto et = chrono::system_clock::now();

  const int numprimeimpl = primeimpl.size();
  minnumessprimeimpl = numprimeimpl;
  if(numprimeimpl > maxnumprimeimpl){
    cerr << "Error: the number of prime implicants is too large!" << endl;
    cerr << "       Increase the value of maxnumprimeimpl and recompile the program." << endl;
    return 1;
  }

  sort(primeimpl.begin(), primeimpl.end(),
       [](const bitset<dmaxnuminputs>& x, const bitset<dmaxnuminputs>& y) {
         return x.count() > y.count();
       });

  if(printprimeimplicants){
    cout << "Prime implicants: " << endl;
    int n = 1;
    int w = to_string(numprimeimpl).length();
    for(auto it = primeimpl.cbegin(); it != primeimpl.cend(); ++it){
      cout << setw(w) << n << ": ";
      cout << impl2str(*it) << endl;
      ++n;
    }
    cout << endl;
  }

  if(printtime)
    cerr << "Time for constructing prime implicants table: "
         << static_cast<double>(chrono::duration_cast<chrono::microseconds>(et-st).count())/1000000
         << "s"
         << endl;

  if(!nosearch){
    vector<int> cover;
    bitset<maxnumprimeimpl> searched;
    st = chrono::system_clock::now();
    search(cover, 0, searched);
    et = chrono::system_clock::now();

    cout << "Results:" << endl;
    for(auto it = essprimeimpl.cbegin(); it != essprimeimpl.cend(); ++it){
      cout << oname << " = ";
      bool firstflag = true;
      for(auto it2 = it->cbegin(); it2 != it->cend(); ++it2){
        if(!firstflag)
          cout << " | ";
        cout << "(" << impl2str(primeimpl[*it2]) << ")";
        firstflag = false;
      }
      cout << endl;
    }

    if(printtime)
      cerr << "Time for solving the minimal cover problem: "
           << static_cast<double>(chrono::duration_cast<chrono::microseconds>(et-st).count())/1000000
           << "s"
           << endl;
  }

  return 0;
}
