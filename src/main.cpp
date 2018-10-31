#include <iostream>
#include <vector>
#include <thread>
#include <unordered_set>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::thread;
using std::move;
using std::ref;
using std::distance;
using std::unordered_set;

template<typename T>
void sum(T *start, T *end, T &sumOfRange) {
    // TODO does this work for generics?
    sumOfRange = 0;

    T *curr = start;
    while (curr < end) {
        sumOfRange += *curr;
        curr++;
    }
}

template<typename T>
thread sumInThread(T *start, T *end, T &sumOfRange) {
    return move(thread(sum<int>, start, end, ref(sumOfRange)));
}

int getNumberOfThreads() {
    int numberOfHardwareThreads = thread::hardware_concurrency();
    if (numberOfHardwareThreads < 2) {
        numberOfHardwareThreads = 2;
    }
    return numberOfHardwareThreads;
}

template<typename T>
void fork(T *start, T *end, int numberOfThreads, vector<thread> &threads, vector<T> &sumResults) {
    long numberOfElements = distance(start, end);
    int elementsPerThread = static_cast<int>(numberOfElements / (numberOfThreads));

    T *current = start;

    for (int i = 0; i < numberOfThreads - 1; i++) {
        threads.emplace_back(sumInThread<int>(current, current + elementsPerThread, sumResults[i + 1]));
        current += elementsPerThread;
        cerr << "Started thread " << i << endl;
    }

    sum<int>(current, end, sumResults[0]);
}

template<typename T>
T join(vector<thread> &threads, vector<T> &sumResults) {
    T totalSum = sumResults[0];

    unordered_set<int> finishedThreads;
    bool areAllThreadsFinished = false;
    while (!areAllThreadsFinished) {
        bool areAllThreadsFinishedInThisLoop = true;
        for (int i = 0; i < threads.size(); i++) {
            if (finishedThreads.count(i) == 1) {
                continue;
            }
            if (threads[i].joinable()) {
                cerr << "Joining thread " << i << endl;
                threads[i].join();
                cout << "Value of " << i << ": " << sumResults[i + 1] << endl;
                totalSum += sumResults[i + 1];
                finishedThreads.insert(i);
            } else {
//                cerr << "Can't join thread " << i << " yet" << endl;
                areAllThreadsFinishedInThisLoop = false;
            }
        }
        areAllThreadsFinished = areAllThreadsFinishedInThisLoop;
    }

    return totalSum;
}

template<typename T>
int multithreadedSum(T *start, T *end) {
    int numberOfThreads = getNumberOfThreads();

    vector<T> sumResults;
    vector<thread> threads;

    sumResults.reserve(static_cast<unsigned long>(numberOfThreads));
    threads.reserve(static_cast<unsigned long>(numberOfThreads - 1));

    fork(start, end, numberOfThreads, threads, sumResults);
    T totalSum = join(threads, sumResults);


    return totalSum;
}

int main() {
    vector<vector<int>> tests = {
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
            {1},
            {0},
            {-1, -2, -3},
            {1, 2, 3, 4},
            {1, 2, 3}
    };
    vector<int> expected = {
            78,
            1,
            0,
            -6,
            10,
            6
    };
    for (int i = 0; i < tests.size(); i++) {
        int actual = multithreadedSum<int>(&*tests[i].begin(), &*tests[i].end());
        cout << "Actual: " << actual << " Expected: " << expected[i] << endl;
    }
}