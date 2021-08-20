#include <gmp.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

const unsigned int processorCount = thread::hardware_concurrency();
mutex newComputeMutex, printTotalsMutex;
mpz_t current;
vector<mpz_t> currents(processorCount);

void print()
{
    for (size_t i = 0; i < processorCount; ++i)
        cout << "Thread " << i + 1 << ": " << currents[i];
    cout << "\e[A";
}

void go(int threadNumber)
{
    mpz_t temp;
    mpz_init(temp);
    while (true)
    {
        unique_lock<mutex> newComputeLock(newComputeMutex);

        mpz_set(temp, current);
        mpz_add_ui(current, current, 1);
        mpz_set(currents[threadNumber], temp);

        newComputeLock.unlock();

        while (mpz_cmp_si(temp, 1))
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

        if (printTotalsMutex.try_lock())
        {
            print();
            printTotalsMutex.unlock();
        }
    }
}

int main()
{
    const auto startFrom = "295147905179353994601";

    mpz_init_set_str(current, startFrom, 10);
    vector<thread> threads;

    for (size_t i = 0; i < processorCount; ++i) 
    {
        threads.push_back(thread(go, i));
    }

    for (auto& t : threads)
        t.join();
}

