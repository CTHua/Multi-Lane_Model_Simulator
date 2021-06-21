/*
    1-dimension cellular automaton
    NaSch Model Simulation
    By CTHua 2021/05/11
*/
#include "stdc++.h"
/* Initialize Random Number Generator */

// random_device
std::random_device rd;

// random_generator(use mersenne_twister_engine)
std::mt19937_64 generator(rd());

// random_distribution(uniform_distribution)
std::uniform_real_distribution<double> rand_double(0.0, 1.0);
std::uniform_int_distribution<int> rand_int(0, INT_MAX);
int rint() { return rand_int(generator); }
double rdouble() { return rand_double(generator); }

/**/
class Car
{
public:
    //Default constructor for Car, it means this is an empty car.
    Car()
    {
        velocity = -1;
        distance = -1;
    }

    //Constructor for Car, it means this is a Car, velocity is v;
    Car(int v)
    {
        velocity = v;
    }

    //Step 1, if velocity < vmax, then velocity + 1
    void faster(int vmax_)
    {
        velocity = std::min(vmax_, velocity + 1);
    }

    //Step 2, if velocity > distance between the front car, velocity = distance
    void slower()
    {
        velocity = velocity > distance ? distance : velocity;
    }

    //Step 3, if velocity > 0, then velocity has a probability "prob" let the velocity - 1.
    void random_slower(double prob)
    {
        double r = rdouble();
        if (r < prob && velocity > 0)
            velocity = velocity - 1;
    }

    //do Step 1, 2, 3
    void oneRound(int vmax, double prob)
    {
        if (velocity == -1)
            return;
        faster(vmax);
        slower();
        random_slower(prob);
    }

    /*Some functions let outer scope can get prototype of this class */
    int get_velocity() { return velocity; }
    int get_distance() { return distance; }
    void set_velocity(int velocity_) { velocity = velocity_; }
    void set_distance(int distance_) { distance = distance_; }

private:
    int velocity;
    int distance;
};

class Lane
{
public:
    //Constructor of one lane of high way
    Lane(int length_, int carNum_, int vmax_, double slowProb_, int laneNum_) : length(length_), vmax(vmax_), carNum(carNum_), laneNum(laneNum_), slowProb(slowProb_)
    {
        list = new Car[length];
        int *carPos = new int[carNum];
        for (int i = 0; i < carNum; i++)
        {
            int randomPos = rint() % length;
            bool duplicate = true;
            while (duplicate)
            {
                int countDuplicate = 0;
                for (countDuplicate = 0; countDuplicate < i; countDuplicate++)
                {
                    if (randomPos == carPos[countDuplicate])
                    {
                        randomPos = rint() % length;
                        break;
                    }
                }
                if (countDuplicate == i)
                    duplicate = false;
            }
            carPos[i] = randomPos;
            list[carPos[i]] = Car(rint() % vmax + 1);
        }
        for (int i = 0; i < carNum_; i++)
            update_distance();
        delete[] carPos;
    }
    ~Lane()
    {
        delete[] list;
    }
    void update_distance()
    {
        int calculatedCount = 0;
        for (int i = 0; i < length && calculatedCount < carNum; i++)
        {
            while (list[i].get_velocity() == -1)
                i++;
            for (int j = i + 1; j <= length + i; j++)
            {
                int pos = j % length;
                if (list[pos].get_velocity() != -1)
                {
                    int distance = j - i;
                    list[i].set_distance(distance - 1);
                    i = pos - 1;
                    calculatedCount++;
                    break;
                }
            }
        }
    }
    void nextPos()
    {
        for (int i = 0; i < length; i++)
            list[i].oneRound(vmax, slowProb);
        for (int i = 0; i < length; i++)
        {
            if (list[i].get_velocity() != -1 && list[i].get_distance() != -1)
            {
                int newPos = (list[i].get_velocity() + i) % length;
                list[newPos] = Car(list[i].get_velocity());
                list[newPos].set_distance(-1);
                if (newPos != i)
                    list[i].set_velocity(-1);
            }
        }
    }
    std::string debugPrint()
    {
        std::string result = "";
        for (int i = 0; i < length; i++)
        {
            if (list[i].get_velocity() != -1)
            {
                result += std::to_string(list[i].get_velocity());
                result += "(";
                result += std::to_string(list[i].get_distance());
                result += ") ";
            }
            else
                result += "  .  ";
        }
        return result;
    }
    int totalVelocity()
    {
        int total = 0;
        for (int i = 0; i < length; i++)
        {
            int v = list[i].get_velocity();
            total += v == -1 ? 0 : v;
        }
        return total;
    }

private:
    Car *list;
    int length;
    int carNum;
    int laneNum;
    int vmax;
    double slowProb;
};
int main(int argc, char **argv)
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << std::string(argv[0]) << " length max_velocity probability t_0 T average" << std::endl;
        std::cerr << "---" << std::endl;
        std::cerr << "length: length of lane" << std::endl;
        std::cerr << "max_velocity: maximum velocity in one lane" << std::endl;
        std::cerr << "probability: probability let car be slower" << std::endl;
        std::cerr << "t_0: do t_0 round and start calculate" << std::endl;
        std::cerr << "T: one sample simulate round" << std::endl;
        std::cerr << "average: number of independent sample" << std::endl;
        exit(1);
    }

    /* Initialize Properties*/
    // length of highway
    int length = atoi(argv[1]);

    // the maximum velocity of cars
    int vmax = atoi(argv[2]);

    // probaility let car slower
    double probability = atof(argv[3]);

    //uncalc simulate times
    int t_0 = atoi(argv[4]);

    //T times average per one sample
    int T = atoi(argv[5]);

    //number of samples
    int average = atoi(argv[6]);

    /*Check Properties*/
    if (length <= 1)
    {
        std::cerr << "Length must be larger than 1" << std::endl;
        return 1;
    }
    if (vmax <= 0)
    {
        std::cerr << "max_velocity must be larger than 0" << std::endl;
        return 1;
    }
    if (probability >= 1)
    {
        std::cerr << "probability must be smaller than 1" << std::endl;
        return 1;
    }
    if (t_0 <= 0)
    {
        std::cerr << "t_0 must be larger than 0" << std::endl;
        return 1;
    }
    if (T <= 0)
    {
        std::cerr << "T must be larger than 0" << std::endl;
        return 1;
    }
    if (average <= 0)
    {
        std::cerr << "average must be larger than 0" << std::endl;
        return 1;
    }

    /* Start Simulation */

    //fileIO
    std::string fileName = "./data/";
    fileName += "L" + std::to_string(length);
    fileName += "_v" + std::to_string(vmax);
    fileName += "_p" + std::to_string(probability);
    fileName += ".dat";
    int systemReturn = system("mkdir data 2>/dev/null");
    std::fstream result;
    result.open(fileName, std::ios::out);
    //Start simulate
    result << 0 << " " << 0 << std::endl;
    for (int numberOfCar = 1; numberOfCar <= length; numberOfCar++)
    {
        double density = double(numberOfCar) / length;
        double vel = 0;
        for (int independent = 1; independent <= average; independent++)
        {
            Lane highway(length, numberOfCar, vmax, probability, 1);
            for (int r = 1; r <= t_0; r++)
            {
                highway.nextPos();
                highway.update_distance();
            }

            for (int r = 1; r <= T; r++)
            {
                highway.nextPos();
                highway.update_distance();
                vel += double(highway.totalVelocity()) / numberOfCar;
            }
        }
        vel /= T;
        vel /= average;
        double flow = vel * density;
        result << density << " " << flow << " " << vel << std::endl;
    }
    result.close();
    return 0;
}