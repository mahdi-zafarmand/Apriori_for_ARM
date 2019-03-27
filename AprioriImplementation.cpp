#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <cmath>
#include <map>
#include <typeinfo>
#include <iomanip>
#include <sstream>

using namespace std;

typedef vector<set<int> > DataSet;
typedef map<set<int>, int> SetsCounter;
typedef map<int, int> ItemsCounter;

class RuleFinder;

struct Database {
// this structure is used to load the transaction database file into the program.
private:
    std::string fileName;
    DataSet* dataSet;
    ItemsCounter* itemsCounter;
    int numOfTransactions;

    void readDatabase();
    void parseTransaction(set<int>& transaction, std::string& line, const std::string& fileType);

public:
    explicit Database(std::string& _fileName):fileName(_fileName), numOfTransactions(0) {
        dataSet = new DataSet;
        itemsCounter = new ItemsCounter;
        readDatabase();
        cout << "There are " << numOfTransactions << " lines of transactions in this database." << endl;
        cout << "There are " << itemsCounter->size() << " different items in this database." << endl;
    }
    int getNumOfTransactions() { return numOfTransactions;}
    ItemsCounter* getItemsCounter() { return itemsCounter;}
    DataSet* getDataSet() { return dataSet;}
};

class Apriori {
// this class is implemented to find frequent item sets based on a given database.
    friend class RuleFinder;

private:
    Database* database;
    DataSet* dataSet;
    int minSupport;
    float minConfidence;
    int numOfTransactions;
    unsigned int precision;
    vector<SetsCounter>* fullAnswer;
    vector<string>* fullAnswerText;
    SetsCounter candidates;
    SetsCounter largestSets;
    bool done;

    void createInitialCandidates();
    void createLargestSets();
    void generateCandidates();
    bool generateCandidatesHelper(set<int>& newCandidate, const int& newInt);
    void processCandidates(const unsigned int& k);
    void processCandidatesHelper1(const set<int>& transaction, set<int>::iterator start, set<int>& subSet, int k);
    void processCandidatesHelper2(const set<int>& transaction);
    void extractFrequentItemSetsText();
    void writeFrequentItemSetsInFile();

public:
    Apriori(Database* _d, float _ms, float _mc):database(_d), minConfidence(_mc) {
        done = false;
        precision = 2;
        dataSet = database->getDataSet();
        numOfTransactions = database->getNumOfTransactions();
        minSupport = int(round((_ms < 1) ? (_ms * numOfTransactions) : (_ms)));
        fullAnswer = new vector<SetsCounter>;
        fullAnswerText = new vector<string>;
        largestSets.clear();
    }
    ~Apriori() {
        delete fullAnswer;
        delete fullAnswerText;
    }
    void findFrequentItemSets();
    void reportTheResults(char reportOption);

};

class RuleFinder {
// this class is implemented to find strong association rules based on a given frequent item sets.

private:
    Apriori* aprioriSolver;
    vector<SetsCounter>* input;
    vector<string>* fullAnswerText;
    string answerText;
    float minConfidence;
    int numOfTransactions;
    int numOfStrongRules;
    unsigned int precision;

    void findStrongRulesHelper1(const set<int>& xANDy, set<int>::iterator start, set<int>& x, set<int>& y, size_t k);
    void findStrongRulesHelper2(const set<int>& x, const set<int>& y, float support, float confidence);
    void writeStrongRulesInFile();

public:
    explicit RuleFinder(Apriori* _as):aprioriSolver(_as), numOfStrongRules(0) {
        input = _as->fullAnswer;
        minConfidence = _as->minConfidence;
        numOfTransactions = _as->numOfTransactions;
        precision = _as->precision;
        fullAnswerText = new vector<string>;
    }
    ~RuleFinder() {
        delete fullAnswerText;
    }
    void findStrongRules();
    void reportTheResults(char reportOption);
};


template <typename T>
string set2string(set<T> s) {				// to print a set of items in a line -> used to generate txt
    string message;
    for (const auto& item : s) {
        message.append(to_string(item));
        message.append(", ");
    }
    return message.substr(0, message.size() - 2);
}

unsigned int nChoosek(unsigned int n, unsigned int k) {
    // this function returns the number of combinations(n,k), we can say C(n,k) = n! / k! * (n-k)!
    if (k > n) return 0;
    if (k * 2 < n) k = n-k;
    if (k == 0) return 1;
    unsigned int result = 1;
    unsigned int r = 1;
    while (n > k) {
        result *= ++k;
        result /= r++;
    }
    return result;
}

