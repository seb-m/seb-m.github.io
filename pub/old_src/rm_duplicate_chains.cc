/*
    (c) www.sisecurite.fr - 3/14/2008

    Sebastien Martini (sebastien.martini@sisecurite.fr)
    Nicolas Gereone (nicolas.gereone@sisecurite.fr)

    License: MIT License

    /!\ This code assume that input tables are already sorted (with rtsort).

    Example:
       $ g++ -Wall -O3 rm_duplicate_chains.cc -o rm_duplicate_chains -lboost_regex
       $ ./rm_duplicate_chains /src_path/ /dst_path/

    Dependancies:
       Boost Regex Library (package libboost-regex-dev on Ubuntu/Debian)
*/
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <list>
#include <set>
#include <map>
#include <iterator>
#include <string>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/regex.hpp>

using namespace std;

typedef unsigned long long uint64; // 64 bits uint


template<class T>
inline string to_string(const T& t) {
    stringstream ss;
    ss << t;
    return ss.str();
}

class TableWriter {
    public:
        TableWriter(const string dst_path, const string filename,
                    const unsigned int chunk_size):
        dst_path_(dst_path), filename_(filename),
        chunk_size_(chunk_size), bytes_count_(0), count_(0), previous_value_(0),
        filename_regex_("(.*?#.*?[xX])(\\d+)(.*?)(\\.rt)") {
            OpenTable();
        }

        ~TableWriter() {
            if (!(out_table_ == NULL))
                CloseTable();
        }

        void Write(const char* data, const unsigned int size) {
            out_table_->write(data, size);
            bytes_count_ += 16;
            if (bytes_count_ >= chunk_size_) {
                CloseTable();
                OpenTable();
            }
        }

    private:
        void OpenTable() {
            out_table_ = new ofstream((dst_path_ + "current.rt.temp").c_str(),
                                      ios::binary | ios::out);
            assert(out_table_ != NULL);
        }

        void CloseTable() {
            delete out_table_;
            out_table_ = NULL;

            const string nbchains = to_string<unsigned int>(bytes_count_ / 16);
            const string strcount = to_string<unsigned int>(count_);
            const string repl = "\\1" + nbchains + "\\3__" + strcount + "_\\4";

            string dst_filename = regex_replace(filename_, filename_regex_,repl,
                                    boost::match_default | boost::format_sed);

            cout << "Written file " << count_ << " compounded of "
                 << nbchains << " chains ("
                 << bytes_count_ << " bytes) -> "
                 << dst_filename << endl;

            count_++;
            bytes_count_ = 0;

            if (rename((dst_path_ + "current.rt.temp").c_str(),
                       (dst_path_ + dst_filename).c_str()))
                perror(NULL);
        }

        const string& dst_path_;
        const string& filename_;
        unsigned int chunk_size_;  // in bytes
        unsigned int bytes_count_;
        unsigned int count_;
        uint64 previous_value_;
        const boost::regex filename_regex_;
        ofstream* out_table_;

        // Disallow evil constructors
 	TableWriter(const TableWriter&);
 	TableWriter& operator=(const TableWriter&);
};

class Table {
    public:
        Table(const string src_filename, TableWriter& tw):
        src_filename_(src_filename), table_writer_(tw) {
            in_table_ = new ifstream(src_filename_.c_str(), ios::binary|ios::in);
            assert(in_table_ != NULL);
        }

        ~Table() {
            delete in_table_;
        }

        uint64 GetNextKey() {
            if (in_table_->eof())
                return -1;
            in_table_->read(current_chain_, 16);
            return *((uint64 *) (current_chain_ + 8));
        }

        const string Filename() const {
            return src_filename_;
        }

        void WriteCurrentChain() {
            table_writer_.Write(current_chain_, 16);
        }

        ostream& Print(ostream& os) const {
            os << Filename() << endl;
            return os;
        }

