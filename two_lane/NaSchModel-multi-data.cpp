/*
    n-dimension cellular automaton
    NaSch Model Simulation
    By CTHua 2021/06/21
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
    void set_velocity(int velocity_) { velocity = velocity_; }

    int get_distance() { return distance; }
    void set_distance(int distance_) { distance = distance_; }

    int get_d_0() { return d_0; }
    void set_d_0(int d_0_) { d_0 = d_0_; }

    int get_d_0_back() { return d_0_back; }
    void set_d_0_back(int d_0_back_) { d_0_back = d_0_back_; }

private:
    int velocity;
    int distance;
    int d_0;
    int d_0_back;
};

class Lane
{
public:
    //Constructor of one lane of high way
    Lane(int length_, int carNum_, int vmax_, double slowProb_, int laneNum_, bool symmetry_) : length(length_), vmax(vmax_), carNum(carNum_), slowProb(slowProb_), laneNum(laneNum_), symmetry(symmetry_)
    {
        /* init Lane */
        list = new Car *[laneNum];
        for (int i = 0; i < laneNum; i++)
            list[i] = new Car[length];

        /* init origin car position */
        int oneLaneCarNum = carNum / laneNum + carNum % laneNum;
        int *carPos = new int[oneLaneCarNum];
        for (int i = 0; i < oneLaneCarNum; i++)
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
            list[0][carPos[i]] = Car(rint() % vmax + 1);
        }
        delete[] carPos;
        oneLaneCarNum = carNum / laneNum;
        for (int j = 1; j < laneNum; j++)
        {
            int *carPos = new int[carNum];
            for (int i = 0; i < oneLaneCarNum; i++)
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
                list[j][carPos[i]] = Car(rint() % vmax + 1);
            }
            delete[] carPos;
        }

        /* update car and car distance */
        for (int i = 0; i < carNum_; i++)
            update_distance();
    }
    ~Lane()
    {
        for (int i = 0; i < laneNum; i++)
            delete[] list[i];
        delete[] list;
    }
    void update_distance()
    {
        for (int j = 0; j < laneNum; j++)
        {
            int first = 0;
            while (list[j][first].get_velocity() == -1)
                first++;
            for (int i = first; i < length; i++)
            {
                /* jump to car position*/
                while (list[j][i].get_velocity() == -1)
                    i++;

                /* find this lane car distance */
                for (int k = i + 1; k <= length + i; k++)
                {
                    int pos = k % length;
                    if (list[j][pos].get_velocity() != -1)
                    {
                        int distance = k - i;
                        list[j][i].set_distance(distance - 1);
                        i = pos - 1;
                        break;
                    }
                }
                if (i + 1 == first)
                    break;
            }
        }

        /* distance to next lane*/
        for (int j = 0; j < laneNum; j++)
        {
            int calculatedCount = 0;
            for (int i = 0; i < length; i++)
            {
                /* jump to car position*/
                if (list[j][i].get_velocity() == -1)
                    continue;

                /* find next lane car distance (d_0, d_0_back)*/
                bool found_d_0 = false, found_d_0_back = false;
                for (int k = 1; k < length / 2; k++)
                {
                    if (!found_d_0 && list[(j + 1) % laneNum][(i + k) % length].get_velocity() != -1)
                    {
                        list[j][i].set_d_0(k);
                        found_d_0 = true;
                    }
                    if (!found_d_0_back && list[(j + 1) % laneNum][(i - k + length) % length].get_velocity() != -1)
                    {
                        list[j][i].set_d_0_back(k);
                        found_d_0_back = true;
                    }
                }
            }
        }
    }
    void switchLane()
    {
        if (symmetry)
        {
            for (int j = 0; j < laneNum; j++)
            {
                for (int i = 0; i < length; i++)
                {
                    if (list[j][i].get_velocity() != -1)
                    {
                        int l = std::min(list[j][i].get_velocity() + 1, vmax);
                        int l_0 = list[j][i].get_distance();
                        int l_0_back = vmax;

                        int d = list[j][i].get_distance();
                        int d_0 = list[j][i].get_d_0();
                        int d_0_back = list[j][i].get_d_0_back();

                        /*
                        when meet all condition then switch the lane
                        (C1) d < l
                        (C2) d_0 > l_0
                        (C3) adjacent position is empty
                        (C4) d_0_back >= l_0_back
                        */
                        if (d < l && d_0 > l_0 && list[(j + 1) % laneNum][i].get_velocity() == -1 && d_0_back >= l_0_back)
                        {
                            list[(j + 1) % laneNum][i] = Car(list[j][i].get_velocity());
                            list[j][i] = Car(-1);
                        }
                    }
                }
            }
        }
        else
        {
            /*to L*/
            int j = 1;
            for (int i = 0; i < length; i++)
            {
                if (list[j][i].get_velocity() != -1)
                {
                    int l = std::min(list[j][i].get_velocity() + 1, vmax);
                    int l_0 = list[j][i].get_distance();
                    int l_0_back = vmax;

                    int d = list[j][i].get_distance();
                    int d_0 = list[j][i].get_d_0();
                    int d_0_back = list[j][i].get_d_0_back();

                    /*
                    when meet all condition then switch the lane
                    (R1) d < l
                    (R2) d_0 > l_0
                    (R3) adjacent position is empty
                    (R4) d_0_back >= l_0_back
                    */
                    if (d < l && d_0 > l_0 && list[(j + 1) % laneNum][i].get_velocity() == -1 && d_0_back >= l_0_back)
                    {
                        list[(j + 1) % laneNum][i] = Car(list[j][i].get_velocity());
                        list[j][i] = Car(-1);
                    }
                }
            }
            /*to R*/
            j = 0;
            for (int i = 0; i < length; i++)
            {
                if (list[j][i].get_velocity() != -1)
                {
                    int l = std::min(list[j][i].get_velocity() + 1, vmax);
                    int l_0 = list[j][i].get_distance();
                    int l_0_back = vmax;

                    int d = list[j][i].get_distance();
                    int d_0 = list[j][i].get_d_0();
                    int d_0_back = list[j][i].get_d_0_back();

                    /*
                    when meet all condition then switch the lane
                    (L1) d_0 >= v
                    (L2) adjacent position is empty
                    (L3) d_0_back >= l_0_back
                    */
                    if (d_0 >= list[j][i].get_velocity() && list[(j + 1) % laneNum][i].get_velocity() == -1 && d_0_back >= l_0_back)
                    {
                        list[(j + 1) % laneNum][i] = Car(list[j][i].get_velocity());
                        list[j][i] = Car(-1);
                    }
                }
            }
        }
    }
    void nextPos()
    {
        for (int j = 0; j < laneNum; j++)
        {
            for (int i = 0; i < length; i++)
                list[j][i].oneRound(vmax, slowProb);
            for (int i = 0; i < length; i++)
            {
                if (list[j][i].get_velocity() != -1 && list[j][i].get_distance() != -1)
                {
                    int newPos = (list[j][i].get_velocity() + i) % length;
                    list[j][newPos] = Car(list[j][i].get_velocity());
                    list[j][newPos].set_distance(-1);
                    if (newPos != i)
                        list[j][i].set_velocity(-1);
                }
            }
        }
    }
    int totalVelocity()
    {
        int total = 0;
        for (int j = 0; j < laneNum; j++)
        {
            for (int i = 0; i < length; i++)
            {
                int v = list[j][i].get_velocity();
                total += v == -1 ? 0 : v;
            }
        }
        return total;
    }

    double *usage()
    {
        double *used = new double[laneNum];
        for (int j = 0; j < laneNum; j++)
        {
            int total = 0;
            for (int i = 0; i < length; i++)
            {
                int v = list[j][i].get_velocity();
                total += v == -1 ? 0 : 1;
            }
            used[j] = double(total) / carNum;
        }
        return used;
    }

