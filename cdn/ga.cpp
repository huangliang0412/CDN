#include "ga.h"
#include "graph.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
using namespace std;
Answer::Answer(int length): serverNode(length, 0), bestServerNode(length, 0), coefficient(length, 0.0), lowestCost(INFLL), cost(INFLL) {}

Answer::Answer(int length, vector<int> & serve, Graph  &pic): serverNode(length, 0), bestServerNode(length, 0), coefficient(length, 0.0) {
    int size = serve.size();
    for (int i = 0; i < size; ++i) {
        bestServerNode[serve[i]] = serverNode[serve[i]] = 1;
    }
    ServerAdd(serve, pic);
    cost = lowestCost = mcmf(pic) + size * pic.server_cost;
}
bool cmp(const Answer & answer1, const Answer & answer2) {
    if(answer1.cost == answer2.cost)
        return answer1.lowestCost < answer2.lowestCost;
    else
        return answer1.cost < answer2.cost;
}

PSO::PSO(Graph &pic) {
    //uck = &fk;
    spfa(pic);
    len = pic.NetNodeNum;
    pmutation = 0.012;
    pcross = 0.8;
    pManSelect = 0.01;
    temp = 1;
    c1 = 2.0; // 1.6
    c2 = 2.0; // 2.0
    w = 0.85; // 0.8

}

void PSO::SwapService(Answer &s, Graph &pic) {
    //clock_t t1 = clock();
    /*
    int r1, r2;
    do {
        r1 = rand() % len;
    } while (s.serverNode[r1] > 0.5);
    do {
        r2 = rand() % len;
    } while (s.serverNode[r2] < 0.5);
    swap(s.serverNode[r1], s.serverNode[r2]);

    */
    vector<int> server, unserver;
    for (int i = 0; i < len; ++i) {
        if (s.serverNode[i]) {
            server.push_back(i);
        } else {
            unserver.push_back(i);
        }
    }
    int isServerindex = 0;
    int isnotServerindex = 0;
    int server_size = server.size();
    int unserver_size = unserver.size();
    int isServer  = 0;
    int isnotServer = 0;
    random_shuffle(server.begin(), server.end());
    random_shuffle(unserver.begin(), unserver.end());

    for (int i = 0; i < 3; ++i) {
        int num = 15 * ((temp >> 8) + 1);
        for (; isServerindex < server_size; ++isServerindex) {
            isServer = server[isServerindex];
            if (serverSet[isServer] < temp)
                break;
        }
        serverSet[isServer] = num;
        for (; isnotServerindex < unserver_size; ++isnotServerindex) {
            isnotServer = unserver[isnotServerindex];
            if (serverSet[isnotServer] < temp)
                break;
        }
        serverSet[isnotServer] = 0.7 * num;  // 0.7
        swap(s.serverNode[isServer], s.serverNode[isnotServer]);
        swap(server[isServerindex], unserver[isnotServerindex]);
        ++temp;
        if (isServerindex == server_size || isnotServerindex == unserver_size)
            break;
    }
    ServerAdd(server, pic);
    s.cost = mcmf(pic) + server.size() * pic.server_cost;
    if (s.cost < s.lowestCost) {
        s.bestServerNode = s.serverNode;
        s.lowestCost = s.cost;
    }
    if (s.lowestCost < bestAnswer.lowestCost) {
        bestAnswer.bestServerNode = s.bestServerNode;
        bestAnswer.lowestCost = s.lowestCost;
        //std::cout << bestAnswer.lowestCost << endl;
    }


}
void PSO::mutation(vector<double> &v) {
    /*for (int i = 0; i < v.size(); i++)
    {
        if (random1() < pmutation)
        {
            int mp = (int)random1()*v.size();
            v[mp] = v[i]^1;
        }
    }*/
    for (int i = 0; i < v.size(); i++) {
        double t = (double) rand() / RAND_MAX;
        if (t < pmutation) {
            v[i] += ((double) rand() / RAND_MAX) * 2 - 1;
            //v[i] = max(0.0, v[i]);
            v[i] = min(v[i], 1.0);
        }

    }
    for (int i = 0; i < v.size(); i++) {
        double t = (double) rand() / RAND_MAX;
        if (t < pManSelect) {
            v[i] -= 0.5;
            //v[i] = max(0.0, v[i]);
            v[i] = min(v[i], 1.0);
        }

    }
}
void PSO::placeServer(vector<int> &server) {
    decode(bestAnswer.bestServerNode, server);
}
void PSO::addServer(vector<int> &servervec, Graph &pic) {
    anwserSet.emplace_back(len, servervec, pic);
}
void PSO::decode(vector<int> & ansser1, vector<int> & ansser2) {
    ansser2.clear();
    for (int i = 0; i < len; ++i) {
        if (ansser1[i])
            ansser2.push_back(i);
    }
}



