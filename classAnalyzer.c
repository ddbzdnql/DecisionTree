#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "classAnalyzer.h"

using namespace std;

mushroomAnalyzer::mushroomAnalyzer(){}

/*
 * The detailed initializer of mushrronAnalyzer; using the input
 * to fill-in:
 *   DLStat, the 3-dimensional array that stores how
 *   one single property influences the mushroom's edibility.
 *   DLIndex, the int array that will store the ordering of all
 *   properties by comparing the majority rate of each property.
 *   The algorithm applied here does not strictly stick to the 
 *   information gain method but is also more optimal than just
 *   random selection.
 */
void 
mushroomAnalyzer::init(){
  ifstream inputF;
  /* 0-fill all the DLStat and DLIndex arrays. */
  for (int i = 0; i < numOfProperty; i++){
    DLIndex[i] = 0;
    for (int j = 0; j < 127; j++){
      DLStat[i][j][0] = 0;
      DLStat[i][j][1] = 0;     
    }
  }

  /* Open the file for input. */
  inputF.open(inputStream);
  /* Check if successfully opened. */
  /* Read the data one line at a time. Split the line delimited 
   * by ',' and if this line indicates an edbile mushroom, 
   * increase 1 for the first slot in every DLStat element that
   * corresponds to a property of this mushroom. 
   * e.g.: the line read in is:
   * e,x,s,y,t,a,f,c,b,k,e,c,s,s,w,w,p,w,o,p,n,n,h 
   * because its 1st property is 'x'(120) and it is edible,
   * DLStat[0][120][0] should increase by 1, meaning that
   * there exists at least one datum whose first property
   * is x and is edible.
   * Applying this operation to all the data, we will get an
   * array containing the info apopos how one single property
   * influences the edibility. 
   */
  if(inputF.is_open()){
    for (int i = 0; i < DEFAULT_DATA_LENGTH; i++){
      string nextLine;
      getline(inputF, nextLine);
      char edibility = nextLine[0];
      int count = 1;
      for (int j = 1; j < nextLine.length(); j++){
        if (nextLine[j] != ','){
          if (edibility == 'e'){
            DLStat[count-1][nextLine[j]][0] ++;
          }
          else{
            DLStat[count-1][nextLine[j]][1] ++;
          }
          count ++;
        }
      }
    }

    /* With DLStat filled-in, the program goes on to decide the order
     * by which it splits the properties.
     * To achieve this, the program calculates the majority rate for
     * each property.
     * e.g.: DLStat[0][120][0] is 100 and DLStat[0][120][1] is 0.
     * The majority rate for x in the first property is then:
     * MAX(DLStat[0][120][0], DLStat[0][120][1]) / (DLStat[0][120][0]
     * + DLStat[0][120][1])
     * Therefore this value is added into DLIndex[0] and the count is
     * increased by 1.
     * When the first property has been exhausted, divide DLIndex[0]
     * with count will get the averge majority rate for the 1st property.
     */
    for (int i = 0; i < numOfProperty; i++){
      int count = 0;
      for (int j = 0; j < 127; j++){
        if (DLStat[i][j][0] || DLStat[i][j][1]){
          count ++;
          double rate = 2 - 2 * (DLStat[i][j][0] > DLStat[i][j][1] 
            ? DLStat[i][j][0] : DLStat[i][j][1] + 0.0) / 
            (DLStat[i][j][0] + DLStat[i][j][1]);
          DLIndex[i] += rate;
        }
      }
      DLIndex[i] = DLIndex[i] / count;
    }
    inputF.close();
    root = new propertyNode(this);
  }
  else{
    cout << "Error opening file!" << endl;
  }

}

/* Accessor method for mushroonAnalyzer that returns DLIndex*/
array<double, numOfProperty> 
mushroomAnalyzer::getDLOrder(){
  return DLIndex;
}

/* Acessor method for mushroomAnalyzer that returns DLStat */
array<array<array<int, 2>, 127>, numOfProperty> 
mushroomAnalyzer::getDLCont(){
  return DLStat;
}

/* The method that makes the prediction by recursively calling
 * proceed to matching propertyNodes and contNodes. */
char 
mushroomAnalyzer::match(string toMatch){
  return (root -> proceed(toMatch));
}