private:
    Car **list;
    int length;
    int carNum;
    int laneNum;
    int vmax;
    double slowProb;
    bool symmetry;
};
int main(int argc, char **argv)
{
    if (argc != 8)
    {
        std::cerr << "Usage: " << std::string(argv[0]) << " length max_velocity probability t_0 T average symmetry" << std::endl;
        std::cerr << "---" << std::endl;
        std::cerr << "length: length of lane" << std::endl;
        std::cerr << "max_velocity: maximum velocity in one lane" << std::endl;
        std::cerr << "probability: probability let car be slower" << std::endl;
        std::cerr << "t_0: do t_0 round and start calculate" << std::endl;
        std::cerr << "T: one sample simulate round" << std::endl;
        std::cerr << "average: number of independent sample" << std::endl;
        std::cerr << "symmetry: 1 for symmetry, 0 for asymmetry" << std::endl;
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

    bool symmetry = bool(atoi(argv[7]));
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

    //Start simulate

    int laneNum = 2;
    std::string fileName = "./data/";
    fileName += "L" + std::to_string(length);
    fileName += "_v" + std::to_string(vmax);
    fileName += "_p" + std::to_string(probability);
    fileName += symmetry ? "_symmetry" : "_asymmetry";
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
        double *usage = new double[laneNum];
        for (int independent = 1; independent <= average; independent++)
        {
            Lane highway(length, numberOfCar, vmax, probability, laneNum, symmetry);
            for (int r = 1; r <= t_0; r++)
            {
                /* switch lane */
                highway.switchLane();
                highway.update_distance();

                /* update NaSch model */
                highway.nextPos();
                highway.update_distance();
            }

            for (int r = 1; r <= T; r++)
            {
                /* switch lane */
                highway.switchLane();
                highway.update_distance();

                /* update NaSch model */
                highway.nextPos();
                highway.update_distance();

                vel += double(highway.totalVelocity()) / numberOfCar;
                double *tmp = highway.usage();
                for (int i = 0; i < laneNum; i++)
                    usage[i] += tmp[i];
                delete[] tmp;
            }
        }
        vel /= T;
        vel /= average;
        double flow = vel * density;
        result << density << " " << flow << " " << vel;
        for (int i = 0; i < laneNum; i++)
            result << " " << usage[i] / T / average;
        result << std::endl;
    }
    result.close();
    return 0;
}