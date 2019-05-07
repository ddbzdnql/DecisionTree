#include <vector>
#include <string>
#include <fstream>
#include <array>

#define DECISION_COL 0
#define DEFAULT_DATA_LENGTH 4000
#define numOfProperty 22
#define inputStream "agaricus-lepiota.data.txt"

using namespace std;

class propertyNode;
class contNode;

class mushroomAnalyzer{
public:
  mushroomAnalyzer();
  void init();
  array<double, numOfProperty> getDLOrder();
  array<array<array<int, 2>, 127> ,numOfProperty> getDLCont();
  char match(string toMatch);

private:
  array<array<array<int, 2>, 127> ,numOfProperty>  DLStat;
  array<double, numOfProperty> DLIndex;
  propertyNode* root;
};

class propertyNode{
  private:
    vector<pair<int, char>> backtrace;
    array<double, numOfProperty> forward;
    mushroomAnalyzer* source;
    contNode* parent;
    vector<contNode*> children;
    vector<char> serial;
    int id;

  public:
    propertyNode(mushroomAnalyzer* from);
    propertyNode(contNode* from, array<double, numOfProperty> fw, int id);
    array<double, numOfProperty> getDLOrder();
    void build();
    mushroomAnalyzer* getSource();
    int getID();
    char proceed(string match);
};

class contNode{
  private:
    vector<pair<int,char>> backtrace;
    array<double, numOfProperty> forward;
    propertyNode* parent;
    mushroomAnalyzer* source;
    bool finality;
    char result = ' ';
    propertyNode* child;
  public:
    contNode(propertyNode* from, vector<pair<int, char>> bt);
    vector<pair<int, char>> getDLOrder();
    mushroomAnalyzer* getSource();
    void build();
    char proceed(string match);
};