void PSO::cross(Answer & s1, Answer & s2) {
    //clock_t t1 = clock();
     int r1 = rand() % len, r2 = rand() % len;
    if (r1 > r2)
        swap(r1, r2);
    while (r1 < r2) {
        swap(s1.serverNode[r1], s2.serverNode[r1]);
        ++r1;
    }
    swap(s1.serverNode, s2.serverNode);
    //cout << "cross:" << (double)(clock()- t1) / CLOCKS_PER_SEC << endl;
}




void PSO::update(Answer &answ, Graph &pic) {
    //clock_t t1 = clock();
    // int s;
    for (int i = 0; i < len; ++i) {
        answ.coefficient[i] = (w * answ.coefficient[i] + c1 * rand() / RAND_MAX * (answ.bestServerNode[i] - answ.serverNode[i]) + c2 * rand() / RAND_MAX * (bestAnswer.bestServerNode[i] - answ.serverNode[i]));
        // std::cout << answ.coefficient[i] << std::endl;
        // answ.serverNode[i] = 1 / (1 + exp( -100 * answ.coefficient[i]));
        // if (rand() / RAND_MAX < s)
        //   answ.serverNode[i] = 1;
        // else
        //   answ.serverNode[i] = 0;
        answ.serverNode[i] = 1 / (1 + exp(100 * (0.5 - answ.serverNode[i] - answ.coefficient[i]))) + 0.5;  // sigmoid function
        //std::cout << answ.serverNode[i] << std::endl;
        // answ.serverNode[i] = 1 / (1 + exp(0.0001 - answ.coefficient[i]));
    }
    /*
    vector<int> serv;
    decode(answ.serverNode, serv);
    ServerAdd(serv, pic);
    answ.cost = mcmf(pic) + serv.size() * pic.server_cost;
    if (answ.cost < answ.lowestCost) {
        answ.lowestCost = answ.cost;
        answ.bestServerNode = answ.serverNode;

        if (answ.lowestCost < bestAnswer.lowestCost) {
            bestAnswer.bestServerNode = answ.bestServerNode;
            bestAnswer.lowestCost = answ.lowestCost;
        }
        //std::cout << bestAnswer.lowestCost << std::endl;
    }
    //cout << "PSO:" << (double)(clock()- t1) / CLOCKS_PER_SEC << endl;
     */
}

void PSO::run(Graph &pic) {
    int i;
    int j = max >> 1;
    for (i = 0; i < max; ++i) {
        SwapService(anwserSet[i], pic);
    }
    for (i = 0; i < max; ++i) {
        update(anwserSet[i], pic);
    }

   cout << bestAnswer.lowestCost << endl;
}

double PSO::init(int size, Graph &pic) {
    max = size;
    serverSet.resize(len, 0);

    int number = anwserSet.size(), limit_size = max >> 1;
    vector<int> server;
    sort(anwserSet.begin(), anwserSet.end(), cmp);
    bestAnswer = anwserSet[0];
    decode(bestAnswer.bestServerNode, server);
    int best_size = server.size() * 0.7;
    if (number < limit_size) {
        for (int i = number; i < max; ++i) {
            kmeans(best_size, server, pic);
            addServer(server, pic);
        }
    } else {
        anwserSet.resize(limit_size);
        for (int i = limit_size; i < max; ++i) {
            kmeans(best_size, server, pic);
            addServer(server, pic);
        }
    }
    clock_t t1 = clock();
    run(pic);
    clock_t t2 = clock();
    return double(t2 - t1) / CLOCKS_PER_SEC;
}