//////////////////////////////////////////
//////////////////////////////////////////

int main(int argc, char* argv[]) {
    clock_t timerStart = clock();
    cout << "Timer starts ...\n";
    std::string fileName = "10Kdataset.txt";
    float minimumSupport = 0.001;
    float minimumConfidence = 0.8;
    char reportOption = 'a';
    if (argc == 5) {
        fileName = argv[1];
        minimumSupport = atof(argv[2]);
        minimumConfidence = atof(argv[3]);
        reportOption = *(argv[4]);
    } else if (argc == 4) {
        fileName = argv[1];
        minimumSupport = atof(argv[2]);
        minimumConfidence = atof(argv[3]);
        reportOption = ' ';
    } else {
        cout << "Not enough input arguments!\n" <<endl;
        return 0;
    }
    auto* database = new Database(fileName);
    auto* apriori = new Apriori(database, minimumSupport, minimumConfidence);
    auto ruleFinder = new RuleFinder(apriori);
    apriori->findFrequentItemSets();
    apriori->reportTheResults(reportOption);
    ruleFinder->findStrongRules();
    ruleFinder->reportTheResults(reportOption);
    clock_t timerStop = clock();
    float elapsedTime = float(timerStop-timerStart) / CLOCKS_PER_SEC;
    cout << "Elapsed time = " << elapsedTime << " seconds."<< endl;
    return 0;
}

///////////////////////////////////////////
///////////////////////////////////////////

void Database::readDatabase() {
    // this function reads the Database from a txt or csv file.
    std::ifstream inputFile(fileName.c_str());
    std::string typeOfFile = fileName.substr(fileName.rfind('.'));
    std::string line;
    set<int> transaction;
    while(getline(inputFile, line)) {
        parseTransaction(transaction, line, typeOfFile);
        dataSet->push_back(transaction);
        for (const auto& item : transaction) {
            (*itemsCounter)[item]++;
        }
        transaction.clear();
        numOfTransactions++;
    }
    inputFile.close();
}

void Database::parseTransaction(set<int>& transaction, std::string& line, const std::string& fileType){
    // this method takes a line of transaction and updates the #transaction set of items in it.
    // input line is a string. if there is a \n in the end of the line in .txt file, this method removes it.
    // {I added supporting csv files because the only databases other than the given one I found were in csv format.}
    if (fileType == ".txt" && line.find('\n') > 0) {
        line = line.substr(0, line.find('\n'));
    }
    if (fileType == ".csv") {
        line = line.substr(1, line.length() - 1);
        line[line.length() - 1] = ',';
    }
    size_t delimiterIndex;      // items will be separated using the corresponding delimiter of file type.
    while (fileType == ".txt" && line.length() > 0) {
        delimiterIndex = line.find(' ');
        transaction.insert(stoi(line.substr(0, delimiterIndex)));
        line = line.substr(delimiterIndex + 1);
    }
    while (fileType == ".csv" && line.length() > 0) {
        delimiterIndex = line.find(',');
        transaction.insert(stoi(line.substr(0, delimiterIndex)));
        line = line.substr(delimiterIndex + 1);
    }
}

void Apriori::createInitialCandidates() {
    // this method traverses on the #database's #itemsCounter and updates the #candidate attribute of its own.
    set<int> tempSet;
    for (const auto& onePair : *(database->getItemsCounter())) {
        tempSet.insert(onePair.first);
        candidates.insert(pair<set<int>, int>(tempSet, onePair.second));
        tempSet.erase(onePair.first);
    }
}

void Apriori::createLargestSets() {
    // this method creates LargestSets, then updates #fullAnswer based on sets and their number of repetition
    for (const auto& candidate : candidates) {    // SetsCounter: map* of {set<int>, int}
        if (candidate.second >= minSupport) {
            largestSets.insert(pair<set<int>, int>(candidate.first, candidate.second));
        }
    }
    candidates.clear();                         // to make it prepared for getting the next round of candidates.
    done = largestSets.empty();                 // #largestSets is the SetsCounter containing LargestSets.
    extractFrequentItemSetsText();              // extract frequent item sets in format of strings
    fullAnswer->push_back(largestSets);         // adding LargestSets to #fullAnswer to keep it until the end.
}