    private:
        const string& src_filename_;  // source filename
        TableWriter& table_writer_;  // handle write
        char current_chain_[16];  // hold the current chain
        ifstream* in_table_;  // input table

        // Disallow evil constructors
 	Table(const Table&);
 	Table& operator=(const Table&);
};

ostream& operator<< (ostream& os, const Table& table) {
    table.Print(os);
    return os;
}

void ListDirectory(const string src_path, set<string>& fset) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(src_path.c_str());
    if (dir == NULL)
        perror("opendir() error");

    const boost::regex ext_regex_(".*?\\.rt");
    while ((entry = readdir(dir)) != NULL) {
        string fn(entry->d_name);
        if (entry->d_type != DT_REG || fn == "." || fn == ".." ||
	    !regex_match(fn, ext_regex_))
	  continue;

        fset.insert(fn);
    }
    closedir(dir);
}

void InsertInMultiMapIfNotEOF(multimap<uint64, Table*>& mm, uint64 key,
                              Table* t) {
    if (key != (uint64) -1)
        mm.insert(pair<uint64, Table*>(key, t));
}

void Init(const string src_path, const set<string>& lf, TableWriter& tw,
          list<Table*>& lt, multimap<uint64, Table*>& mm) {
    Table *table = NULL;

    for (set<string>::const_iterator iter = lf.begin(); iter != lf.end();
         ++iter) {
        table = new Table(src_path + *iter, tw);
        lt.push_back(table);
        InsertInMultiMapIfNotEOF(mm, table->GetNextKey(), table);
    }
}

void PrintMultiMap(const multimap<uint64, Table*>& mm) {
    // Debug function
    for (multimap<uint64, Table*>::const_iterator iter = mm.begin();
         iter != mm.end(); ++iter) {
        cout << iter->first << " -> "
             << *iter->second
             << endl;
    }
}

void RemoveDup(multimap<uint64, Table*>& mm) {
    uint64 cur_min = 0;

    for (multimap<uint64, Table*>::iterator iter = mm.begin();
         iter != mm.end();) {
        cur_min = iter->first;
        (iter->second)->WriteCurrentChain();
        InsertInMultiMapIfNotEOF(mm, (iter->second)->GetNextKey(),
                                 iter->second);
        mm.erase(iter++);

        while(iter != mm.end()) {
            if (cur_min != iter->first)
                break;
            InsertInMultiMapIfNotEOF(mm, (iter->second)->GetNextKey(),
                                     iter->second);
            mm.erase(iter++);
        }
    }
}

int main(int argc, char *argv[]) {
    // Fixme options: ugly crap, use getopt instead
    if (argc < 3) {
	cerr << "usage: " << argv[0]
	     << " rainbow_tables_pathname destination_directory"
             << " [output chunk size in mb, 1024mb by default]"
	     << endl;
	exit (1);
    }

    for (int i = 1; i <= 2; ++i)
        if (argv[i][strlen(argv[i]) - 1] != '/') {
            cerr << "Path " << argv[i] << " must end with a '/'" << endl;
            exit(1);
        }
    string src_path = argv[1];
    string dst_path = argv[2];
    unsigned int chunk_size = 1024;
    if (argc == 4)
	chunk_size = atoi(argv[3]);
    chunk_size *= 1048576;
    assert(!(chunk_size % 16));

    // Initializes list of Table and multimap container
    set<string> fset;
    ListDirectory(src_path, fset);
    if (fset.empty())
        return 0;

    TableWriter tw(dst_path, *fset.rbegin(), chunk_size);

    list<Table*> lt;
    multimap<uint64, Table*> mm;
    Init(src_path, fset, tw, lt, mm);

    // Remove duplicates
    //PrintMultiMap(mm);
    cout << "Removing duplicate chains..." << endl;
    RemoveDup(mm);
    assert(mm.empty());

    // Close, move and clean-up
    cout << "Closing..." << endl;
    for (list<Table*>::iterator iter = lt.begin(); iter != lt.end(); ++iter)
        delete *iter;

    return 0;
}
