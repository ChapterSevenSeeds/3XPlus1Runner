#include <gmp.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <conio.h>
#include <chrono>

using namespace std;

const unsigned int processorCount = thread::hardware_concurrency();
mutex newComputeMutex;
mpz_t current, sinceLastCheck;
vector<mpz_t> currents(processorCount);
vector<chrono::steady_clock::time_point> timestamps(processorCount);
chrono::steady_clock::time_point timeSinceLastCheck;

string min()
{
    stringstream result;
    mpz_t temp;
    mpz_init(temp);
    mpz_set(temp, currents[0]);

    for (const auto& item : currents)
    {
        if (mpz_cmp(temp, item) == -1)
            mpz_set(temp, item);
    }

    result << temp;

    return result.str();
}

void print()
{
    mpz_sub(sinceLastCheck, current, sinceLastCheck);

    auto currentTime = chrono::steady_clock::now();
    auto timespanSinceLastCheck = chrono::duration_cast<chrono::seconds>(currentTime - timeSinceLastCheck).count();

    mpz_fdiv_q_ui(sinceLastCheck, sinceLastCheck, timespanSinceLastCheck ? timespanSinceLastCheck : 1);

    for (size_t i = 0; i < processorCount; ++i)
        cout << "Thread " << i + 1 << ": " << chrono::duration_cast<chrono::seconds>(currentTime - timestamps[i]).count() << " seconds" << endl;

    cout << "Current: " << min() << endl;
    cout << "Numbers per second: " << sinceLastCheck << endl;
    cout << "[s] Status, [q] Quit, [p] Pause" << endl << endl;

    mpz_set(sinceLastCheck, current);
    timeSinceLastCheck = chrono::steady_clock::now();
}

void go(int threadNumber)
{
    mpz_t temp, original;
    mpz_init(temp);
    mpz_init(original);
    while (true)
    {
        timestamps[threadNumber] = chrono::steady_clock::now();
        unique_lock<mutex> newComputeLock(newComputeMutex);

        mpz_set(original, current);
        mpz_add_ui(current, current, 1);
        mpz_set(currents[threadNumber], original);
        mpz_set(temp, original);

        newComputeLock.unlock();

        while (mpz_cmp(temp, original) >= 0)
        {
            if (mpz_odd_p(temp))
            {
                mpz_mul_si(temp, temp, 3);
                mpz_add_ui(temp, temp, 1);
            }
            else
            {
                mpz_divexact_ui(temp, temp, 2);
            }
        }
    }
}

int main()
{
    string startFrom;
    cout << "Start from: ";
    cin >> startFrom;

    mpz_init_set_str(current, startFrom.c_str(), 10);
    mpz_set(sinceLastCheck, current);
    timeSinceLastCheck = chrono::steady_clock::now();

    vector<thread> threads;

    for (size_t i = 0; i < processorCount; ++i) 
    {
        threads.push_back(thread(go, i));
    }

    print();

    while (true)
    {
        int c = _getch();

        if (c == 's')
            print();
    }
}