void Apriori::extractFrequentItemSetsText() {
    // this method converts the items in the #largeSets into string with regards to the given precision.
    string tempString;
    stringstream tempStream;
    for (const auto& oneSetInfo : largestSets) {
        tempString.append(set2string(oneSetInfo.first));
        tempStream << fixed << setprecision(precision) << float(oneSetInfo.second)/numOfTransactions;
        tempString.append(" (" + tempStream.str() + ")\n");
        tempStream.str("");
    }
    if (tempString.empty())
        return;
    fullAnswerText->push_back(tempString);
}

void Apriori::findFrequentItemSets() {
    // this method runs the Apriori algorithms to find the frequent item sets.
    fullAnswer->clear();        // vector* of map* of {set<int>, int}: each item of vector for a different k.
    unsigned int k = 1;
    createInitialCandidates();  // creates 1-Candidates based on database.
    createLargestSets();        // creates 1-LargestSets and add them to #fullAnswer
    while (!done) {             // done becomes true when there is no #largestSets to continue.
        ++k;
        generateCandidates();   // each time for every loop, first it generates #candidates.
        processCandidates(k);   // each time for every loop, it extracts #largestSets.
        createLargestSets();    // creates k-LargestSets and add them to #fullAnswer
    }
    fullAnswer->pop_back();
}

void Apriori::generateCandidates() {
    // this method generates all possible new candidates based on three below conditions, and insert them in #candidates.
    set<int> allPreviousSets, tempSet;
    // first, extract all available items.
    for (const auto& oneSet : largestSets) {
        allPreviousSets.insert(oneSet.first.begin(), oneSet.first.end());
    }
    /*
     * 1. new item should not be in the previous set.
     * 2. all (k-1)-subsets of newly k-items generated set should be inside previous #largestSets.
     * 3. newly generated set should not be added to #candidates using different order of items (but same set).
     */
    for (auto& prevSet : largestSets) {
        for (const auto& newInt : allPreviousSets) {
            tempSet = prevSet.first;
            if (tempSet.find(newInt) == tempSet.end()) {
                if (generateCandidatesHelper(tempSet, newInt)) {
                    if (candidates.find(tempSet) == candidates.end()) {
                        // initial its number of repetition to zero, it will increment in #processCandidatesHelpers
                        candidates.insert(pair<set<int>, int>(tempSet,0));
                    }
                }
            }
        }
    }
    largestSets.clear();
}

bool Apriori::generateCandidatesHelper(set<int>& newCandidate, const int& newInt) {
    // this method investigates if all (k-1)-subsets of the #newCandidate exists in previous #largeSets or not.
    newCandidate.insert(newInt);
    for (const auto item : newCandidate) {
        newCandidate.erase(item);
        if (largestSets.find(newCandidate) == largestSets.end()) return false;
        newCandidate.insert(item);
    }
    return true;
}

void Apriori::processCandidates(const unsigned int& k) {
    // this method utilizes two helper methods to whether add a previously generated candidate to #fullAnswer or not.
    set<int> dummySet;
    size_t candidatesSize = candidates.size();
    for (const auto& transaction : *dataSet) {
        if (transaction.size() >= k) {                   // ignores transactions with less number of items than k
            // candidates are more than the k-subsets of #transaction or vice versa, proceed to add based on this.
            if (candidatesSize > nChoosek(transaction.size(), k)) {
                //dummySet.clear();
                processCandidatesHelper1(transaction, transaction.begin(), dummySet, k);
            } else {
                processCandidatesHelper2(transaction);
            }
        }
    }
}

void Apriori::processCandidatesHelper1(const set<int>& transaction, set<int>::iterator start, set<int>& subSet, int k) {
    // this method increases number of repetition of every possible k-subset (increment in the #candidates attribute)
    // of the #transaction, if that #subset exists in the #candidates as well.
    if(k == 0){               // it checks if the subset has exactly k items.
        if(candidates.find(subSet) != candidates.end()){
            candidates[subSet]++;
        }
    } else {				// recursively traverses on all items to generate all possible k-subsets of #transaction.
        // I got the idea of creating subsets like this, from a github repository that does not exist anymore. I just
        // want to indicate that the whole idea is not mine, but I implement that differently and more efficiently.
        for(auto item = start; item != transaction.end(); ++item){
            subSet.insert(*item);
            processCandidatesHelper1(transaction, ++start, subSet, k - 1);
            subSet.erase(*item);
        }
    }
}

