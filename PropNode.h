#include <vector>
#include <string>


using namespace std;

class propNode{
public:
  propNode(mushroomAnalyzer* fromFile, int index);

private:
  mushroomAnalyzer* source;
  int id;
  int[127] cont;
  contNode*[127] contList;
}