/* Constructo for root propertyNode.
 * The operations consist of:
 *   set the source using from
 *   set the forward array using the DLIndex in source
 *   delete the first element in forward array in order
 *   set the id
 *   build its children, which is a list of contNodes
 */
propertyNode::propertyNode(mushroomAnalyzer* from){
  source = from;
  forward = source -> getDLOrder();
  int min = 0;
  for (int i = 0; i < numOfProperty; i++){
    if (forward[i] < forward[min]){
      min = i;
    }
  }
  forward[min] = 100;
  id = min;
  //cout << id << " start building propertyNode" << endl;
  build();
}

/*
 * Constructor for child propertyNode.
 * The operations consist of:
 *   set parent
 *   set forward
 *   set source
 *   set backtrace
 *   build its children
 */
propertyNode::propertyNode(contNode* from, array<double, numOfProperty> fw, int id){
  parent = from;
  forward = fw;
  source = parent -> getSource();
  backtrace = parent -> getDLOrder();
  this -> id = id;
  //cout << id << " start building propertyNode" << endl;
  this -> build();
  //cout << id << " propertyNode build complete" << endl;
}

/*
 * The method that interconnects between levels of propertyNodes
 * and contNodes.
 * One propertyNode has many contNodes as children since one
 * property can have many contents for it.
 * Construct a child node for each possible content by pushing
 * and then popping different pair of <int, char> into backtrace.
 * The forward array can be left unchanged because this build step
 * does not go to the next property in the tree.
 */
void propertyNode::build(){
  array<array<array<int, 2>, 127>, numOfProperty> DLStat = source -> getDLCont();
  vector<pair<int, char>> copy = backtrace;

  for(int i = 0; i < 127; i++){
    if (DLStat[id][i][0] || DLStat[id][i][1]){
      pair<int, char> toAdd(id, i);
      copy.push_back(toAdd);
      serial.push_back(i);
      //cout << "continue to next" << endl; 
      contNode* toPush = new contNode(this, copy);
      children.push_back(toPush);
      copy.pop_back();
    }
  }
}

/*
 * Accessor method for propertyNode that returns the ID
 */
int propertyNode::getID(){
  return id;
}

/*
 * Accessor method for propertyNode that returns the
 * forward array
 */
array<double, numOfProperty> propertyNode::getDLOrder(){
  return forward;
}

/*
 * Accessor method for propertyNode that returns the
 * source pointer to the mushroomAnalyzer
 */
mushroomAnalyzer* propertyNode::getSource(){
  return source;
}

/*
 * Looks like the reverse of build, search through
 * all possible content of this property and find the
 * one that matches the given string, call proceed
 * method on that one contNode child.
 */
char propertyNode::proceed(string match){
  char toComp = match[id*2+2];
  for (int i = 0;  i < children.size(); i++){
    if (serial[i] == toComp){
      return (children[i] -> proceed(match));
      break;
    } 
  }
  return 'E';
}

/*
 * Constructor for contNode
 * The operations consist of:
 *   set the backtrace
 *   set the parent
 *   set the forward
 *   set the source
 *   initialize finality with true
 *   build its child
 */
contNode::contNode(propertyNode* from, vector<pair<int, char>> bt){
  backtrace = bt;
  parent = from;
  forward = from -> getDLOrder();
  source = parent -> getSource();
  finality = true;
  build();
}

/*
 * Accessor method for contNode that returns backtrace
 */
vector<pair<int, char>> contNode::getDLOrder(){
  return backtrace;
}

/*
 * Accessor method for contNode that returns source
 */
mushroomAnalyzer* contNode::getSource(){
  return source;
}

/*
 * The build method for contNode.
 * Check through the database and match each data with
 * this -> backtrace; if the result does not vary
 * throughout the whole match process, then terminate
 * and return because this trace is final by reaching
 * this node. If the result varies at some point, 
 * terminate and go on to the next property, because
 * this trace is not final up to this node.
 * If one trace exhausted the properties and still
 * did not reach a final trace, set finality as true
 * but result as 'E' and return.
 */