void Apriori::processCandidatesHelper2(const set<int>& transaction) {
    // this method increases number of repetition of each #candidate by one (meaning that candidate exist in #transaction),
    // unless there is at least one #item in that #candidate, which does not exist in the #transaction.
    bool shouldIncrease = true;
    for (const auto& candidate : candidates) {
        for (const auto& item : candidate.first) {
            if (transaction.find(item) == transaction.end()) {
                shouldIncrease = false;
                break;
            }
        }
        if (shouldIncrease) {
            candidates[candidate.first]++;
        }
        shouldIncrease = true;
    }
}

void Apriori::reportTheResults(const char reportOption) {
    // this method manages output of frequent #largeItemSets, both displaying and writing to file
    writeFrequentItemSetsInFile();
    if (reportOption == 'f' || reportOption == 'a' || reportOption == ' ') {
        size_t i = 0;
        for (const auto& oneMap : *fullAnswer) {
            cout << "Number of frequent " << to_string(++i) << "_itemsets: " << oneMap.size() << "\n";
        }
    }
}

void Apriori::writeFrequentItemSetsInFile() {
    // this method writes frequent item sets in a the file.
    string fileName;
    for (size_t i = 0; i< fullAnswerText->size(); i++) {
        fileName = "frequent " + to_string(i + 1) + "_itemsets.txt";
        ofstream outputFile(fileName);
        outputFile << (*fullAnswerText)[i];
        outputFile.close();
    }
}

void RuleFinder::findStrongRules() {
    // this method finds all strong association rules.
    set<int> x, y;
    for (size_t i = 1; i< input->size(); ++i) {                 // since 1-largest sets cannot make association rules
        for (auto& oneSetInfo : (*input)[i]) {   // separates one {set<int>, int} for one set of k-frequent item sets.
            for (size_t j = 1; j < oneSetInfo.first.size(); ++j) {                           // for non-empty subsets
                findStrongRulesHelper1(oneSetInfo.first, oneSetInfo.first.begin(), x, y, j);
            }
        }
        fullAnswerText->push_back(answerText.substr(0, answerText.size() - 1));
        answerText.clear();
    }
}

void RuleFinder::findStrongRulesHelper1(const set<int>& xANDy, set<int>::iterator start, set<int>& x, set<int>& y, size_t k) {
    if(k == 0){             // it checks if the subset has exactly k items.
        float confidence = float((*input)[xANDy.size() - 1][xANDy]) / (*input)[y.size() - 1][y];
        if (confidence >= minConfidence) {
            numOfStrongRules++;
            set_difference(xANDy.begin(), xANDy.end(), y.begin(), y.end(), inserter(x, x.end()));
            findStrongRulesHelper2(y, x, float((*input)[xANDy.size() - 1][xANDy])/numOfTransactions, confidence);
            x.clear();
        }
    } else {				// recursively traverses on all items to generate all possible k-subsets of #xANDy.
        for(auto item = start; item != xANDy.end(); ++item){
            y.insert(*item);
            findStrongRulesHelper1(xANDy, ++start, x, y, k - 1);
            y.erase(*item);
        }
    }
}

void RuleFinder::findStrongRulesHelper2(const set<int>& x, const set<int>& y, float support, float confidence) {
    // this method makes a string for a string rule and appends it to the #answerText.
    stringstream tempStream;
    string tempString1 = set2string(x);
    string tempString2 = set2string(y);
    string strongRule = tempString1 + " -> " + tempString2;
    tempStream << fixed << setprecision(precision) << support << confidence ;
    string tempString3 = tempStream.str();
    strongRule.append(" (" + tempString3.substr(0, precision + 2) + ", " + tempString3.substr(precision + 2) + ")\n");
    answerText.append(strongRule);
}

void RuleFinder::reportTheResults(const char reportOption) {
    // this method manages output of frequent strong rules, both displaying and writing to file
    writeStrongRulesInFile();
    if (reportOption == 'r' || reportOption == 'a' || reportOption == ' ') {
        cout << "Number of association rules: " << numOfStrongRules << endl;
    }
}

void RuleFinder::writeStrongRulesInFile() {
    // this method writes strong rules in a file.
    ofstream outputFile("associationRules.txt");
    for (const auto& item : *fullAnswerText) {
        outputFile << item << endl;
    }
    outputFile.close();
}
