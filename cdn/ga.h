//#pragma once
#ifndef CDN_GA_H
#define CDN_GA_H

#include "graph.h"
#include <vector>
//using namespace std;
struct Graph;
//class Answer;

class Answer {
public:
    Answer(int length=0);
    Answer(int length, std::vector<int> & serve, Graph &pic);
    int lowestCost;
    int cost;
    std::vector<int> serverNode;
    std::vector<int> bestServerNode;
    std::vector<double> coefficient;

};

bool cmp(const Answer & answer1, const Answer & answer2);
class PSO {
public:
    PSO(Graph &pic);
    void mutation(std::vector<double >& vec);
  //  mutation
    void decode(std::vector<int> & ansser1, std::vector<int> & ansser2);
    void cross(Answer & s1, Answer & s2);
    void SwapService(Answer &s, Graph &pic);
    void update(Answer &answ, Graph &pic);
    void placeServer(std::vector<int> &server);
    void addServer(std::vector<int> &servervec, Graph &pic);
    void run(Graph &pic);
    double init(int size, Graph &pic);


private:
    std::vector<Answer> anwserSet;
    std::vector<int> serverSet;
    Answer bestAnswer;
    int len;
    int temp;
    int max;
    double pmutation;
    double pcross;
    double pManSelect;
    double c1;
    double c2;
    double w;
};
#endif //CDN_GA_H