void contNode::build(){
  ifstream inputF;
  inputF.open(inputStream);
  if (inputF.is_open()){
    char res;
    for (int i = 0; i < DEFAULT_DATA_LENGTH; i++){
      int ini = 1; int right = 1;
      string nextLine;
      getline(inputF, nextLine);
      for (int j = 0; j < backtrace.size(); j ++){
        int index = (backtrace[j].first) * 2 + 2;
        char match = backtrace[j].second;
        if (nextLine[index] != match){
          right = 0;
          break;
        }
      }
      if (right){
        if (ini){
          res = nextLine[0];
          ini = 0;
        }
        else{
          if (res != nextLine[0]){
            finality = false;
            break;
          }
        }
      }
    }
    inputF.close();
    if (!finality){
      //cout << parent -> getID() << " contNode build complete" << endl;
      int min = 0; int count = 0;
      for (int i = 0; i < numOfProperty; i++){
        if (forward[i] < forward[min]){
          min = i;
        }
        if (forward[i] == 100){
          count ++;
        }
      }
      if (count != numOfProperty){
        forward[min] = 100;
        //cout << "continue to next" << endl;
        child = new propertyNode(this, forward, min);
      }
      else{
        finality = true;
        result = 'E';
      }
    }
    else{
      result = res;
      //cout << parent -> getID() << " contNode build complete" << endl;
      return;
    }
  }
  else{
    cout << "Error opening file" << endl;
  }
}

/*
 * The proceed method for contNode.
 * If this node is final, return its result.
 * If this node is not final, go to next property.
 */
char contNode::proceed(string match){
  if (finality){
    return result;
  }
  else{
    return (child -> proceed(match));
  }
}

/*
 * Tester for the Decision Tree structure.
 */
int main(){
  cout << "Start building DT..." << endl;
  /* Create and initialize the object to test with */
  mushroomAnalyzer test;
  test.init();
  cout << "DT building complete. Now running auto test with 4000 sets of data..." << endl;
  /* Create connect to the database */
  ifstream inputF;
  inputF.open(inputStream);
  /* Create the file that saves the output */
  ofstream outputF;
  outputF.open("ACCURACY");
  /* Run the Decision Tree on 4000 sets of test data */
  int countR = 0; int countW = 0;
  for (int i = 0; i < 8000; i++){
    if (i >= 4000){
    string nextLine;
    getline(inputF, nextLine);
    char res = test.match(nextLine);
      if (res != nextLine[0]){
        outputF << "Wrong prediction on test " << i 
          << ": should be " << nextLine[0] 
          << ", but was " << res << endl;
        countW++;
      }
      else{
        outputF << res << " is the correct prediction on " << i << endl;
        countR++;
      }
    }
  }
  inputF.close();
  outputF << "Out of " << (countR + countW) << " predictions, "
    << countR << " are right, standing at a "
    << (countR + 0.0) / (countR + countW) * 100 << "% rate of accuracy." << endl;
  cout << "Out of " << (countR + countW) << " predictions, "
    << countR << " are right, standing at a "
    << (countR + 0.0) / (countR + countW) * 100 << "% rate of accuracy." << endl;

  /* Start the manual test */
  string inputS;
  cout << "The auto test has been completed and the result is in ACCURACY file," << endl 
    << "now you can manually test this DT." << endl;
  cout << "Please type in data in the form of ?,x,s,n,t,p,f,c,n,k,e,e,s,s,w,w,p,w,o,p,k,s,u."
    << endl << "Then press enter, the program will first predict its edibility and then search through"
    << "the data for actual match and compare both." <<endl;
  cout << "To quit, type in quit and enter." << endl << "Please type in mushroom description." << endl;
  cin >> inputS;
  while (inputS != "quit"){
    char res = test.match(inputS);
    inputS[0] = res;

    string wrong = inputS;
    if (res == 'e'){
      cout << "According to the DT, this mushroom is edible." << endl;
      cout << "Checking for match in database." << endl;
      wrong[0] = 'p';
    }
    else{
      cout << "According to the DT, this mushroom is poisonous." << endl;
      cout << "Checking for match in database." << endl;      
      wrong[0] = 'e';
    }
    inputF.open(inputStream);
    bool correctness = true;
    for (int i = 0; i < 8000; i++){
      string nextLine;
      getline(inputF, nextLine);
      if (nextLine == inputS){
        cout << "Prediction is correct." << endl;
        correctness = false;
        break;
      }
      if (nextLine == wrong){
        cout << "Prediction is wrong." << endl;
        correctness = false;
        break;
      }
    }
    if (correctness){
      cout << "Not match found." << endl;
    }
    cout << "Please type in mushroom description." << endl << "To quit, type in quit and enter." << endl;
    cin >> inputS;
  }
}
