/*
    n-dimension cellular automaton
    NaSch Model Simulation
    By CTHua 2021/06/21
*/
#include "stdc++.h"
#include <curses.h>
#include <unistd.h>
#define StartX 1
#define StartY 1
WINDOW *win;
int speed = 1;
double averageSpeed = 0;
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

        puts("origin statement");
        debugPrint();
        puts("-------");
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
    void debugPrint()
    {
        for (int j = 0; j < laneNum; j++)
        {
            for (int i = 0; i < length; i++)
            {
                if (list[j][i].get_velocity() != -1)
                    printf("%d(%d) ", list[j][i].get_velocity(), list[j][i].get_distance());
                else
                    printf("  .  ");
            }
            puts("");
        }
    }
    void colorPrint()
    {
        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < laneNum; j++)
            {
                int vel = list[j][i].get_velocity();
                if (vel != -1)
                {
                    averageSpeed += vel;
                    wattron(win, COLOR_PAIR(vel + 1));
                    mvwprintw(win, StartY + j, StartX + i * 2, "  ");
                    wattroff(win, COLOR_PAIR(vel + 1));
                    usleep(1000 / (vel + 1));
                }
                else
                {
                    wattron(win, COLOR_PAIR(vmax + 2));
                    mvwprintw(win, StartY + j, StartX + i * 2, "  ");
                    wattroff(win, COLOR_PAIR(vmax + 2));
                }
                wrefresh(win);
            }
        }
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
        std::cerr << "Usage: " << std::string(argv[0]) << " length number_of_car max_velocity probability round speed symmetry" << std::endl;
        exit(1);
    }

    /* Initialize Properties*/
    // length of highway
    int L = atoi(argv[1]);

    // number of Car
    int numberOfCar = atoi(argv[2]);

    // the maximum velocity of cars
    int vmax = atoi(argv[3]);

    // probaility let car slower
    double probability = atof(argv[4]);

    // simulate times
    int round = atoi(argv[5]);

    speed = atoi(argv[6]);

    bool symmetry = bool(atoi(argv[7]));
    /*Check Properties*/
    if (L <= 1)
    {
        std::cerr << "Length must be larger than 1" << std::endl;
        return 1;
    }
    if (numberOfCar >= L)
    {
        std::cerr << "number_of_car must be smaller than length" << std::endl;
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
    if (round <= 0)
    {
        std::cerr << "round must be larger than 0" << std::endl;
        return 1;
    }
    if (speed <= 0)
    {
        std::cerr << "speed must be larger than 0" << std::endl;
        return 1;
    }

    /* Start Simulation */

    //Start simulate

    int laneNum = 2;
    Lane highway(L, numberOfCar, vmax, probability, laneNum, symmetry);

    initscr();
    start_color();
    init_color(vmax + 2, 780, 780, 780);
    init_pair(vmax + 2, vmax + 2, vmax + 2);
    curs_set(0);
    for (int i = 0; i <= vmax; i++)
    {
        int red = 1000 / (vmax + 1) * (vmax - i);
        int green = 1000 / (vmax + 1) * (i + 1);
        init_color(i + 50, red, green, 0);
        init_pair(i + 1, vmax + 1, i + 50);
    }
    clear();
    win = newwin(laneNum + 8, L * 2 + 2, 0, 0);
    box(win, '|', '-');

    for (int r = 1; r <= round; r++)
    {
        /* switch lane */
        highway.switchLane();
        highway.update_distance();

        /* update NaSch model */
        highway.nextPos();
        highway.update_distance();

        highway.colorPrint();
        mvwprintw(win, laneNum + 3, 1, "Speed: %dx", speed);
        mvwprintw(win, laneNum + 4, 1, "Now round: %d", r);
        mvwprintw(win, laneNum + 5, 1, "Average Speed: %f", averageSpeed / numberOfCar);
        mvwprintw(win, laneNum + 6, 1, "Mode: %s", symmetry ? "Symmetry" : "Asymmetry");

        touchwin(win);
        wrefresh(win);
        usleep(500000 / speed);
        averageSpeed = 0;
    }
    getchar();
    endwin();

    puts("final statement");
    highway.debugPrint();
    puts("-------");
    return 0;
}